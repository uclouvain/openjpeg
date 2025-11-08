/*
 * OpenJPEG - Phase 4 Optimized Lifting Scheme Wavelet
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 *
 * In-place 5/3 and 9/7 DWT with NEON acceleration
 */

#include "opj_includes.h"
#include <string.h>

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* 5/3 Lifting coefficients (reversible) */
#define ALPHA_53  (-0.5f)
#define BETA_53   (0.25f)

/* 9/7 Lifting coefficients (irreversible) */
#define ALPHA_97  (-1.586134342f)
#define BETA_97   (-0.052980118f)
#define GAMMA_97  (0.882911075f)
#define DELTA_97  (0.443506852f)
#define K_97      (1.230174105f)
#define K_INV_97  (1.0f / K_97)

typedef struct {
    OPJ_INT32* buffer;
    int size;
    int allocated;
} lifting_buffer_t;

static lifting_buffer_t g_lift_buf = {0};

/* Allocate lifting buffer */
static OPJ_BOOL ensure_buffer(int size) {
    if (size <= g_lift_buf.allocated) {
        return OPJ_TRUE;
    }
    
    if (g_lift_buf.buffer) {
        opj_aligned_free(g_lift_buf.buffer);
    }
    
    g_lift_buf.buffer = (OPJ_INT32*)opj_aligned_malloc(size * sizeof(OPJ_INT32));
    if (!g_lift_buf.buffer) {
        g_lift_buf.allocated = 0;
        return OPJ_FALSE;
    }
    
    g_lift_buf.allocated = size;
    return OPJ_TRUE;
}

/* Cleanup lifting buffer */
void opj_lifting_cleanup(void) {
    if (g_lift_buf.buffer) {
        opj_aligned_free(g_lift_buf.buffer);
        g_lift_buf.buffer = NULL;
        g_lift_buf.allocated = 0;
    }
}

#ifdef __ARM_NEON

/* NEON: 5/3 Forward Lifting - Predict Step */
static void lifting_53_predict_neon(OPJ_INT32* s, OPJ_INT32* d, int len) {
    int i = 0;
    
    /* First sample */
    d[0] -= s[0];
    
    /* NEON: Process 4 samples at a time */
    for (i = 1; i <= len - 5; i += 4) {
        int32x4_t s_prev = vld1q_s32(s + i - 1);
        int32x4_t s_curr = vld1q_s32(s + i);
        int32x4_t d_curr = vld1q_s32(d + i);
        
        /* d[i] -= (s[i-1] + s[i]) >> 1 */
        int32x4_t sum = vaddq_s32(s_prev, s_curr);
        int32x4_t predict = vshrq_n_s32(sum, 1);
        int32x4_t result = vsubq_s32(d_curr, predict);
        
        vst1q_s32(d + i, result);
    }
    
    /* Scalar fallback for remaining */
    for (; i < len; i++) {
        d[i] -= (s[i - 1] + s[i]) >> 1;
    }
    
    /* Last sample */
    if (len > 0) {
        d[len] -= s[len - 1];
    }
}

/* NEON: 5/3 Forward Lifting - Update Step */
static void lifting_53_update_neon(OPJ_INT32* s, OPJ_INT32* d, int len) {
    int i = 0;
    
    /* NEON: Process 4 samples at a time */
    for (; i <= len - 4; i += 4) {
        int32x4_t s_curr = vld1q_s32(s + i);
        int32x4_t d_curr = vld1q_s32(d + i);
        int32x4_t d_next = vld1q_s32(d + i + 1);
        
        /* s[i] += (d[i] + d[i+1] + 2) >> 2 */
        int32x4_t sum = vaddq_s32(d_curr, d_next);
        sum = vaddq_s32(sum, vdupq_n_s32(2));
        int32x4_t update = vshrq_n_s32(sum, 2);
        int32x4_t result = vaddq_s32(s_curr, update);
        
        vst1q_s32(s + i, result);
    }
    
    /* Scalar fallback */
    for (; i < len; i++) {
        s[i] += (d[i] + d[i + 1] + 2) >> 2;
    }
}

/* NEON: 9/7 Forward Lifting - Step 1 */
static void lifting_97_step1_neon(float* s, float* d, int len, float coeff) {
    int i = 0;
    float32x4_t vcoeff = vdupq_n_f32(coeff);
    
    /* First sample */
    d[0] += coeff * (s[0] + s[0]);
    
    for (; i <= len - 5; i += 4) {
        /* Load data */
        float32x4_t s_prev = vld1q_f32(s + i - 1);
        float32x4_t s_curr = vld1q_f32(s + i);
        float32x4_t d_curr = vld1q_f32(d + i);
        
        /* d[i] += coeff * (s[i-1] + s[i]) */
        float32x4_t sum = vaddq_f32(s_prev, s_curr);
        d_curr = vmlaq_f32(d_curr, sum, vcoeff);
        
        vst1q_f32(d + i, d_curr);
    }
    
    /* Scalar fallback */
    for (; i < len; i++) {
        d[i] += coeff * (s[i - 1] + s[i]);
    }
    
    /* Last sample */
    if (len > 0) {
        d[len] += coeff * (s[len - 1] + s[len - 1]);
    }
}

/* NEON: 9/7 Forward Lifting - Step 2 */
static void lifting_97_step2_neon(float* s, float* d, int len, float coeff) {
    int i = 0;
    float32x4_t vcoeff = vdupq_n_f32(coeff);
    
    for (; i <= len - 4; i += 4) {
        float32x4_t s_curr = vld1q_f32(s + i);
        float32x4_t d_curr = vld1q_f32(d + i);
        float32x4_t d_next = vld1q_f32(d + i + 1);
        
        /* s[i] += coeff * (d[i] + d[i+1]) */
        float32x4_t sum = vaddq_f32(d_curr, d_next);
        s_curr = vmlaq_f32(s_curr, sum, vcoeff);
        
        vst1q_f32(s + i, s_curr);
    }
    
    /* Scalar fallback */
    for (; i < len; i++) {
        s[i] += coeff * (d[i] + d[i + 1]);
    }
}

/* NEON: 9/7 Scaling */
static void lifting_97_scale_neon(float* s, float* d, int len_s, int len_d) {
    int i = 0;
    float32x4_t k = vdupq_n_f32(K_97);
    float32x4_t k_inv = vdupq_n_f32(K_INV_97);
    
    /* Scale low-pass (s) */
    for (; i <= len_s - 4; i += 4) {
        float32x4_t vals = vld1q_f32(s + i);
        vals = vmulq_f32(vals, k_inv);
        vst1q_f32(s + i, vals);
    }
    for (; i < len_s; i++) {
        s[i] *= K_INV_97;
    }
    
    /* Scale high-pass (d) */
    i = 0;
    for (; i <= len_d - 4; i += 4) {
        float32x4_t vals = vld1q_f32(d + i);
        vals = vmulq_f32(vals, k);
        vst1q_f32(d + i, vals);
    }
    for (; i < len_d; i++) {
        d[i] *= K_97;
    }
}

#else
/* Scalar implementations */
static void lifting_53_predict_scalar(OPJ_INT32* s, OPJ_INT32* d, int len) {
    d[0] -= s[0];
    for (int i = 1; i < len; i++) {
        d[i] -= (s[i - 1] + s[i]) >> 1;
    }
    if (len > 0) {
        d[len] -= s[len - 1];
    }
}

static void lifting_53_update_scalar(OPJ_INT32* s, OPJ_INT32* d, int len) {
    for (int i = 0; i < len; i++) {
        s[i] += (d[i] + d[i + 1] + 2) >> 2;
    }
}

static void lifting_97_step1_scalar(float* s, float* d, int len, float coeff) {
    d[0] += coeff * (s[0] + s[0]);
    for (int i = 1; i < len; i++) {
        d[i] += coeff * (s[i - 1] + s[i]);
    }
    if (len > 0) {
        d[len] += coeff * (s[len - 1] + s[len - 1]);
    }
}

static void lifting_97_step2_scalar(float* s, float* d, int len, float coeff) {
    for (int i = 0; i < len; i++) {
        s[i] += coeff * (d[i] + d[i + 1]);
    }
}

static void lifting_97_scale_scalar(float* s, float* d, int len_s, int len_d) {
    for (int i = 0; i < len_s; i++) {
        s[i] *= K_INV_97;
    }
    for (int i = 0; i < len_d; i++) {
        d[i] *= K_97;
    }
}
#endif

/* 5/3 Forward DWT - 1D (in-place) */
OPJ_BOOL opj_dwt_53_forward_1d(OPJ_INT32* data, int len) {
    if (len < 2) return OPJ_TRUE;
    
    int len_s = (len + 1) / 2;  /* Low-pass length */
    int len_d = len / 2;         /* High-pass length */
    
    if (!ensure_buffer(len)) {
        return OPJ_FALSE;
    }
    
    /* Split into even (s) and odd (d) samples */
    OPJ_INT32* s = g_lift_buf.buffer;
    OPJ_INT32* d = s + len_s;
    
    for (int i = 0; i < len_s; i++) {
        s[i] = data[2 * i];
    }
    for (int i = 0; i < len_d; i++) {
        d[i] = data[2 * i + 1];
    }
    
#ifdef __ARM_NEON
    /* Predict: d -= (s_prev + s) / 2 */
    lifting_53_predict_neon(s, d, len_d);
    
    /* Update: s += (d + d_next) / 4 */
    lifting_53_update_neon(s, d, len_s);
#else
    lifting_53_predict_scalar(s, d, len_d);
    lifting_53_update_scalar(s, d, len_s);
#endif
    
    /* Interleave back: [s0, s1, ..., d0, d1, ...] */
    memcpy(data, s, len_s * sizeof(OPJ_INT32));
    memcpy(data + len_s, d, len_d * sizeof(OPJ_INT32));
    
    return OPJ_TRUE;
}

/* 9/7 Forward DWT - 1D (in-place) */
OPJ_BOOL opj_dwt_97_forward_1d(float* data, int len) {
    if (len < 2) return OPJ_TRUE;
    
    int len_s = (len + 1) / 2;
    int len_d = len / 2;
    
    if (!ensure_buffer(len * sizeof(float) / sizeof(OPJ_INT32))) {
        return OPJ_FALSE;
    }
    
    float* s = (float*)g_lift_buf.buffer;
    float* d = s + len_s;
    
    /* Split */
    for (int i = 0; i < len_s; i++) {
        s[i] = data[2 * i];
    }
    for (int i = 0; i < len_d; i++) {
        d[i] = data[2 * i + 1];
    }
    
#ifdef __ARM_NEON
    /* 4-step lifting scheme */
    lifting_97_step1_neon(s, d, len_d, ALPHA_97);
    lifting_97_step2_neon(s, d, len_s, BETA_97);
    lifting_97_step1_neon(s, d, len_d, GAMMA_97);
    lifting_97_step2_neon(s, d, len_s, DELTA_97);
    lifting_97_scale_neon(s, d, len_s, len_d);
#else
    lifting_97_step1_scalar(s, d, len_d, ALPHA_97);
    lifting_97_step2_scalar(s, d, len_s, BETA_97);
    lifting_97_step1_scalar(s, d, len_d, GAMMA_97);
    lifting_97_step2_scalar(s, d, len_s, DELTA_97);
    lifting_97_scale_scalar(s, d, len_s, len_d);
#endif
    
    /* Interleave */
    memcpy(data, s, len_s * sizeof(float));
    memcpy(data + len_s, d, len_d * sizeof(float));
    
    return OPJ_TRUE;
}

/* 2D Forward DWT (separable) */
OPJ_BOOL opj_dwt_forward_2d(OPJ_INT32* data, int width, int height, 
                           OPJ_BOOL reversible) {
    /* Horizontal transform */
    for (int y = 0; y < height; y++) {
        if (reversible) {
            if (!opj_dwt_53_forward_1d(data + y * width, width)) {
                return OPJ_FALSE;
            }
        } else {
            if (!opj_dwt_97_forward_1d((float*)(data + y * width), width)) {
                return OPJ_FALSE;
            }
        }
    }
    
    /* Vertical transform (need transpose or column access) */
    if (!ensure_buffer(height)) {
        return OPJ_FALSE;
    }
    
    for (int x = 0; x < width; x++) {
        /* Extract column */
        for (int y = 0; y < height; y++) {
            g_lift_buf.buffer[y] = data[y * width + x];
        }
        
        /* Transform */
        if (reversible) {
            if (!opj_dwt_53_forward_1d(g_lift_buf.buffer, height)) {
                return OPJ_FALSE;
            }
        } else {
            if (!opj_dwt_97_forward_1d((float*)g_lift_buf.buffer, height)) {
                return OPJ_FALSE;
            }
        }
        
        /* Write back column */
        for (int y = 0; y < height; y++) {
            data[y * width + x] = g_lift_buf.buffer[y];
        }
    }
    
    return OPJ_TRUE;
}

/* Multi-level DWT */
OPJ_BOOL opj_dwt_forward_multilevel(OPJ_INT32* data, int width, int height,
                                   int levels, OPJ_BOOL reversible) {
    int curr_w = width;
    int curr_h = height;
    
    for (int level = 0; level < levels; level++) {
        if (curr_w < 2 || curr_h < 2) break;
        
        /* Transform only LL subband from previous level */
        if (!opj_dwt_forward_2d(data, curr_w, curr_h, reversible)) {
            return OPJ_FALSE;
        }
        
        /* Next level operates on LL subband (top-left quarter) */
        curr_w = (curr_w + 1) / 2;
        curr_h = (curr_h + 1) / 2;
    }
    
    return OPJ_TRUE;
}

/* Get subband energy (for rate allocation) */
float opj_dwt_subband_energy(const OPJ_INT32* data, int width, int height,
                             int subband_x, int subband_y,
                             int subband_w, int subband_h) {
    double energy = 0.0;
    
    for (int y = subband_y; y < subband_y + subband_h && y < height; y++) {
        for (int x = subband_x; x < subband_x + subband_w && x < width; x++) {
            OPJ_INT32 val = data[y * width + x];
            energy += (double)val * val;
        }
    }
    
    return (float)(energy / (subband_w * subband_h));
}
