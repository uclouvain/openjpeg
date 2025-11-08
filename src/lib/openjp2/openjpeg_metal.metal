/*
 * OpenJPEG Metal GPU Acceleration Kernels
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Metal 4 shaders for JPEG 2000 encoding/decoding on Apple Silicon
 */

#include <metal_stdlib>
using namespace metal;

/* ========================================================================
 * Multi-Component Transform (MCT) Kernels
 * ======================================================================== */

/**
 * Forward RGB to YUV color transform (reversible)
 * Each thread processes one pixel
 */
kernel void mct_encode_reversible(
    device int32_t* c0 [[buffer(0)]],  // R -> Y
    device int32_t* c1 [[buffer(1)]],  // G -> U
    device int32_t* c2 [[buffer(2)]],  // B -> V
    constant uint& n   [[buffer(3)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= n) return;
    
    int32_t r = c0[gid];
    int32_t g = c1[gid];
    int32_t b = c2[gid];
    
    // Y = (R + 2*G + B) >> 2
    int32_t y = (r + (g << 1) + b) >> 2;
    // U = B - G
    int32_t u = b - g;
    // V = R - G
    int32_t v = r - g;
    
    c0[gid] = y;
    c1[gid] = u;
    c2[gid] = v;
}

/**
 * Inverse YUV to RGB color transform (reversible)
 */
kernel void mct_decode_reversible(
    device int32_t* c0 [[buffer(0)]],  // Y -> R
    device int32_t* c1 [[buffer(1)]],  // U -> G
    device int32_t* c2 [[buffer(2)]],  // V -> B
    constant uint& n   [[buffer(3)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= n) return;
    
    int32_t y = c0[gid];
    int32_t u = c1[gid];
    int32_t v = c2[gid];
    
    // G = Y - ((U + V) >> 2)
    int32_t g = y - ((u + v) >> 2);
    // R = V + G
    int32_t r = v + g;
    // B = U + G
    int32_t b = u + g;
    
    c0[gid] = r;
    c1[gid] = g;
    c2[gid] = b;
}

/**
 * Forward RGB to YCbCr color transform (irreversible)
 */
kernel void mct_encode_irreversible(
    device float* c0 [[buffer(0)]],  // R -> Y
    device float* c1 [[buffer(1)]],  // G -> Cb
    device float* c2 [[buffer(2)]],  // B -> Cr
    constant uint& n [[buffer(3)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= n) return;
    
    float r = c0[gid];
    float g = c1[gid];
    float b = c2[gid];
    
    float y  =  0.299f * r + 0.587f * g + 0.114f * b;
    float cb = -0.16875f * r - 0.33126f * g + 0.5f * b;
    float cr =  0.5f * r - 0.41869f * g - 0.08131f * b;
    
    c0[gid] = y;
    c1[gid] = cb;
    c2[gid] = cr;
}

/**
 * Inverse YCbCr to RGB color transform (irreversible)
 */
kernel void mct_decode_irreversible(
    device float* c0 [[buffer(0)]],  // Y -> R
    device float* c1 [[buffer(1)]],  // Cb -> G
    device float* c2 [[buffer(2)]],  // Cr -> B
    constant uint& n [[buffer(3)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= n) return;
    
    float y  = c0[gid];
    float cb = c1[gid];
    float cr = c2[gid];
    
    float r = y + 1.402f * cr;
    float g = y - 0.34413f * cb - 0.71414f * cr;
    float b = y + 1.772f * cb;
    
    c0[gid] = r;
    c1[gid] = g;
    c2[gid] = b;
}

/* ========================================================================
 * Discrete Wavelet Transform (DWT) Kernels
 * ======================================================================== */

/**
 * 1D Horizontal 5/3 wavelet transform (lifting scheme) - Forward
 * Processes one row per thread using shared memory for efficiency
 */
kernel void dwt_forward_5_3_horizontal(
    device const int32_t* input  [[buffer(0)]],
    device int32_t* output_low   [[buffer(1)]],
    device int32_t* output_high  [[buffer(2)]],
    constant uint& width         [[buffer(3)]],
    constant uint& height        [[buffer(4)]],
    threadgroup int32_t* shared  [[threadgroup(0)]],
    uint gid [[thread_position_in_grid]],
    uint lid [[thread_position_in_threadgroup]])
{
    if (gid >= height) return;
    
    const uint row_offset = gid * width;
    const uint half_width = (width + 1) / 2;
    
    // Load row into shared memory with boundary handling
    for (uint i = lid; i < width; i += 256) {
        shared[i] = input[row_offset + i];
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);
    
    // Split into even (low) and odd (high) samples
    // Even: 0, 2, 4, ...
    // Odd:  1, 3, 5, ...
    
    // Predict step: high[i] -= (low[i] + low[i+1]) / 2
    for (uint i = lid; i < half_width - 1; i += 256) {
        uint even_idx = i * 2;
        uint odd_idx = even_idx + 1;
        if (odd_idx < width) {
            int32_t predict = (shared[even_idx] + shared[even_idx + 2]) >> 1;
            output_high[gid * half_width + i] = shared[odd_idx] - predict;
        }
    }
    
    // Handle last odd sample
    if (lid == 0 && (width & 1) == 0) {
        output_high[gid * half_width + half_width - 1] = 
            shared[width - 1] - shared[width - 2];
    }
    
    threadgroup_barrier(mem_flags::mem_threadgroup);
    
    // Update step: low[i] += (high[i-1] + high[i] + 2) / 4
    if (lid == 0) {
        output_low[gid * half_width] = shared[0] + 
            ((output_high[gid * half_width] + 2) >> 2);
    }
    
    for (uint i = lid + 1; i < half_width; i += 256) {
        uint high_idx = gid * half_width;
        int32_t update = (output_high[high_idx + i - 1] + 
                         output_high[high_idx + i] + 2) >> 2;
        output_low[gid * half_width + i] = shared[i * 2] + update;
    }
}

/**
 * 1D Vertical 5/3 wavelet transform - Forward
 * Processes one column per thread
 */
kernel void dwt_forward_5_3_vertical(
    device const int32_t* input_low  [[buffer(0)]],
    device const int32_t* input_high [[buffer(1)]],
    device int32_t* output           [[buffer(2)]],
    constant uint& width             [[buffer(3)]],
    constant uint& height            [[buffer(4)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= width) return;
    
    const uint half_height = (height + 1) / 2;
    
    // Interleave low and high frequency bands back into spatial domain
    for (uint row = 0; row < half_height - 1; row++) {
        uint even_pos = row * 2;
        uint odd_pos = even_pos + 1;
        
        output[even_pos * width + gid] = input_low[row * width + gid];
        if (odd_pos < height) {
            output[odd_pos * width + gid] = input_high[row * width + gid];
        }
    }
    
    // Handle last row if odd height
    if ((height & 1) == 1) {
        output[(height - 1) * width + gid] = 
            input_low[(half_height - 1) * width + gid];
    }
}

/**
 * 1D Horizontal 5/3 inverse wavelet transform
 */
kernel void dwt_inverse_5_3_horizontal(
    device const int32_t* input_low  [[buffer(0)]],
    device const int32_t* input_high [[buffer(1)]],
    device int32_t* output           [[buffer(2)]],
    constant uint& width             [[buffer(3)]],
    constant uint& height            [[buffer(4)]],
    threadgroup int32_t* shared_low  [[threadgroup(0)]],
    threadgroup int32_t* shared_high [[threadgroup(1)]],
    uint gid [[thread_position_in_grid]],
    uint lid [[thread_position_in_threadgroup]])
{
    if (gid >= height) return;
    
    const uint half_width = (width + 1) / 2;
    const uint row_offset = gid * half_width;
    
    // Load low and high bands into shared memory
    for (uint i = lid; i < half_width; i += 256) {
        shared_low[i] = input_low[row_offset + i];
        if (i < width - half_width) {
            shared_high[i] = input_high[row_offset + i];
        }
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);
    
    // Inverse update step
    for (uint i = lid + 1; i < half_width; i += 256) {
        int32_t update = (shared_high[i - 1] + shared_high[i] + 2) >> 2;
        shared_low[i] -= update;
    }
    if (lid == 0) {
        shared_low[0] -= (shared_high[0] + 2) >> 2;
    }
    
    threadgroup_barrier(mem_flags::mem_threadgroup);
    
    // Inverse predict step and interleave
    for (uint i = lid; i < half_width - 1; i += 256) {
        int32_t predict = (shared_low[i] + shared_low[i + 1]) >> 1;
        output[gid * width + i * 2] = shared_low[i];
        output[gid * width + i * 2 + 1] = shared_high[i] + predict;
    }
    
    // Handle last sample
    if (lid == 0) {
        if ((width & 1) == 1) {
            output[gid * width + width - 1] = shared_low[half_width - 1];
        } else {
            output[gid * width + width - 1] = 
                shared_high[half_width - 1] + shared_low[half_width - 1];
        }
    }
}

/* ========================================================================
 * Quantization Kernels
 * ======================================================================== */

/**
 * Parallel quantization of wavelet coefficients
 */
kernel void quantize_coefficients(
    device const int32_t* input     [[buffer(0)]],
    device int32_t* output          [[buffer(1)]],
    constant float& step_size       [[buffer(2)]],
    constant uint& n                [[buffer(3)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= n) return;
    
    float value = float(input[gid]);
    output[gid] = int32_t(value / step_size);
}

/**
 * Parallel dequantization
 */
kernel void dequantize_coefficients(
    device const int32_t* input     [[buffer(0)]],
    device int32_t* output          [[buffer(1)]],
    constant float& step_size       [[buffer(2)]],
    constant uint& n                [[buffer(3)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= n) return;
    
    output[gid] = int32_t(float(input[gid]) * step_size);
}

/* ========================================================================
 * Utility Kernels
 * ======================================================================== */

/**
 * Parallel memory copy with potential format conversion
 */
kernel void parallel_memcpy(
    device const int32_t* src [[buffer(0)]],
    device int32_t* dst       [[buffer(1)]],
    constant uint& n          [[buffer(2)]],
    uint gid [[thread_position_in_grid]])
{
    if (gid >= n) return;
    dst[gid] = src[gid];
}

/**
 * Transpose operation for cache-efficient processing
 */
kernel void transpose_2d(
    device const int32_t* input [[buffer(0)]],
    device int32_t* output      [[buffer(1)]],
    constant uint& width        [[buffer(2)]],
    constant uint& height       [[buffer(3)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= width || gid.y >= height) return;
    
    output[gid.x * height + gid.y] = input[gid.y * width + gid.x];
}
