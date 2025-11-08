/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * ARM NEON optimizations for Multi-Component Transform
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * All rights reserved.
 */

#ifdef __ARM_NEON

#include "opj_includes.h"
#include <arm_neon.h>

/* Forward reversible MCT - ARM NEON optimized version */
void opj_mct_encode_neon(
    OPJ_INT32* OPJ_RESTRICT c0,
    OPJ_INT32* OPJ_RESTRICT c1,
    OPJ_INT32* OPJ_RESTRICT c2,
    OPJ_SIZE_T n)
{
    OPJ_SIZE_T i;
    const OPJ_SIZE_T len = n;

    /* Process 4 pixels at a time using NEON (128-bit SIMD) */
    for (i = 0; i < (len & ~3U); i += 4) {
        int32x4_t r = vld1q_s32(&c0[i]);  /* Load R values */
        int32x4_t g = vld1q_s32(&c1[i]);  /* Load G values */
        int32x4_t b = vld1q_s32(&c2[i]);  /* Load B values */
        
        /* Y = (R + 2*G + B) >> 2 */
        int32x4_t y = vaddq_s32(g, g);    /* G * 2 */
        y = vaddq_s32(y, b);              /* + B */
        y = vaddq_s32(y, r);              /* + R */
        y = vshrq_n_s32(y, 2);            /* >> 2 */
        
        /* U = B - G */
        int32x4_t u = vsubq_s32(b, g);
        
        /* V = R - G */
        int32x4_t v = vsubq_s32(r, g);
        
        /* Store results */
        vst1q_s32(&c0[i], y);
        vst1q_s32(&c1[i], u);
        vst1q_s32(&c2[i], v);
    }

    /* Handle remaining pixels (0-3) with scalar code */
    for (; i < len; ++i) {
        OPJ_INT32 r = c0[i];
        OPJ_INT32 g = c1[i];
        OPJ_INT32 b = c2[i];
        OPJ_INT32 y = (r + (g * 2) + b) >> 2;
        OPJ_INT32 u = b - g;
        OPJ_INT32 v = r - g;
        c0[i] = y;
        c1[i] = u;
        c2[i] = v;
    }
}

/* Inverse reversible MCT - ARM NEON optimized version */
void opj_mct_decode_neon(
    OPJ_INT32* OPJ_RESTRICT c0,
    OPJ_INT32* OPJ_RESTRICT c1,
    OPJ_INT32* OPJ_RESTRICT c2,
    OPJ_SIZE_T n)
{
    OPJ_SIZE_T i;
    const OPJ_SIZE_T len = n;

    /* Process 4 pixels at a time using NEON */
    for (i = 0; i < (len & ~3U); i += 4) {
        int32x4_t y = vld1q_s32(&c0[i]);  /* Load Y values */
        int32x4_t u = vld1q_s32(&c1[i]);  /* Load U values */
        int32x4_t v = vld1q_s32(&c2[i]);  /* Load V values */
        
        /* G = Y - ((U + V) >> 2) */
        int32x4_t uv_sum = vaddq_s32(u, v);
        int32x4_t g = vsubq_s32(y, vshrq_n_s32(uv_sum, 2));
        
        /* R = V + G */
        int32x4_t r = vaddq_s32(v, g);
        
        /* B = U + G */
        int32x4_t b = vaddq_s32(u, g);
        
        /* Store results */
        vst1q_s32(&c0[i], r);
        vst1q_s32(&c1[i], g);
        vst1q_s32(&c2[i], b);
    }

    /* Handle remaining pixels with scalar code */
    for (; i < len; ++i) {
        OPJ_INT32 y = c0[i];
        OPJ_INT32 u = c1[i];
        OPJ_INT32 v = c2[i];
        OPJ_INT32 g = y - ((u + v) >> 2);
        OPJ_INT32 r = v + g;
        OPJ_INT32 b = u + g;
        c0[i] = r;
        c1[i] = g;
        c2[i] = b;
    }
}

/* Forward irreversible MCT - ARM NEON optimized (floating-point) */
void opj_mct_encode_real_neon(
    OPJ_FLOAT32* OPJ_RESTRICT c0,
    OPJ_FLOAT32* OPJ_RESTRICT c1,
    OPJ_FLOAT32* OPJ_RESTRICT c2,
    OPJ_SIZE_T n)
{
    OPJ_SIZE_T i;
    const OPJ_SIZE_T len = n;
    
    /* NEON floating-point registers */
    const float32x4_t c0_coeff = vdupq_n_f32(0.299f);
    const float32x4_t c1_coeff = vdupq_n_f32(0.587f);
    const float32x4_t c2_coeff = vdupq_n_f32(0.114f);
    
    const float32x4_t cb_r_coeff = vdupq_n_f32(-0.16875f);
    const float32x4_t cb_g_coeff = vdupq_n_f32(-0.33126f);
    const float32x4_t cb_b_coeff = vdupq_n_f32(0.5f);
    
    const float32x4_t cr_r_coeff = vdupq_n_f32(0.5f);
    const float32x4_t cr_g_coeff = vdupq_n_f32(-0.41869f);
    const float32x4_t cr_b_coeff = vdupq_n_f32(-0.08131f);

    /* Process 4 values at a time */
    for (i = 0; i < (len & ~3U); i += 4) {
        float32x4_t r = vld1q_f32(&c0[i]);
        float32x4_t g = vld1q_f32(&c1[i]);
        float32x4_t b = vld1q_f32(&c2[i]);
        
        /* Y = 0.299*R + 0.587*G + 0.114*B */
        float32x4_t y = vmulq_f32(r, c0_coeff);
        y = vmlaq_f32(y, g, c1_coeff);  /* Fused multiply-add */
        y = vmlaq_f32(y, b, c2_coeff);
        
        /* Cb = -0.16875*R - 0.33126*G + 0.5*B */
        float32x4_t cb = vmulq_f32(r, cb_r_coeff);
        cb = vmlaq_f32(cb, g, cb_g_coeff);
        cb = vmlaq_f32(cb, b, cb_b_coeff);
        
        /* Cr = 0.5*R - 0.41869*G - 0.08131*B */
        float32x4_t cr = vmulq_f32(r, cr_r_coeff);
        cr = vmlaq_f32(cr, g, cr_g_coeff);
        cr = vmlaq_f32(cr, b, cr_b_coeff);
        
        vst1q_f32(&c0[i], y);
        vst1q_f32(&c1[i], cb);
        vst1q_f32(&c2[i], cr);
    }

    /* Handle remaining elements */
    for (; i < len; ++i) {
        OPJ_FLOAT32 r = c0[i];
        OPJ_FLOAT32 g = c1[i];
        OPJ_FLOAT32 b = c2[i];
        OPJ_FLOAT32 y =  0.299f * r + 0.587f * g + 0.114f * b;
        OPJ_FLOAT32 cb = -0.16875f * r - 0.33126f * g + 0.5f * b;
        OPJ_FLOAT32 cr =  0.5f * r - 0.41869f * g - 0.08131f * b;
        c0[i] = y;
        c1[i] = cb;
        c2[i] = cr;
    }
}

/* Inverse irreversible MCT - ARM NEON optimized */
void opj_mct_decode_real_neon(
    OPJ_FLOAT32* OPJ_RESTRICT c0,
    OPJ_FLOAT32* OPJ_RESTRICT c1,
    OPJ_FLOAT32* OPJ_RESTRICT c2,
    OPJ_SIZE_T n)
{
    OPJ_SIZE_T i;
    const OPJ_SIZE_T len = n;
    
    const float32x4_t cr_r_coeff = vdupq_n_f32(1.402f);
    const float32x4_t cb_g_coeff = vdupq_n_f32(-0.34413f);
    const float32x4_t cr_g_coeff = vdupq_n_f32(-0.71414f);
    const float32x4_t cb_b_coeff = vdupq_n_f32(1.772f);

    /* Process 4 values at a time */
    for (i = 0; i < (len & ~3U); i += 4) {
        float32x4_t y  = vld1q_f32(&c0[i]);
        float32x4_t cb = vld1q_f32(&c1[i]);
        float32x4_t cr = vld1q_f32(&c2[i]);
        
        /* R = Y + 1.402*Cr */
        float32x4_t r = vmlaq_f32(y, cr, cr_r_coeff);
        
        /* G = Y - 0.34413*Cb - 0.71414*Cr */
        float32x4_t g = y;
        g = vmlaq_f32(g, cb, cb_g_coeff);
        g = vmlaq_f32(g, cr, cr_g_coeff);
        
        /* B = Y + 1.772*Cb */
        float32x4_t b = vmlaq_f32(y, cb, cb_b_coeff);
        
        vst1q_f32(&c0[i], r);
        vst1q_f32(&c1[i], g);
        vst1q_f32(&c2[i], b);
    }

    /* Handle remaining elements */
    for (; i < len; ++i) {
        OPJ_FLOAT32 y  = c0[i];
        OPJ_FLOAT32 cb = c1[i];
        OPJ_FLOAT32 cr = c2[i];
        OPJ_FLOAT32 r = y + 1.402f * cr;
        OPJ_FLOAT32 g = y - 0.34413f * cb - 0.71414f * cr;
        OPJ_FLOAT32 b = y + 1.772f * cb;
        c0[i] = r;
        c1[i] = g;
        c2[i] = b;
    }
}

#endif /* __ARM_NEON */
