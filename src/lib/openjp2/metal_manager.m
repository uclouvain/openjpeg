/*
 * Metal GPU Manager for OpenJPEG
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Metal framework integration for GPU-accelerated JPEG 2000 operations
 */

#if defined(__APPLE__) && defined(HAVE_METAL)

#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#include "opj_includes.h"
#include "metal_manager.h"

/* Metal context structure */
struct opj_metal_context {
    id<MTLDevice> device;
    id<MTLCommandQueue> queue;
    id<MTLLibrary> library;
    
    /* Compute pipeline states */
    id<MTLComputePipelineState> mct_encode_pipeline;
    id<MTLComputePipelineState> mct_decode_pipeline;
    id<MTLComputePipelineState> dwt_forward_h_pipeline;
    id<MTLComputePipelineState> dwt_forward_v_pipeline;
    id<MTLComputePipelineState> dwt_inverse_h_pipeline;
    id<MTLComputePipelineState> dwt_inverse_v_pipeline;
    id<MTLComputePipelineState> quantize_pipeline;
    
    /* Buffer pools for reuse */
    NSMutableArray<id<MTLBuffer>>* buffer_pool;
    
    /* Configuration */
    size_t min_size_for_gpu;  /* Minimum pixels to use GPU */
    bool enabled;
};

/* ========================================================================
 * Initialization and Cleanup
 * ======================================================================== */

opj_metal_context* opj_metal_init(void)
{
    @autoreleasepool {
        opj_metal_context* ctx = (opj_metal_context*)calloc(1, sizeof(opj_metal_context));
        if (!ctx) {
            return NULL;
        }
        
        /* Get default Metal device */
        ctx->device = MTLCreateSystemDefaultDevice();
        if (!ctx->device) {
            opj_event_msg(NULL, EVT_WARNING, "Metal: No GPU device available\n");
            free(ctx);
            return NULL;
        }
        
        /* Create command queue */
        ctx->queue = [ctx->device newCommandQueue];
        if (!ctx->queue) {
            opj_event_msg(NULL, EVT_ERROR, "Metal: Failed to create command queue\n");
            free(ctx);
            return NULL;
        }
        
        /* Load shader library */
        NSError* error = nil;
        NSString* libraryPath = [[NSBundle mainBundle] pathForResource:@"openjpeg_metal" 
                                                                 ofType:@"metallib"];
        
        if (libraryPath) {
            ctx->library = [ctx->device newLibraryWithFile:libraryPath error:&error];
        } else {
            /* Try to load default library (compiled into app) */
            ctx->library = [ctx->device newDefaultLibrary];
        }
        
        if (!ctx->library) {
            opj_event_msg(NULL, EVT_WARNING, 
                "Metal: Failed to load shader library: %s\n",
                [[error localizedDescription] UTF8String]);
            free(ctx);
            return NULL;
        }
        
        /* Create compute pipeline states */
        if (!opj_metal_create_pipelines(ctx)) {
            opj_metal_cleanup(ctx);
            return NULL;
        }
        
        /* Initialize buffer pool */
        ctx->buffer_pool = [[NSMutableArray alloc] initWithCapacity:8];
        
        /* Set configuration */
        ctx->min_size_for_gpu = 512 * 512;  /* Use GPU for images >= 512x512 */
        ctx->enabled = true;
        
        opj_event_msg(NULL, EVT_INFO, 
            "Metal: Initialized GPU acceleration on %s\n",
            [[ctx->device name] UTF8String]);
        
        return ctx;
    }
}

static bool opj_metal_create_pipelines(opj_metal_context* ctx)
{
    @autoreleasepool {
        NSError* error = nil;
        
        /* MCT encode pipeline */
        id<MTLFunction> function = [ctx->library newFunctionWithName:@"mct_encode_reversible"];
        if (function) {
            ctx->mct_encode_pipeline = [ctx->device newComputePipelineStateWithFunction:function
                                                                                   error:&error];
            if (!ctx->mct_encode_pipeline) {
                opj_event_msg(NULL, EVT_ERROR, "Metal: Failed to create MCT encode pipeline\n");
                return false;
            }
        }
        
        /* MCT decode pipeline */
        function = [ctx->library newFunctionWithName:@"mct_decode_reversible"];
        if (function) {
            ctx->mct_decode_pipeline = [ctx->device newComputePipelineStateWithFunction:function
                                                                                   error:&error];
        }
        
        /* DWT pipelines */
        function = [ctx->library newFunctionWithName:@"dwt_forward_5_3_horizontal"];
        if (function) {
            ctx->dwt_forward_h_pipeline = [ctx->device newComputePipelineStateWithFunction:function
                                                                                      error:&error];
        }
        
        function = [ctx->library newFunctionWithName:@"dwt_inverse_5_3_horizontal"];
        if (function) {
            ctx->dwt_inverse_h_pipeline = [ctx->device newComputePipelineStateWithFunction:function
                                                                                      error:&error];
        }
        
        /* Quantization pipeline */
        function = [ctx->library newFunctionWithName:@"quantize_coefficients"];
        if (function) {
            ctx->quantize_pipeline = [ctx->device newComputePipelineStateWithFunction:function
                                                                                 error:&error];
        }
        
        return true;
    }
}

void opj_metal_cleanup(opj_metal_context* ctx)
{
    if (!ctx) return;
    
    @autoreleasepool {
        /* Release pipelines */
        ctx->mct_encode_pipeline = nil;
        ctx->mct_decode_pipeline = nil;
        ctx->dwt_forward_h_pipeline = nil;
        ctx->dwt_forward_v_pipeline = nil;
        ctx->dwt_inverse_h_pipeline = nil;
        ctx->dwt_inverse_v_pipeline = nil;
        ctx->quantize_pipeline = nil;
        
        /* Release library and queue */
        ctx->library = nil;
        ctx->queue = nil;
        ctx->device = nil;
        
        /* Clear buffer pool */
        [ctx->buffer_pool removeAllObjects];
        ctx->buffer_pool = nil;
        
        free(ctx);
    }
}

/* ========================================================================
 * Buffer Management
 * ======================================================================== */

static id<MTLBuffer> opj_metal_get_buffer(opj_metal_context* ctx, size_t size)
{
    @autoreleasepool {
        /* Try to reuse buffer from pool */
        for (id<MTLBuffer> buffer in ctx->buffer_pool) {
            if ([buffer length] >= size) {
                [ctx->buffer_pool removeObject:buffer];
                return buffer;
            }
        }
        
        /* Create new buffer */
        id<MTLBuffer> buffer = [ctx->device newBufferWithLength:size
                                                         options:MTLResourceStorageModeShared];
        return buffer;
    }
}

static void opj_metal_return_buffer(opj_metal_context* ctx, id<MTLBuffer> buffer)
{
    @autoreleasepool {
        if ([ctx->buffer_pool count] < 16) {  /* Limit pool size */
            [ctx->buffer_pool addObject:buffer];
        }
    }
}

/* ========================================================================
 * MCT Operations
 * ======================================================================== */

bool opj_metal_mct_encode(
    opj_metal_context* ctx,
    OPJ_INT32* c0,
    OPJ_INT32* c1,
    OPJ_INT32* c2,
    OPJ_SIZE_T n)
{
    if (!ctx || !ctx->enabled || n < ctx->min_size_for_gpu) {
        return false;  /* Use CPU path */
    }
    
    @autoreleasepool {
        size_t buffer_size = n * sizeof(OPJ_INT32);
        
        /* Create Metal buffers */
        id<MTLBuffer> buffer0 = opj_metal_get_buffer(ctx, buffer_size);
        id<MTLBuffer> buffer1 = opj_metal_get_buffer(ctx, buffer_size);
        id<MTLBuffer> buffer2 = opj_metal_get_buffer(ctx, buffer_size);
        
        if (!buffer0 || !buffer1 || !buffer2) {
            return false;
        }
        
        /* Copy data to GPU */
        memcpy([buffer0 contents], c0, buffer_size);
        memcpy([buffer1 contents], c1, buffer_size);
        memcpy([buffer2 contents], c2, buffer_size);
        
        /* Create command buffer and encoder */
        id<MTLCommandBuffer> commandBuffer = [ctx->queue commandBuffer];
        id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];
        
        /* Set pipeline and buffers */
        [encoder setComputePipelineState:ctx->mct_encode_pipeline];
        [encoder setBuffer:buffer0 offset:0 atIndex:0];
        [encoder setBuffer:buffer1 offset:0 atIndex:1];
        [encoder setBuffer:buffer2 offset:0 atIndex:2];
        
        /* Set thread count parameter */
        uint32_t count = (uint32_t)n;
        [encoder setBytes:&count length:sizeof(count) atIndex:3];
        
        /* Calculate thread groups */
        NSUInteger threadExecutionWidth = [ctx->mct_encode_pipeline threadExecutionWidth];
        MTLSize threadsPerGroup = MTLSizeMake(threadExecutionWidth, 1, 1);
        MTLSize numThreadgroups = MTLSizeMake((n + threadExecutionWidth - 1) / threadExecutionWidth, 1, 1);
        
        /* Dispatch */
        [encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
        [encoder endEncoding];
        
        /* Commit and wait */
        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];
        
        /* Copy results back */
        memcpy(c0, [buffer0 contents], buffer_size);
        memcpy(c1, [buffer1 contents], buffer_size);
        memcpy(c2, [buffer2 contents], buffer_size);
        
        /* Return buffers to pool */
        opj_metal_return_buffer(ctx, buffer0);
        opj_metal_return_buffer(ctx, buffer1);
        opj_metal_return_buffer(ctx, buffer2);
        
        return true;
    }
}

bool opj_metal_mct_decode(
    opj_metal_context* ctx,
    OPJ_INT32* c0,
    OPJ_INT32* c1,
    OPJ_INT32* c2,
    OPJ_SIZE_T n)
{
    if (!ctx || !ctx->enabled || n < ctx->min_size_for_gpu) {
        return false;
    }
    
    /* Similar implementation to encode, but using mct_decode_pipeline */
    @autoreleasepool {
        size_t buffer_size = n * sizeof(OPJ_INT32);
        
        id<MTLBuffer> buffer0 = opj_metal_get_buffer(ctx, buffer_size);
        id<MTLBuffer> buffer1 = opj_metal_get_buffer(ctx, buffer_size);
        id<MTLBuffer> buffer2 = opj_metal_get_buffer(ctx, buffer_size);
        
        if (!buffer0 || !buffer1 || !buffer2) {
            return false;
        }
        
        memcpy([buffer0 contents], c0, buffer_size);
        memcpy([buffer1 contents], c1, buffer_size);
        memcpy([buffer2 contents], c2, buffer_size);
        
        id<MTLCommandBuffer> commandBuffer = [ctx->queue commandBuffer];
        id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];
        
        [encoder setComputePipelineState:ctx->mct_decode_pipeline];
        [encoder setBuffer:buffer0 offset:0 atIndex:0];
        [encoder setBuffer:buffer1 offset:0 atIndex:1];
        [encoder setBuffer:buffer2 offset:0 atIndex:2];
        
        uint32_t count = (uint32_t)n;
        [encoder setBytes:&count length:sizeof(count) atIndex:3];
        
        NSUInteger threadExecutionWidth = [ctx->mct_decode_pipeline threadExecutionWidth];
        MTLSize threadsPerGroup = MTLSizeMake(threadExecutionWidth, 1, 1);
        MTLSize numThreadgroups = MTLSizeMake((n + threadExecutionWidth - 1) / threadExecutionWidth, 1, 1);
        
        [encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
        [encoder endEncoding];
        
        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];
        
        memcpy(c0, [buffer0 contents], buffer_size);
        memcpy(c1, [buffer1 contents], buffer_size);
        memcpy(c2, [buffer2 contents], buffer_size);
        
        opj_metal_return_buffer(ctx, buffer0);
        opj_metal_return_buffer(ctx, buffer1);
        opj_metal_return_buffer(ctx, buffer2);
        
        return true;
    }
}

/* ========================================================================
 * Configuration
 * ======================================================================== */

void opj_metal_set_enabled(opj_metal_context* ctx, bool enabled)
{
    if (ctx) {
        ctx->enabled = enabled;
    }
}

bool opj_metal_is_available(opj_metal_context* ctx)
{
    return ctx != NULL && ctx->enabled;
}

void opj_metal_set_min_size(opj_metal_context* ctx, size_t min_pixels)
{
    if (ctx) {
        ctx->min_size_for_gpu = min_pixels;
    }
}

const char* opj_metal_get_device_name(opj_metal_context* ctx)
{
    if (!ctx || !ctx->device) {
        return "No GPU";
    }
    
    @autoreleasepool {
        return [[ctx->device name] UTF8String];
    }
}

#endif /* __APPLE__ && HAVE_METAL */
