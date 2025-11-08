/*
 * Asynchronous Metal Pipeline Manager for OpenJPEG
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Enables overlapped CPU-GPU execution for maximum throughput
 */

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "opj_includes.h"

/* ========================================================================
 * Pipeline Stage Structure
 * ======================================================================== */

typedef enum {
    STAGE_IDLE,
    STAGE_PREPARING,
    STAGE_EXECUTING,
    STAGE_COMPLETING,
    STAGE_DONE
} stage_status_t;

typedef struct opj_metal_stage {
    stage_status_t status;
    id<MTLCommandBuffer> command_buffer;
    id<MTLBuffer> input_buffers[3];
    id<MTLBuffer> output_buffers[3];
    size_t data_size;
    void (*completion_callback)(struct opj_metal_stage*);
    void *user_data;
    double submit_time;
    double complete_time;
} opj_metal_stage_t;

/* ========================================================================
 * Async Pipeline Context
 * ======================================================================== */

typedef struct {
    id<MTLDevice> device;
    id<MTLCommandQueue> command_queue;
    id<MTLComputePipelineState> mct_pipeline;
    id<MTLComputePipelineState> dwt_pipeline;
    
    /* Pipeline stages for double/triple buffering */
    opj_metal_stage_t stages[3];
    int current_stage;
    int num_stages;
    
    /* Synchronization */
    dispatch_semaphore_t stage_semaphore;
    dispatch_queue_t completion_queue;
    
    /* Statistics */
    unsigned long frames_processed;
    double total_gpu_time;
    double total_wait_time;
    bool pipeline_enabled;
    
} opj_metal_async_context;

/* ========================================================================
 * Initialization
 * ======================================================================== */

opj_metal_async_context* opj_metal_async_init(void)
{
    @autoreleasepool {
        opj_metal_async_context *ctx = calloc(1, sizeof(opj_metal_async_context));
        if (!ctx) return NULL;
        
        /* Get default Metal device */
        ctx->device = MTLCreateSystemDefaultDevice();
        if (!ctx->device) {
            free(ctx);
            return NULL;
        }
        
        /* Create command queue */
        ctx->command_queue = [ctx->device newCommandQueue];
        if (!ctx->command_queue) {
            free(ctx);
            return NULL;
        }
        
        /* Load Metal library */
        NSError *error = nil;
        NSBundle *bundle = [NSBundle mainBundle];
        NSString *libraryPath = [bundle pathForResource:@"openjpeg_metal" 
                                                 ofType:@"metallib"];
        
        id<MTLLibrary> library;
        if (libraryPath) {
            NSURL *libraryURL = [NSURL fileURLWithPath:libraryPath];
            library = [ctx->device newLibraryWithURL:libraryURL error:&error];
        } else {
            library = [ctx->device newDefaultLibrary];
        }
        
        if (!library) {
            free(ctx);
            return NULL;
        }
        
        /* Create pipeline states */
        id<MTLFunction> mct_function = [library newFunctionWithName:@"mct_encode"];
        id<MTLFunction> dwt_function = [library newFunctionWithName:@"dwt_5_3_forward_h"];
        
        if (mct_function) {
            ctx->mct_pipeline = [ctx->device newComputePipelineStateWithFunction:mct_function
                                                                           error:&error];
        }
        
        if (dwt_function) {
            ctx->dwt_pipeline = [ctx->device newComputePipelineStateWithFunction:dwt_function
                                                                           error:&error];
        }
        
        /* Initialize pipeline stages */
        ctx->num_stages = 3;  /* Triple buffering */
        for (int i = 0; i < ctx->num_stages; i++) {
            ctx->stages[i].status = STAGE_IDLE;
        }
        
        /* Create synchronization primitives */
        ctx->stage_semaphore = dispatch_semaphore_create(ctx->num_stages);
        ctx->completion_queue = dispatch_queue_create("com.openjpeg.metal.completion",
                                                      DISPATCH_QUEUE_SERIAL);
        
        ctx->pipeline_enabled = true;
        
        return ctx;
    }
}

/* ========================================================================
 * Stage Management
 * ======================================================================== */

opj_metal_stage_t* opj_metal_acquire_stage(opj_metal_async_context *ctx)
{
    /* Wait for available stage */
    dispatch_semaphore_wait(ctx->stage_semaphore, DISPATCH_TIME_FOREVER);
    
    /* Find idle stage */
    for (int i = 0; i < ctx->num_stages; i++) {
        if (ctx->stages[i].status == STAGE_IDLE) {
            ctx->stages[i].status = STAGE_PREPARING;
            return &ctx->stages[i];
        }
    }
    
    return NULL;
}

void opj_metal_release_stage(opj_metal_async_context *ctx, opj_metal_stage_t *stage)
{
    stage->status = STAGE_IDLE;
    dispatch_semaphore_signal(ctx->stage_semaphore);
}

/* ========================================================================
 * Async MCT Operations
 * ======================================================================== */

void opj_metal_mct_encode_async(
    opj_metal_async_context *ctx,
    OPJ_INT32 *c0,
    OPJ_INT32 *c1,
    OPJ_INT32 *c2,
    size_t n,
    void (*callback)(void*),
    void *user_data)
{
    @autoreleasepool {
        opj_metal_stage_t *stage = opj_metal_acquire_stage(ctx);
        if (!stage) return;
        
        stage->data_size = n * sizeof(OPJ_INT32);
        stage->user_data = user_data;
        
        /* Create Metal buffers */
        stage->input_buffers[0] = [ctx->device newBufferWithBytes:c0
                                                            length:stage->data_size
                                                           options:MTLResourceStorageModeShared];
        stage->input_buffers[1] = [ctx->device newBufferWithBytes:c1
                                                            length:stage->data_size
                                                           options:MTLResourceStorageModeShared];
        stage->input_buffers[2] = [ctx->device newBufferWithBytes:c2
                                                            length:stage->data_size
                                                           options:MTLResourceStorageModeShared];
        
        /* Create command buffer */
        stage->command_buffer = [ctx->command_queue commandBuffer];
        id<MTLComputeCommandEncoder> encoder = [stage->command_buffer computeCommandEncoder];
        
        /* Setup compute pass */
        [encoder setComputePipelineState:ctx->mct_pipeline];
        [encoder setBuffer:stage->input_buffers[0] offset:0 atIndex:0];
        [encoder setBuffer:stage->input_buffers[1] offset:0 atIndex:1];
        [encoder setBuffer:stage->input_buffers[2] offset:0 atIndex:2];
        
        uint32_t size_param = (uint32_t)n;
        [encoder setBytes:&size_param length:sizeof(uint32_t) atIndex:3];
        
        /* Dispatch */
        MTLSize gridSize = MTLSizeMake((n + 255) / 256, 1, 1);
        MTLSize threadgroupSize = MTLSizeMake(256, 1, 1);
        [encoder dispatchThreadgroups:gridSize threadsPerThreadgroup:threadgroupSize];
        [encoder endEncoding];
        
        /* Setup completion handler */
        __block opj_metal_async_context *block_ctx = ctx;
        __block opj_metal_stage_t *block_stage = stage;
        __block OPJ_INT32 *dst0 = c0, *dst1 = c1, *dst2 = c2;
        
        [stage->command_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            /* Copy results back */
            memcpy(dst0, [block_stage->input_buffers[0] contents], block_stage->data_size);
            memcpy(dst1, [block_stage->input_buffers[1] contents], block_stage->data_size);
            memcpy(dst2, [block_stage->input_buffers[2] contents], block_stage->data_size);
            
            /* Call user callback */
            if (callback) {
                dispatch_async(block_ctx->completion_queue, ^{
                    callback(block_stage->user_data);
                });
            }
            
            /* Update statistics */
            block_ctx->frames_processed++;
            block_stage->complete_time = CACurrentMediaTime();
            block_ctx->total_gpu_time += block_stage->complete_time - block_stage->submit_time;
            
            /* Release stage */
            opj_metal_release_stage(block_ctx, block_stage);
        }];
        
        /* Submit */
        stage->submit_time = CACurrentMediaTime();
        stage->status = STAGE_EXECUTING;
        [stage->command_buffer commit];
    }
}

/* ========================================================================
 * Batch Processing Pipeline
 * ======================================================================== */

typedef struct {
    OPJ_INT32 *data[3];
    size_t size;
    int batch_id;
} batch_item_t;

void opj_metal_batch_mct_async(
    opj_metal_async_context *ctx,
    batch_item_t *items,
    int num_items,
    void (*batch_complete_callback)(int batch_id, void*),
    void *user_data)
{
    @autoreleasepool {
        /* Process items in pipeline fashion */
        for (int i = 0; i < num_items; i++) {
            batch_item_t *item = &items[i];
            
            opj_metal_stage_t *stage = opj_metal_acquire_stage(ctx);
            if (!stage) continue;
            
            stage->data_size = item->size * sizeof(OPJ_INT32);
            
            /* Create buffers */
            for (int ch = 0; ch < 3; ch++) {
                stage->input_buffers[ch] = [ctx->device newBufferWithBytes:item->data[ch]
                                                                    length:stage->data_size
                                                                   options:MTLResourceStorageModeShared];
            }
            
            /* Create command buffer */
            stage->command_buffer = [ctx->command_queue commandBuffer];
            id<MTLComputeCommandEncoder> encoder = [stage->command_buffer computeCommandEncoder];
            
            [encoder setComputePipelineState:ctx->mct_pipeline];
            for (int ch = 0; ch < 3; ch++) {
                [encoder setBuffer:stage->input_buffers[ch] offset:0 atIndex:ch];
            }
            
            uint32_t size_param = (uint32_t)item->size;
            [encoder setBytes:&size_param length:sizeof(uint32_t) atIndex:3];
            
            MTLSize gridSize = MTLSizeMake((item->size + 255) / 256, 1, 1);
            MTLSize threadgroupSize = MTLSizeMake(256, 1, 1);
            [encoder dispatchThreadgroups:gridSize threadsPerThreadgroup:threadgroupSize];
            [encoder endEncoding];
            
            /* Completion handler */
            __block int batch_id = item->batch_id;
            __block opj_metal_async_context *block_ctx = ctx;
            __block opj_metal_stage_t *block_stage = stage;
            __block OPJ_INT32 *dst[3] = {item->data[0], item->data[1], item->data[2]};
            __block size_t block_size = stage->data_size;
            
            [stage->command_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
                /* Copy results */
                for (int ch = 0; ch < 3; ch++) {
                    memcpy(dst[ch], [block_stage->input_buffers[ch] contents], block_size);
                }
                
                /* Callback */
                if (batch_complete_callback) {
                    dispatch_async(block_ctx->completion_queue, ^{
                        batch_complete_callback(batch_id, user_data);
                    });
                }
                
                opj_metal_release_stage(block_ctx, block_stage);
            }];
            
            stage->submit_time = CACurrentMediaTime();
            stage->status = STAGE_EXECUTING;
            [stage->command_buffer commit];
        }
    }
}

/* ========================================================================
 * Pipeline Control
 * ======================================================================== */

void opj_metal_async_wait_all(opj_metal_async_context *ctx)
{
    /* Wait for all stages to complete */
    for (int i = 0; i < ctx->num_stages; i++) {
        if (ctx->stages[i].status == STAGE_EXECUTING) {
            [ctx->stages[i].command_buffer waitUntilCompleted];
        }
    }
}

void opj_metal_async_flush(opj_metal_async_context *ctx)
{
    /* Commit any pending work */
    opj_metal_async_wait_all(ctx);
}

/* ========================================================================
 * Statistics
 * ======================================================================== */

void opj_metal_async_get_stats(
    opj_metal_async_context *ctx,
    unsigned long *frames_processed,
    double *avg_gpu_time,
    double *pipeline_efficiency)
{
    if (frames_processed) {
        *frames_processed = ctx->frames_processed;
    }
    
    if (avg_gpu_time && ctx->frames_processed > 0) {
        *avg_gpu_time = ctx->total_gpu_time / ctx->frames_processed;
    }
    
    if (pipeline_efficiency && ctx->frames_processed > 0) {
        /* Measure overlap efficiency */
        double theoretical_time = ctx->total_gpu_time;
        double actual_time = ctx->total_gpu_time + ctx->total_wait_time;
        *pipeline_efficiency = theoretical_time / actual_time;
    }
}

void opj_metal_async_print_stats(opj_metal_async_context *ctx)
{
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║          Metal Async Pipeline Statistics                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Frames Processed: %lu\n", ctx->frames_processed);
    
    if (ctx->frames_processed > 0) {
        double avg_time = ctx->total_gpu_time / ctx->frames_processed;
        printf("  Avg GPU Time:     %.3f ms\n", avg_time * 1000);
        printf("  Total GPU Time:   %.3f s\n", ctx->total_gpu_time);
        
        double efficiency = 1.0;
        if (ctx->total_wait_time > 0) {
            efficiency = ctx->total_gpu_time / (ctx->total_gpu_time + ctx->total_wait_time);
        }
        printf("  Pipeline Efficiency: %.1f%%\n", efficiency * 100);
        
        double throughput = ctx->frames_processed / ctx->total_gpu_time;
        printf("  Throughput:       %.1f frames/sec\n", throughput);
    }
    
    printf("\n");
}

/* ========================================================================
 * Cleanup
 * ======================================================================== */

void opj_metal_async_cleanup(opj_metal_async_context *ctx)
{
    if (!ctx) return;
    
    /* Wait for all work to complete */
    opj_metal_async_wait_all(ctx);
    
    /* Release resources */
    if (ctx->stage_semaphore) {
        dispatch_release(ctx->stage_semaphore);
    }
    
    if (ctx->completion_queue) {
        dispatch_release(ctx->completion_queue);
    }
    
    free(ctx);
}

/* ========================================================================
 * Convenience Functions
 * ======================================================================== */

bool opj_metal_async_is_idle(opj_metal_async_context *ctx)
{
    for (int i = 0; i < ctx->num_stages; i++) {
        if (ctx->stages[i].status != STAGE_IDLE) {
            return false;
        }
    }
    return true;
}

int opj_metal_async_get_pending_count(opj_metal_async_context *ctx)
{
    int count = 0;
    for (int i = 0; i < ctx->num_stages; i++) {
        if (ctx->stages[i].status == STAGE_EXECUTING) {
            count++;
        }
    }
    return count;
}
