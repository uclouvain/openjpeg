/*
 * Advanced Metal Async Pipeline
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Triple-buffering, async compute, and CPU-GPU overlap
 */

#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#include "opj_includes.h"

/* ========================================================================
 * Triple-Buffered Pipeline
 * ======================================================================== */

#define MAX_BUFFERS_IN_FLIGHT 3

typedef struct {
    id<MTLBuffer> input_buffers[MAX_BUFFERS_IN_FLIGHT];
    id<MTLBuffer> output_buffers[MAX_BUFFERS_IN_FLIGHT];
    id<MTLCommandBuffer> command_buffers[MAX_BUFFERS_IN_FLIGHT];
    dispatch_semaphore_t inflight_semaphore;
    uint32_t current_buffer_index;
    size_t buffer_size;
    BOOL initialized;
} opj_metal_triple_buffer_t;

static opj_metal_triple_buffer_t g_triple_buffer = {0};

/**
 * Initialize triple-buffering system
 */
int opj_metal_triple_buffer_init(
    id<MTLDevice> device,
    size_t buffer_size)
{
    if (g_triple_buffer.initialized) {
        return 0; /* Already initialized */
    }
    
    @autoreleasepool {
        /* Create semaphore for synchronization */
        g_triple_buffer.inflight_semaphore = 
            dispatch_semaphore_create(MAX_BUFFERS_IN_FLIGHT);
        
        /* Allocate triple buffers */
        for (int i = 0; i < MAX_BUFFERS_IN_FLIGHT; i++) {
            g_triple_buffer.input_buffers[i] = 
                [device newBufferWithLength:buffer_size
                                   options:MTLResourceStorageModeShared];
            
            g_triple_buffer.output_buffers[i] = 
                [device newBufferWithLength:buffer_size
                                   options:MTLResourceStorageModeShared];
            
            if (!g_triple_buffer.input_buffers[i] || 
                !g_triple_buffer.output_buffers[i]) {
                return -1;
            }
        }
        
        g_triple_buffer.buffer_size = buffer_size;
        g_triple_buffer.current_buffer_index = 0;
        g_triple_buffer.initialized = YES;
    }
    
    return 0;
}

/**
 * Get next available buffer for processing
 */
void opj_metal_triple_buffer_wait_and_acquire(
    void** input_ptr,
    void** output_ptr,
    uint32_t* buffer_index)
{
    /* Wait for available buffer */
    dispatch_semaphore_wait(g_triple_buffer.inflight_semaphore, 
                           DISPATCH_TIME_FOREVER);
    
    *buffer_index = g_triple_buffer.current_buffer_index;
    
    *input_ptr = [g_triple_buffer.input_buffers[*buffer_index] contents];
    *output_ptr = [g_triple_buffer.output_buffers[*buffer_index] contents];
    
    /* Advance to next buffer */
    g_triple_buffer.current_buffer_index = 
        (*buffer_index + 1) % MAX_BUFFERS_IN_FLIGHT;
}

/**
 * Signal buffer completion
 */
void opj_metal_triple_buffer_signal_completion(void)
{
    dispatch_semaphore_signal(g_triple_buffer.inflight_semaphore);
}

/**
 * Cleanup triple-buffering resources
 */
void opj_metal_triple_buffer_cleanup(void)
{
    if (!g_triple_buffer.initialized) {
        return;
    }
    
    @autoreleasepool {
        /* Wait for all in-flight work to complete */
        for (int i = 0; i < MAX_BUFFERS_IN_FLIGHT; i++) {
            dispatch_semaphore_wait(g_triple_buffer.inflight_semaphore,
                                   DISPATCH_TIME_FOREVER);
        }
        
        /* Release buffers */
        for (int i = 0; i < MAX_BUFFERS_IN_FLIGHT; i++) {
            g_triple_buffer.input_buffers[i] = nil;
            g_triple_buffer.output_buffers[i] = nil;
        }
        
        g_triple_buffer.initialized = NO;
    }
}

/* ========================================================================
 * Async Command Buffer Management
 * ======================================================================== */

typedef struct {
    id<MTLCommandQueue> compute_queue;
    id<MTLCommandQueue> transfer_queue;
    dispatch_queue_t completion_queue;
} opj_metal_async_queues_t;

static opj_metal_async_queues_t g_async_queues = {0};

/**
 * Initialize separate compute and transfer queues
 */
int opj_metal_async_queues_init(id<MTLDevice> device)
{
    @autoreleasepool {
        g_async_queues.compute_queue = [device newCommandQueue];
        g_async_queues.transfer_queue = [device newCommandQueue];
        
        if (!g_async_queues.compute_queue || !g_async_queues.transfer_queue) {
            return -1;
        }
        
        /* Create high-priority dispatch queue for completions */
        g_async_queues.completion_queue = 
            dispatch_queue_create("com.openjpeg.metal.completion",
                                 DISPATCH_QUEUE_CONCURRENT);
        
        return 0;
    }
}

/**
 * Submit async compute command
 */
void opj_metal_submit_async_compute(
    id<MTLComputeCommandEncoder> encoder,
    void (^completion_handler)(void))
{
    @autoreleasepool {
        [encoder endEncoding];
        
        id<MTLCommandBuffer> commandBuffer = [g_async_queues.compute_queue commandBuffer];
        
        /* Add completion handler */
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            dispatch_async(g_async_queues.completion_queue, ^{
                completion_handler();
                opj_metal_triple_buffer_signal_completion();
            });
        }];
        
        [commandBuffer commit];
    }
}

/* ========================================================================
 * CPU-GPU Overlap Pipeline
 * ======================================================================== */

typedef enum {
    STAGE_IDLE,
    STAGE_CPU_PREPROCESS,
    STAGE_GPU_COMPUTE,
    STAGE_CPU_POSTPROCESS
} pipeline_stage_t;

typedef struct {
    pipeline_stage_t current_stage;
    dispatch_queue_t cpu_preprocess_queue;
    dispatch_queue_t cpu_postprocess_queue;
    uint64_t stage_start_time;
    uint64_t stage_times[4];
} opj_overlap_pipeline_t;

static opj_overlap_pipeline_t g_overlap_pipeline = {0};

/**
 * Initialize overlapped pipeline
 */
void opj_overlap_pipeline_init(void)
{
    g_overlap_pipeline.cpu_preprocess_queue = 
        dispatch_queue_create("com.openjpeg.cpu.preprocess",
                             DISPATCH_QUEUE_CONCURRENT);
    
    g_overlap_pipeline.cpu_postprocess_queue = 
        dispatch_queue_create("com.openjpeg.cpu.postprocess",
                             DISPATCH_QUEUE_CONCURRENT);
    
    g_overlap_pipeline.current_stage = STAGE_IDLE;
}

/**
 * Execute preprocessing on CPU while GPU is busy
 */
void opj_overlap_cpu_preprocess(
    const void* input_data,
    size_t input_size,
    void (^preprocess_block)(const void*, size_t),
    void (^completion)(void))
{
    dispatch_async(g_overlap_pipeline.cpu_preprocess_queue, ^{
        uint64_t start = mach_absolute_time();
        
        preprocess_block(input_data, input_size);
        
        uint64_t end = mach_absolute_time();
        g_overlap_pipeline.stage_times[STAGE_CPU_PREPROCESS] = end - start;
        
        if (completion) {
            completion();
        }
    });
}

/**
 * Execute postprocessing on CPU after GPU completes
 */
void opj_overlap_cpu_postprocess(
    const void* gpu_output,
    size_t output_size,
    void (^postprocess_block)(const void*, size_t),
    void (^completion)(void))
{
    dispatch_async(g_overlap_pipeline.cpu_postprocess_queue, ^{
        uint64_t start = mach_absolute_time();
        
        postprocess_block(gpu_output, output_size);
        
        uint64_t end = mach_absolute_time();
        g_overlap_pipeline.stage_times[STAGE_CPU_POSTPROCESS] = end - start;
        
        if (completion) {
            completion();
        }
    });
}

/* ========================================================================
 * Advanced Pipeline Scheduling
 * ======================================================================== */

typedef struct {
    float mct_weight;
    float dwt_weight;
    float t1_weight;
    float t2_weight;
} opj_workload_weights_t;

/**
 * Analyze workload and determine optimal scheduling
 */
void opj_analyze_workload(
    uint32_t width,
    uint32_t height,
    uint32_t num_components,
    opj_workload_weights_t* weights)
{
    uint32_t total_pixels = width * height;
    
    /* MCT weight scales with pixels and components */
    weights->mct_weight = (float)(total_pixels * num_components) / 1000000.0f;
    
    /* DWT weight scales with wavelet levels (log scale) */
    uint32_t max_levels = 0;
    uint32_t dim = width > height ? width : height;
    while (dim > 1) {
        dim >>= 1;
        max_levels++;
    }
    weights->dwt_weight = max_levels * (float)total_pixels / 1000000.0f;
    
    /* T1 weight scales with codeblocks */
    uint32_t num_codeblocks = (width / 64) * (height / 64);
    weights->t1_weight = (float)num_codeblocks / 100.0f;
    
    /* T2 weight is relatively small */
    weights->t2_weight = 0.1f;
}

/**
 * Decide whether to use GPU or CPU for each component
 */
typedef struct {
    BOOL use_gpu_for_mct;
    BOOL use_gpu_for_dwt;
    BOOL use_gpu_for_t1;
    BOOL use_gpu_for_t2;
} opj_scheduling_decision_t;

void opj_make_scheduling_decision(
    const opj_workload_weights_t* weights,
    opj_scheduling_decision_t* decision)
{
    /* Threshold: GPU advantageous above these weights */
    const float MCT_GPU_THRESHOLD = 2.0f;  /* ~2 MP */
    const float DWT_GPU_THRESHOLD = 1.0f;  /* ~500K pixels */
    const float T1_GPU_THRESHOLD = 0.5f;   /* ~50 codeblocks */
    const float T2_GPU_THRESHOLD = 10.0f;  /* Almost never for T2 */
    
    decision->use_gpu_for_mct = weights->mct_weight > MCT_GPU_THRESHOLD;
    decision->use_gpu_for_dwt = weights->dwt_weight > DWT_GPU_THRESHOLD;
    decision->use_gpu_for_t1 = weights->t1_weight > T1_GPU_THRESHOLD;
    decision->use_gpu_for_t2 = weights->t2_weight > T2_GPU_THRESHOLD;
}

/* ========================================================================
 * Memory Prefetching
 * ======================================================================== */

/**
 * Prefetch data into L2 cache before GPU transfer
 */
void opj_metal_prefetch_data(
    const void* data,
    size_t size)
{
    const size_t CACHE_LINE = 64;
    const uint8_t* ptr = (const uint8_t*)data;
    
    /* Touch every cache line */
    for (size_t i = 0; i < size; i += CACHE_LINE) {
        __builtin_prefetch(&ptr[i], 0, 3); /* prefetch for read, high locality */
    }
}

/**
 * Stream data from GPU with prefetching
 */
void opj_metal_stream_from_gpu(
    id<MTLBuffer> gpu_buffer,
    void* cpu_dest,
    size_t size)
{
    @autoreleasepool {
        /* Get GPU buffer contents */
        const void* gpu_ptr = [gpu_buffer contents];
        
        /* Prefetch from GPU memory */
        opj_metal_prefetch_data(gpu_ptr, size);
        
        /* Copy with NEON if available */
        #ifdef __ARM_NEON
        const size_t NEON_CHUNK = 64;
        size_t full_chunks = size / NEON_CHUNK;
        
        const uint8_t* src = (const uint8_t*)gpu_ptr;
        uint8_t* dst = (uint8_t*)cpu_dest;
        
        for (size_t i = 0; i < full_chunks; i++) {
            /* Load 64 bytes */
            uint8x16_t v0 = vld1q_u8(src + 0);
            uint8x16_t v1 = vld1q_u8(src + 16);
            uint8x16_t v2 = vld1q_u8(src + 32);
            uint8x16_t v3 = vld1q_u8(src + 48);
            
            /* Store 64 bytes */
            vst1q_u8(dst + 0, v0);
            vst1q_u8(dst + 16, v1);
            vst1q_u8(dst + 32, v2);
            vst1q_u8(dst + 48, v3);
            
            src += NEON_CHUNK;
            dst += NEON_CHUNK;
        }
        
        /* Copy remainder */
        size_t remainder = size % NEON_CHUNK;
        if (remainder > 0) {
            memcpy(dst, src, remainder);
        }
        #else
        memcpy(cpu_dest, gpu_ptr, size);
        #endif
    }
}

/* ========================================================================
 * Performance Statistics
 * ======================================================================== */

typedef struct {
    uint64_t total_frames_processed;
    uint64_t avg_cpu_preprocess_ns;
    uint64_t avg_gpu_compute_ns;
    uint64_t avg_cpu_postprocess_ns;
    uint64_t avg_total_latency_ns;
    float cpu_gpu_overlap_percent;
} opj_metal_async_stats_t;

static opj_metal_async_stats_t g_async_stats = {0};

void opj_metal_async_update_stats(
    uint64_t cpu_pre_ns,
    uint64_t gpu_ns,
    uint64_t cpu_post_ns,
    uint64_t total_ns)
{
    g_async_stats.total_frames_processed++;
    
    /* Running average */
    uint64_t n = g_async_stats.total_frames_processed;
    g_async_stats.avg_cpu_preprocess_ns = 
        (g_async_stats.avg_cpu_preprocess_ns * (n-1) + cpu_pre_ns) / n;
    g_async_stats.avg_gpu_compute_ns = 
        (g_async_stats.avg_gpu_compute_ns * (n-1) + gpu_ns) / n;
    g_async_stats.avg_cpu_postprocess_ns = 
        (g_async_stats.avg_cpu_postprocess_ns * (n-1) + cpu_post_ns) / n;
    g_async_stats.avg_total_latency_ns = 
        (g_async_stats.avg_total_latency_ns * (n-1) + total_ns) / n;
    
    /* Calculate overlap percentage */
    uint64_t sequential_time = cpu_pre_ns + gpu_ns + cpu_post_ns;
    if (sequential_time > 0) {
        g_async_stats.cpu_gpu_overlap_percent = 
            100.0f * (1.0f - (float)total_ns / (float)sequential_time);
    }
}

void opj_metal_async_get_stats(opj_metal_async_stats_t* stats)
{
    *stats = g_async_stats;
}

void opj_metal_async_reset_stats(void)
{
    memset(&g_async_stats, 0, sizeof(g_async_stats));
}

/* ========================================================================
 * Adaptive Quality Adjustment
 * ======================================================================== */

/**
 * Dynamically adjust quality based on performance
 */
typedef struct {
    float target_fps;
    float current_fps;
    uint32_t quality_level;  /* 0-100 */
    BOOL adaptive_mode;
} opj_adaptive_quality_t;

static opj_adaptive_quality_t g_adaptive_quality = {
    .target_fps = 30.0f,
    .current_fps = 0.0f,
    .quality_level = 80,
    .adaptive_mode = NO
};

void opj_metal_enable_adaptive_quality(float target_fps)
{
    g_adaptive_quality.target_fps = target_fps;
    g_adaptive_quality.adaptive_mode = YES;
}

uint32_t opj_metal_get_adaptive_quality(void)
{
    if (!g_adaptive_quality.adaptive_mode) {
        return g_adaptive_quality.quality_level;
    }
    
    /* Adjust quality based on FPS */
    if (g_adaptive_quality.current_fps < g_adaptive_quality.target_fps * 0.9f) {
        /* Too slow, reduce quality */
        if (g_adaptive_quality.quality_level > 20) {
            g_adaptive_quality.quality_level -= 5;
        }
    } else if (g_adaptive_quality.current_fps > g_adaptive_quality.target_fps * 1.1f) {
        /* Too fast, increase quality */
        if (g_adaptive_quality.quality_level < 95) {
            g_adaptive_quality.quality_level += 5;
        }
    }
    
    return g_adaptive_quality.quality_level;
}

void opj_metal_update_current_fps(float fps)
{
    /* Exponential moving average */
    const float ALPHA = 0.2f;
    g_adaptive_quality.current_fps = 
        ALPHA * fps + (1.0f - ALPHA) * g_adaptive_quality.current_fps;
}
