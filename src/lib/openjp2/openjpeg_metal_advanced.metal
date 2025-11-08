/*
 * Advanced Metal GPU Kernels for OpenJPEG
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Extended Metal 4 shaders with advanced optimizations
 */

#include <metal_stdlib>
#include <metal_compute>
using namespace metal;

/* ========================================================================
 * Advanced MCT with Tiling
 * ======================================================================== */

/**
 * Tiled MCT for better cache locality
 * Processes tiles of pixels to maximize L1 cache hits
 */
kernel void mct_encode_tiled(
    device int32_t* c0 [[buffer(0)]],
    device int32_t* c1 [[buffer(1)]],
    device int32_t* c2 [[buffer(2)]],
    constant uint& width [[buffer(3)]],
    constant uint& height [[buffer(4)]],
    uint2 tile_id [[threadgroup_position_in_grid]],
    uint2 thread_id [[thread_position_in_threadgroup]])
{
    const uint TILE_SIZE = 16;
    uint2 gid = tile_id * TILE_SIZE + thread_id;
    
    if (gid.x >= width || gid.y >= height) return;
    
    uint idx = gid.y * width + gid.x;
    
    int32_t r = c0[idx];
    int32_t g = c1[idx];
    int32_t b = c2[idx];
    
    /* Y = (R + 2*G + B) >> 2 */
    int32_t y = (r + (g << 1) + b) >> 2;
    int32_t u = b - g;
    int32_t v = r - g;
    
    c0[idx] = y;
    c1[idx] = u;
    c2[idx] = v;
}

/**
 * Vectorized MCT using SIMD within GPU
 * Processes 4 pixels per thread using vector types
 */
kernel void mct_encode_vectorized(
    device int4* c0 [[buffer(0)]],
    device int4* c1 [[buffer(1)]],
    device int4* c2 [[buffer(2)]],
    constant uint& n [[buffer(3)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= n) return;
    
    int4 r = c0[gid];
    int4 g = c1[gid];
    int4 b = c2[gid];
    
    /* Vectorized operations on 4 pixels at once */
    int4 y = (r + (g << 1) + b) >> 2;
    int4 u = b - g;
    int4 v = r - g;
    
    c0[gid] = y;
    c1[gid] = u;
    c2[gid] = v;
}

/* ========================================================================
 * Advanced DWT with Shared Memory
 * ======================================================================== */

/**
 * 2D DWT using threadgroup shared memory
 * Significantly reduces global memory access
 */
kernel void dwt_2d_shared_memory(
    device const int32_t* input [[buffer(0)]],
    device int32_t* output_ll [[buffer(1)]],
    device int32_t* output_lh [[buffer(2)]],
    device int32_t* output_hl [[buffer(3)]],
    device int32_t* output_hh [[buffer(4)]],
    constant uint& width [[buffer(5)]],
    constant uint& height [[buffer(6)]],
    threadgroup int32_t* shared [[threadgroup(0)]],
    uint2 gid [[thread_position_in_grid]],
    uint2 lid [[thread_position_in_threadgroup]])
{
    const uint TILE_SIZE = 16;
    const uint SHARED_SIZE = TILE_SIZE + 2; /* With halo for filter */
    
    /* Load tile into shared memory with halo */
    uint2 tile_origin = gid - lid;
    uint2 load_id = tile_origin + lid;
    
    if (load_id.x < width && load_id.y < height) {
        shared[lid.y * SHARED_SIZE + lid.x] = input[load_id.y * width + load_id.x];
    }
    
    /* Load right/bottom halo */
    if (lid.x == TILE_SIZE - 1 && load_id.x + 1 < width) {
        shared[lid.y * SHARED_SIZE + TILE_SIZE] = input[load_id.y * width + load_id.x + 1];
    }
    if (lid.y == TILE_SIZE - 1 && load_id.y + 1 < height) {
        shared[TILE_SIZE * SHARED_SIZE + lid.x] = input[(load_id.y + 1) * width + load_id.x];
    }
    
    threadgroup_barrier(mem_flags::mem_threadgroup);
    
    /* Compute 2D wavelet from shared memory */
    if (gid.x < width - 1 && gid.y < height - 1) {
        int32_t c00 = shared[lid.y * SHARED_SIZE + lid.x];
        int32_t c01 = shared[lid.y * SHARED_SIZE + lid.x + 1];
        int32_t c10 = shared[(lid.y + 1) * SHARED_SIZE + lid.x];
        int32_t c11 = shared[(lid.y + 1) * SHARED_SIZE + lid.x + 1];
        
        /* Haar wavelet for simplicity */
        int32_t ll = (c00 + c01 + c10 + c11) >> 2;
        int32_t lh = (c00 + c01 - c10 - c11) >> 2;
        int32_t hl = (c00 - c01 + c10 - c11) >> 2;
        int32_t hh = (c00 - c01 - c10 + c11) >> 2;
        
        uint out_x = gid.x >> 1;
        uint out_y = gid.y >> 1;
        uint half_width = (width + 1) >> 1;
        uint out_idx = out_y * half_width + out_x;
        
        if ((gid.x & 1) == 0 && (gid.y & 1) == 0) {
            output_ll[out_idx] = ll;
            output_lh[out_idx] = lh;
            output_hl[out_idx] = hl;
            output_hh[out_idx] = hh;
        }
    }
}

/**
 * Separable DWT using transpose for cache efficiency
 */
kernel void dwt_separable_horizontal(
    device const int32_t* input [[buffer(0)]],
    device int32_t* output [[buffer(1)]],
    constant uint& width [[buffer(2)]],
    constant uint& height [[buffer(3)]],
    threadgroup int32_t* shared [[threadgroup(0)]],
    uint gid [[thread_position_in_grid]],
    uint lid [[thread_position_in_threadgroup]])
{
    if (gid >= height) return;
    
    const uint row_offset = gid * width;
    const uint half_width = (width + 1) >> 1;
    
    /* Load row cooperatively */
    for (uint i = lid; i < width; i += 256) {
        shared[i] = input[row_offset + i];
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);
    
    /* 5/3 lifting scheme */
    for (uint i = lid; i < half_width; i += 256) {
        uint even_idx = i * 2;
        uint odd_idx = even_idx + 1;
        
        if (odd_idx < width) {
            /* Predict */
            int32_t predict = (shared[even_idx] + shared[min(even_idx + 2, width - 1)]) >> 1;
            int32_t high = shared[odd_idx] - predict;
            
            /* Update */
            int32_t update = high >> 2;
            int32_t low = shared[even_idx] + update;
            
            output[gid * width + i] = low;
            output[gid * width + half_width + i] = high;
        }
    }
}

/* ========================================================================
 * Quantization with Deadzone
 * ======================================================================== */

/**
 * Advanced quantization with deadzone and rounding
 */
kernel void quantize_with_deadzone(
    device const int32_t* input [[buffer(0)]],
    device int32_t* output [[buffer(1)]],
    constant float& step_size [[buffer(2)]],
    constant float& deadzone [[buffer(3)]],
    constant uint& n [[buffer(4)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= n) return;
    
    float value = float(input[gid]);
    float abs_value = abs(value);
    
    /* Apply deadzone */
    if (abs_value < deadzone) {
        output[gid] = 0;
        return;
    }
    
    /* Quantize with rounding */
    float quantized = (abs_value - deadzone) / step_size;
    int32_t result = int32_t(quantized + 0.5f);
    
    /* Restore sign */
    output[gid] = (value < 0) ? -result : result;
}

/**
 * Vectorized quantization for throughput
 */
kernel void quantize_vectorized(
    device const int4* input [[buffer(0)]],
    device int4* output [[buffer(1)]],
    constant float& step_size [[buffer(2)]],
    constant uint& n [[buffer(3)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= n) return;
    
    int4 values = input[gid];
    float4 f_values = float4(values);
    float4 quantized = f_values / step_size;
    
    /* Round to nearest */
    quantized = floor(quantized + 0.5f);
    
    output[gid] = int4(quantized);
}

/* ========================================================================
 * Rate-Distortion Optimization Helpers
 * ======================================================================== */

/**
 * Compute distortion metrics (MSE) in parallel
 */
kernel void compute_mse(
    device const int32_t* original [[buffer(0)]],
    device const int32_t* reconstructed [[buffer(1)]],
    device float* partial_mse [[buffer(2)]],
    constant uint& n [[buffer(3)]],
    uint gid [[thread_position_in_grid]],
    uint lid [[thread_position_in_threadgroup]],
    uint tid [[threadgroup_position_in_grid]])
{
    threadgroup float shared_sum[256];
    
    float sum = 0.0f;
    uint idx = gid;
    
    /* Each thread accumulates differences */
    while (idx < n) {
        float diff = float(original[idx] - reconstructed[idx]);
        sum += diff * diff;
        idx += 256 * 1024; /* Grid-stride loop */
    }
    
    shared_sum[lid] = sum;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    
    /* Reduction within threadgroup */
    for (uint stride = 128; stride > 0; stride >>= 1) {
        if (lid < stride) {
            shared_sum[lid] += shared_sum[lid + stride];
        }
        threadgroup_barrier(mem_flags::mem_threadgroup);
    }
    
    /* Write partial result */
    if (lid == 0) {
        partial_mse[tid] = shared_sum[0];
    }
}

/* ========================================================================
 * Histogram and Statistics
 * ======================================================================== */

/**
 * Parallel histogram computation for coefficient analysis
 */
kernel void compute_histogram(
    device const int32_t* data [[buffer(0)]],
    device atomic_uint* histogram [[buffer(1)]],
    constant uint& n [[buffer(2)]],
    constant int& min_value [[buffer(3)]],
    constant int& max_value [[buffer(4)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= n) return;
    
    int32_t value = data[gid];
    
    /* Clamp to histogram range */
    if (value < min_value || value > max_value) return;
    
    uint bin = uint(value - min_value);
    atomic_fetch_add_explicit(&histogram[bin], 1, memory_order_relaxed);
}

/* ========================================================================
 * Memory Operations
 * ======================================================================== */

/**
 * High-throughput memory copy using GPU
 */
kernel void memcpy_gpu(
    device const int32_t* src [[buffer(0)]],
    device int32_t* dst [[buffer(1)]],
    constant uint& n [[buffer(2)]],
    uint gid [[thread_position_in_grid]])
{
    uint idx = gid * 4; /* Process 4 elements per thread */
    
    if (idx + 3 < n) {
        /* Vectorized load/store */
        int4 data = *((device int4*)(&src[idx]));
        *((device int4*)(&dst[idx])) = data;
    } else {
        /* Handle remainder */
        for (uint i = idx; i < n && i < idx + 4; i++) {
            dst[i] = src[i];
        }
    }
}

/**
 * Transpose for cache-efficient column access
 */
kernel void transpose_cache_efficient(
    device const int32_t* input [[buffer(0)]],
    device int32_t* output [[buffer(1)]],
    constant uint& width [[buffer(2)]],
    constant uint& height [[buffer(3)]],
    threadgroup int32_t* tile [[threadgroup(0)]],
    uint2 gid [[thread_position_in_grid]],
    uint2 lid [[thread_position_in_threadgroup]])
{
    const uint TILE_SIZE = 32;
    
    /* Load tile */
    uint2 in_idx = gid;
    if (in_idx.x < width && in_idx.y < height) {
        tile[lid.y * TILE_SIZE + lid.x] = input[in_idx.y * width + in_idx.x];
    }
    
    threadgroup_barrier(mem_flags::mem_threadgroup);
    
    /* Store transposed */
    uint2 out_idx = uint2(gid.y, gid.x);
    if (out_idx.x < height && out_idx.y < width) {
        output[out_idx.y * height + out_idx.x] = tile[lid.x * TILE_SIZE + lid.y];
    }
}

/* ========================================================================
 * Batch Processing
 * ======================================================================== */

/**
 * Batch MCT for processing multiple images simultaneously
 */
kernel void mct_encode_batch(
    device int32_t* c0_batch [[buffer(0)]],
    device int32_t* c1_batch [[buffer(1)]],
    device int32_t* c2_batch [[buffer(2)]],
    constant uint* offsets [[buffer(3)]],
    constant uint* sizes [[buffer(4)]],
    constant uint& num_images [[buffer(5)]],
    uint2 gid [[thread_position_in_grid]])
{
    uint image_id = gid.y;
    uint pixel_id = gid.x;
    
    if (image_id >= num_images) return;
    
    uint offset = offsets[image_id];
    uint size = sizes[image_id];
    
    if (pixel_id >= size) return;
    
    uint idx = offset + pixel_id;
    
    int32_t r = c0_batch[idx];
    int32_t g = c1_batch[idx];
    int32_t b = c2_batch[idx];
    
    int32_t y = (r + (g << 1) + b) >> 2;
    int32_t u = b - g;
    int32_t v = r - g;
    
    c0_batch[idx] = y;
    c1_batch[idx] = u;
    c2_batch[idx] = v;
}
