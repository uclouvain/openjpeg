/*
 * OpenJPEG - Phase 4 Adaptive Quantization with NEON
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 *
 * Perceptual quantization with visual masking
 */

#include "opj_includes.h"
#include <math.h>
#include <string.h>

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Visual frequency weights (CSF - Contrast Sensitivity Function) */
static const float csf_weights[10][10] = {
    {1.00f, 0.95f, 0.90f, 0.85f, 0.80f, 0.75f, 0.70f, 0.65f, 0.60f, 0.55f},
    {0.95f, 0.92f, 0.88f, 0.84f, 0.80f, 0.75f, 0.70f, 0.65f, 0.60f, 0.55f},
    {0.90f, 0.88f, 0.85f, 0.82f, 0.78f, 0.74f, 0.70f, 0.65f, 0.60f, 0.55f},
    {0.85f, 0.84f, 0.82f, 0.79f, 0.76f, 0.72f, 0.68f, 0.64f, 0.60f, 0.55f},
    {0.80f, 0.80f, 0.78f, 0.76f, 0.73f, 0.70f, 0.66f, 0.62f, 0.58f, 0.54f},
    {0.75f, 0.75f, 0.74f, 0.72f, 0.70f, 0.67f, 0.64f, 0.60f, 0.56f, 0.52f},
    {0.70f, 0.70f, 0.70f, 0.68f, 0.66f, 0.64f, 0.61f, 0.58f, 0.54f, 0.50f},
    {0.65f, 0.65f, 0.65f, 0.64f, 0.62f, 0.60f, 0.58f, 0.55f, 0.52f, 0.48f},
    {0.60f, 0.60f, 0.60f, 0.60f, 0.58f, 0.56f, 0.54f, 0.52f, 0.49f, 0.46f},
    {0.55f, 0.55f, 0.55f, 0.55f, 0.54f, 0.52f, 0.50f, 0.48f, 0.46f, 0.44f}
};

typedef struct {
    float* variance_map;
    float* activity_map;
    float* masking_map;
    int width;
    int height;
    int initialized;
} opj_aq_context_t;

static opj_aq_context_t g_aq_context = {0};

/* Initialize adaptive quantization context */
OPJ_BOOL opj_aq_init(int width, int height) {
    if (g_aq_context.initialized) {
        opj_aq_cleanup();
    }
    
    size_t size = width * height * sizeof(float);
    g_aq_context.variance_map = (float*)opj_aligned_malloc(size);
    g_aq_context.activity_map = (float*)opj_aligned_malloc(size);
    g_aq_context.masking_map = (float*)opj_aligned_malloc(size);
    
    if (!g_aq_context.variance_map || !g_aq_context.activity_map || 
        !g_aq_context.masking_map) {
        opj_aq_cleanup();
        return OPJ_FALSE;
    }
    
    g_aq_context.width = width;
    g_aq_context.height = height;
    g_aq_context.initialized = 1;
    
    memset(g_aq_context.variance_map, 0, size);
    memset(g_aq_context.activity_map, 0, size);
    memset(g_aq_context.masking_map, 0, size);
    
    return OPJ_TRUE;
}

/* Cleanup adaptive quantization */
void opj_aq_cleanup(void) {
    if (g_aq_context.variance_map) {
        opj_aligned_free(g_aq_context.variance_map);
        g_aq_context.variance_map = NULL;
    }
    if (g_aq_context.activity_map) {
        opj_aligned_free(g_aq_context.activity_map);
        g_aq_context.activity_map = NULL;
    }
    if (g_aq_context.masking_map) {
        opj_aligned_free(g_aq_context.masking_map);
        g_aq_context.masking_map = NULL;
    }
    g_aq_context.initialized = 0;
}

#ifdef __ARM_NEON
/* NEON: Calculate local variance (8 pixels at a time) */
static void calculate_variance_neon(const OPJ_INT32* data, float* variance,
                                   int width, int height) {
    for (int y = 1; y < height - 1; y++) {
        int x = 1;
        for (; x <= width - 9; x += 8) {
            int idx = y * width + x;
            
            /* Load 3x3 neighborhood for 8 pixels */
            float32x4_t sum_low = vdupq_n_f32(0.0f);
            float32x4_t sum_high = vdupq_n_f32(0.0f);
            float32x4_t sum2_low = vdupq_n_f32(0.0f);
            float32x4_t sum2_high = vdupq_n_f32(0.0f);
            
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int offset = (y + dy) * width + (x + dx);
                    
                    /* Load 8 values and convert to float */
                    int32x4_t val_low = vld1q_s32(data + offset);
                    int32x4_t val_high = vld1q_s32(data + offset + 4);
                    
                    float32x4_t fval_low = vcvtq_f32_s32(val_low);
                    float32x4_t fval_high = vcvtq_f32_s32(val_high);
                    
                    sum_low = vaddq_f32(sum_low, fval_low);
                    sum_high = vaddq_f32(sum_high, fval_high);
                    
                    sum2_low = vmlaq_f32(sum2_low, fval_low, fval_low);
                    sum2_high = vmlaq_f32(sum2_high, fval_high, fval_high);
                }
            }
            
            /* Variance = E[X^2] - E[X]^2 */
            float32x4_t mean_low = vmulq_n_f32(sum_low, 1.0f / 9.0f);
            float32x4_t mean_high = vmulq_n_f32(sum_high, 1.0f / 9.0f);
            
            float32x4_t mean2_low = vmulq_f32(mean_low, mean_low);
            float32x4_t mean2_high = vmulq_f32(mean_high, mean_high);
            
            float32x4_t var_low = vsubq_f32(vmulq_n_f32(sum2_low, 1.0f / 9.0f), mean2_low);
            float32x4_t var_high = vsubq_f32(vmulq_n_f32(sum2_high, 1.0f / 9.0f), mean2_high);
            
            vst1q_f32(variance + idx, var_low);
            vst1q_f32(variance + idx + 4, var_high);
        }
        
        /* Handle remaining pixels */
        for (; x < width - 1; x++) {
            int idx = y * width + x;
            float sum = 0.0f, sum2 = 0.0f;
            
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    float val = (float)data[(y + dy) * width + (x + dx)];
                    sum += val;
                    sum2 += val * val;
                }
            }
            
            float mean = sum / 9.0f;
            variance[idx] = (sum2 / 9.0f) - (mean * mean);
        }
    }
}

/* NEON: Calculate activity map (edge detection) */
static void calculate_activity_neon(const OPJ_INT32* data, float* activity,
                                   int width, int height) {
    for (int y = 1; y < height - 1; y++) {
        int x = 1;
        for (; x <= width - 9; x += 8) {
            int idx = y * width + x;
            
            /* Load center pixels */
            int32x4_t center_low = vld1q_s32(data + idx);
            int32x4_t center_high = vld1q_s32(data + idx + 4);
            
            /* Sobel operator for edge detection */
            int32x4_t gx_low = vdupq_n_s32(0);
            int32x4_t gx_high = vdupq_n_s32(0);
            int32x4_t gy_low = vdupq_n_s32(0);
            int32x4_t gy_high = vdupq_n_s32(0);
            
            /* Horizontal gradient (Gx) */
            gx_low = vsubq_s32(vld1q_s32(data + idx + 1), vld1q_s32(data + idx - 1));
            gx_high = vsubq_s32(vld1q_s32(data + idx + 5), vld1q_s32(data + idx + 3));
            
            /* Vertical gradient (Gy) */
            gy_low = vsubq_s32(vld1q_s32(data + (y + 1) * width + x), 
                              vld1q_s32(data + (y - 1) * width + x));
            gy_high = vsubq_s32(vld1q_s32(data + (y + 1) * width + x + 4), 
                               vld1q_s32(data + (y - 1) * width + x + 4));
            
            /* Magnitude = sqrt(Gx^2 + Gy^2) ≈ |Gx| + |Gy| */
            float32x4_t mag_low = vaddq_f32(
                vcvtq_f32_s32(vabsq_s32(gx_low)),
                vcvtq_f32_s32(vabsq_s32(gy_low))
            );
            float32x4_t mag_high = vaddq_f32(
                vcvtq_f32_s32(vabsq_s32(gx_high)),
                vcvtq_f32_s32(vabsq_s32(gy_high))
            );
            
            vst1q_f32(activity + idx, mag_low);
            vst1q_f32(activity + idx + 4, mag_high);
        }
        
        /* Scalar fallback */
        for (; x < width - 1; x++) {
            int idx = y * width + x;
            int gx = data[idx + 1] - data[idx - 1];
            int gy = data[(y + 1) * width + x] - data[(y - 1) * width + x];
            activity[idx] = sqrtf((float)(gx * gx + gy * gy));
        }
    }
}

/* NEON: Apply visual masking */
static void apply_masking_neon(float* masking, const float* variance,
                              const float* activity, int width, int height,
                              float strength) {
    float32x4_t vstrength = vdupq_n_f32(strength);
    float32x4_t vone = vdupq_n_f32(1.0f);
    float32x4_t vepsilon = vdupq_n_f32(1e-6f);
    
    int total = width * height;
    int i = 0;
    
    for (; i <= total - 4; i += 4) {
        float32x4_t var = vld1q_f32(variance + i);
        float32x4_t act = vld1q_f32(activity + i);
        
        /* Normalize */
        var = vaddq_f32(var, vepsilon);
        act = vaddq_f32(act, vepsilon);
        
        /* Masking = 1 + strength * (variance^0.5 + activity^0.5) */
        float32x4_t mask = vaddq_f32(vone,
            vmulq_f32(vstrength,
                vaddq_f32(
                    vsqrtq_f32(var),
                    vsqrtq_f32(act)
                )
            )
        );
        
        vst1q_f32(masking + i, mask);
    }
    
    /* Scalar fallback */
    for (; i < total; i++) {
        float var = variance[i] + 1e-6f;
        float act = activity[i] + 1e-6f;
        masking[i] = 1.0f + strength * (sqrtf(var) + sqrtf(act));
    }
}

#else
/* Scalar fallback implementations */
static void calculate_variance_scalar(const OPJ_INT32* data, float* variance,
                                      int width, int height) {
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int idx = y * width + x;
            float sum = 0.0f, sum2 = 0.0f;
            
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    float val = (float)data[(y + dy) * width + (x + dx)];
                    sum += val;
                    sum2 += val * val;
                }
            }
            
            float mean = sum / 9.0f;
            variance[idx] = (sum2 / 9.0f) - (mean * mean);
        }
    }
}

static void calculate_activity_scalar(const OPJ_INT32* data, float* activity,
                                      int width, int height) {
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int idx = y * width + x;
            int gx = data[idx + 1] - data[idx - 1];
            int gy = data[(y + 1) * width + x] - data[(y - 1) * width + x];
            activity[idx] = sqrtf((float)(gx * gx + gy * gy));
        }
    }
}

static void apply_masking_scalar(float* masking, const float* variance,
                                const float* activity, int width, int height,
                                float strength) {
    int total = width * height;
    for (int i = 0; i < total; i++) {
        float var = variance[i] + 1e-6f;
        float act = activity[i] + 1e-6f;
        masking[i] = 1.0f + strength * (sqrtf(var) + sqrtf(act));
    }
}
#endif

/* Compute perceptual quantization weights */
OPJ_BOOL opj_aq_compute_weights(const OPJ_INT32* data, int width, int height,
                                float strength, float* weights) {
    if (!g_aq_context.initialized) {
        if (!opj_aq_init(width, height)) {
            return OPJ_FALSE;
        }
    }
    
#ifdef __ARM_NEON
    calculate_variance_neon(data, g_aq_context.variance_map, width, height);
    calculate_activity_neon(data, g_aq_context.activity_map, width, height);
    apply_masking_neon(g_aq_context.masking_map, g_aq_context.variance_map,
                      g_aq_context.activity_map, width, height, strength);
#else
    calculate_variance_scalar(data, g_aq_context.variance_map, width, height);
    calculate_activity_scalar(data, g_aq_context.activity_map, width, height);
    apply_masking_scalar(g_aq_context.masking_map, g_aq_context.variance_map,
                        g_aq_context.activity_map, width, height, strength);
#endif
    
    /* Copy masking map to output weights */
    memcpy(weights, g_aq_context.masking_map, width * height * sizeof(float));
    
    return OPJ_TRUE;
}

/* Apply CSF weights to subband */
void opj_aq_apply_csf_weights(OPJ_INT32* data, int width, int height,
                              int subband_type, float base_weight) {
    /* Determine CSF matrix size based on subband */
    int csf_size = (width < 10 || height < 10) ? 
                   ((width < height) ? width : height) : 10;
    
    for (int y = 0; y < height && y < csf_size; y++) {
        for (int x = 0; x < width && x < csf_size; x++) {
            float weight = csf_weights[y][x] * base_weight;
            data[y * width + x] = (OPJ_INT32)(data[y * width + x] * weight);
        }
    }
}

/* Adaptive deadzone quantization */
void opj_aq_deadzone_quantize(OPJ_INT32* data, int width, int height,
                              const float* aq_weights, float deadzone_size) {
    int total = width * height;
    
#ifdef __ARM_NEON
    int i = 0;
    float32x4_t vdeadzone = vdupq_n_f32(deadzone_size);
    
    for (; i <= total - 4; i += 4) {
        int32x4_t vals = vld1q_s32(data + i);
        float32x4_t weights = vld1q_f32(aq_weights + i);
        
        /* Adaptive deadzone = deadzone_size / weight */
        float32x4_t adaptive_dz = vdivq_f32(vdeadzone, weights);
        
        /* Convert to float for comparison */
        float32x4_t fvals = vcvtq_f32_s32(vals);
        float32x4_t abs_vals = vabsq_f32(fvals);
        
        /* Zero out values below adaptive deadzone */
        uint32x4_t mask = vcgtq_f32(abs_vals, adaptive_dz);
        int32x4_t result = vandq_s32(vals, vreinterpretq_s32_u32(mask));
        
        vst1q_s32(data + i, result);
    }
    
    /* Scalar fallback */
    for (; i < total; i++) {
        float adaptive_dz = deadzone_size / aq_weights[i];
        if (fabsf((float)data[i]) < adaptive_dz) {
            data[i] = 0;
        }
    }
#else
    for (int i = 0; i < total; i++) {
        float adaptive_dz = deadzone_size / aq_weights[i];
        if (fabsf((float)data[i]) < adaptive_dz) {
            data[i] = 0;
        }
    }
#endif
}
