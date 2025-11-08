/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * ARM NEON optimizations for Discrete Wavelet Transform
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * All rights reserved.
 */

#ifdef __ARM_NEON

#include "opj_includes.h"
#include <arm_neon.h>

/* NEON equivalent macros to SSE2 operations */
#define VREG        int32x4_t
#define LOAD_CST(x) vdupq_n_s32(x)
#define LOAD(x)     vld1q_s32((const int32_t*)(x))
#define LOADU(x)    vld1q_s32((const int32_t*)(x))
#define STORE(x,y)  vst1q_s32((int32_t*)(x),(y))
#define STOREU(x,y) vst1q_s32((int32_t*)(x),(y))
#define ADD(x,y)    vaddq_s32((x),(y))
#define SUB(x,y)    vsubq_s32((x),(y))
#define SAR(x,y)    vshrq_n_s32((x),(y))
#define VREG_INT_COUNT 4

/* NEON doesn't have ADD3, so we implement it */
#define ADD3(x,y,z) ADD(ADD(x,y),z)

/* Number of columns that we can process in parallel with NEON */
#define PARALLEL_COLS_53_NEON  8

/* ========================================================================
 * Forward DWT 5/3 (Reversible)
 * ======================================================================== */

/**
 * Vertical inverse 5x3 wavelet transform for 8 columns in NEON
 * when top-most pixel is on even coordinate
 */
static void opj_idwt53_v_cas0_mcols_NEON(
    OPJ_INT32* tmp,
    const OPJ_INT32 sn,
    const OPJ_INT32 len,
    OPJ_INT32* tiledp_col,
    const OPJ_SIZE_T stride)
{
    const OPJ_INT32* in_even = &tiledp_col[0];
    const OPJ_INT32* in_odd = &tiledp_col[(OPJ_SIZE_T)sn * stride];

    OPJ_INT32 i;
    OPJ_SIZE_T j;
    VREG d1c_0, d1n_0, s1n_0, s0c_0, s0n_0;
    VREG d1c_1, d1n_1, s1n_1, s0c_1, s0n_1;
    const VREG two = LOAD_CST(2);

    assert(len > 1);
    assert(PARALLEL_COLS_53_NEON == 8);
    assert(VREG_INT_COUNT == 4);

    /* Load initial values */
    s1n_0 = LOADU(in_even + 0);
    s1n_1 = LOADU(in_even + VREG_INT_COUNT);
    d1n_0 = LOADU(in_odd);
    d1n_1 = LOADU(in_odd + VREG_INT_COUNT);

    /* s0n = s1n - ((d1n + 1) >> 1); <==> */
    /* s0n = s1n - ((d1n + d1n + 2) >> 2); */
    s0n_0 = SUB(s1n_0, SAR(ADD3(d1n_0, d1n_0, two), 2));
    s0n_1 = SUB(s1n_1, SAR(ADD3(d1n_1, d1n_1, two), 2));

    /* Main loop */
    for (i = 0, j = 1; i < (len - 3); i += 2, j++) {
        d1c_0 = d1n_0;
        s0c_0 = s0n_0;
        d1c_1 = d1n_1;
        s0c_1 = s0n_1;

        s1n_0 = LOADU(in_even + j * stride);
        s1n_1 = LOADU(in_even + j * stride + VREG_INT_COUNT);
        d1n_0 = LOADU(in_odd + j * stride);
        d1n_1 = LOADU(in_odd + j * stride + VREG_INT_COUNT);

        /* s0n = s1n - ((d1c + d1n + 2) >> 2); */
        s0n_0 = SUB(s1n_0, SAR(ADD3(d1c_0, d1n_0, two), 2));
        s0n_1 = SUB(s1n_1, SAR(ADD3(d1c_1, d1n_1, two), 2));

        STORE(tmp + PARALLEL_COLS_53_NEON * (i + 0), s0c_0);
        STORE(tmp + PARALLEL_COLS_53_NEON * (i + 0) + VREG_INT_COUNT, s0c_1);

        /* d1c + ((s0c + s0n) >> 1) */
        STORE(tmp + PARALLEL_COLS_53_NEON * (i + 1) + 0,
              ADD(d1c_0, SAR(ADD(s0c_0, s0n_0), 1)));
        STORE(tmp + PARALLEL_COLS_53_NEON * (i + 1) + VREG_INT_COUNT,
              ADD(d1c_1, SAR(ADD(s0c_1, s0n_1), 1)));
    }

    STORE(tmp + PARALLEL_COLS_53_NEON * (i + 0) + 0, s0n_0);
    STORE(tmp + PARALLEL_COLS_53_NEON * (i + 0) + VREG_INT_COUNT, s0n_1);

    /* Handle odd length */
    if (len & 1) {
        VREG tmp_len_minus_1;
        s1n_0 = LOADU(in_even + (OPJ_SIZE_T)((len - 1) / 2) * stride);
        tmp_len_minus_1 = SUB(s1n_0, SAR(ADD3(d1n_0, d1n_0, two), 2));
        STORE(tmp + PARALLEL_COLS_53_NEON * (len - 1), tmp_len_minus_1);
        STORE(tmp + PARALLEL_COLS_53_NEON * (len - 2),
              ADD(d1n_0, SAR(ADD(s0n_0, tmp_len_minus_1), 1)));

        s1n_1 = LOADU(in_even + (OPJ_SIZE_T)((len - 1) / 2) * stride + VREG_INT_COUNT);
        tmp_len_minus_1 = SUB(s1n_1, SAR(ADD3(d1n_1, d1n_1, two), 2));
        STORE(tmp + PARALLEL_COLS_53_NEON * (len - 1) + VREG_INT_COUNT,
              tmp_len_minus_1);
        STORE(tmp + PARALLEL_COLS_53_NEON * (len - 2) + VREG_INT_COUNT,
              ADD(d1n_1, SAR(ADD(s0n_1, tmp_len_minus_1), 1)));
    } else {
        STORE(tmp + PARALLEL_COLS_53_NEON * (len - 1) + 0,
              ADD(d1n_0, s0n_0));
        STORE(tmp + PARALLEL_COLS_53_NEON * (len - 1) + VREG_INT_COUNT,
              ADD(d1n_1, s0n_1));
    }

    opj_idwt53_v_final_memcpy_neon(tiledp_col, tmp, len, stride);
}

/**
 * Vertical inverse 5x3 wavelet transform for 8 columns in NEON
 * when top-most pixel is on odd coordinate
 */
static void opj_idwt53_v_cas1_mcols_NEON(
    OPJ_INT32* tmp,
    const OPJ_INT32 sn,
    const OPJ_INT32 len,
    OPJ_INT32* tiledp_col,
    const OPJ_SIZE_T stride)
{
    const OPJ_INT32* in_even = &tiledp_col[0];
    const OPJ_INT32* in_odd = &tiledp_col[(OPJ_SIZE_T)sn * stride];

    OPJ_INT32 i;
    OPJ_SIZE_T j;
    VREG s1_0, s2_0, dc_0, dn_0;
    VREG s1_1, s2_1, dc_1, dn_1;
    const VREG two = LOAD_CST(2);

    assert(len > 2);
    assert(PARALLEL_COLS_53_NEON == 8);
    assert(VREG_INT_COUNT == 4);

    s1_0 = LOADU(in_even + 0);
    s1_1 = LOADU(in_even + VREG_INT_COUNT);
    dc_0 = LOADU(in_odd + 0);
    dc_1 = LOADU(in_odd + VREG_INT_COUNT);
    s2_0 = LOADU(in_even + stride);
    s2_1 = LOADU(in_even + stride + VREG_INT_COUNT);

    /* tmp[0] = dc - ((s1 + s2 + 2) >> 2) */
    STORE(tmp + PARALLEL_COLS_53_NEON * 0,
          SUB(dc_0, SAR(ADD3(s1_0, s2_0, two), 2)));
    STORE(tmp + PARALLEL_COLS_53_NEON * 0 + VREG_INT_COUNT,
          SUB(dc_1, SAR(ADD3(s1_1, s2_1, two), 2)));

    for (i = 1, j = 1; i < (len - 2 - !(len & 1)); i += 2, j++) {
        s1_0 = s2_0;
        s1_1 = s2_1;
        s2_0 = LOADU(in_even + (j + 1) * stride);
        s2_1 = LOADU(in_even + (j + 1) * stride + VREG_INT_COUNT);

        dn_0 = LOADU(in_odd + j * stride);
        dn_1 = LOADU(in_odd + j * stride + VREG_INT_COUNT);

        /* tmp[i] = s1 + ((dc + dn) >> 1) */
        STORE(tmp + PARALLEL_COLS_53_NEON * i,
              ADD(s1_0, SAR(ADD(dc_0, dn_0), 1)));
        STORE(tmp + PARALLEL_COLS_53_NEON * i + VREG_INT_COUNT,
              ADD(s1_1, SAR(ADD(dc_1, dn_1), 1)));

        dc_0 = dn_0;
        dc_1 = dn_1;

        /* tmp[i+1] = dn - ((s1 + s2 + 2) >> 2) */
        STORE(tmp + PARALLEL_COLS_53_NEON * (i + 1),
              SUB(dn_0, SAR(ADD3(s1_0, s2_0, two), 2)));
        STORE(tmp + PARALLEL_COLS_53_NEON * (i + 1) + VREG_INT_COUNT,
              SUB(dn_1, SAR(ADD3(s1_1, s2_1, two), 2)));
    }

    STORE(tmp + PARALLEL_COLS_53_NEON * i,
          ADD(s2_0, SAR(ADD(dc_0, dc_0), 1)));
    STORE(tmp + PARALLEL_COLS_53_NEON * i + VREG_INT_COUNT,
          ADD(s2_1, SAR(ADD(dc_1, dc_1), 1)));

    if (!(len & 1)) {
        STORE(tmp + PARALLEL_COLS_53_NEON * (len - 1),
              ADD(dn_0, SAR(ADD(s2_0, s2_0), 1)));
        STORE(tmp + PARALLEL_COLS_53_NEON * (len - 1) + VREG_INT_COUNT,
              ADD(dn_1, SAR(ADD(s2_1, s2_1), 1)));
    }

    opj_idwt53_v_final_memcpy_neon(tiledp_col, tmp, len, stride);
}

/**
 * Final memcpy for vertical inverse DWT
 */
static void opj_idwt53_v_final_memcpy_neon(
    OPJ_INT32* tiledp_col,
    const OPJ_INT32* tmp,
    OPJ_INT32 len,
    OPJ_SIZE_T stride)
{
    OPJ_INT32 i;
    for (i = 0; i < len; ++i) {
        STOREU(&tiledp_col[(OPJ_SIZE_T)i * stride + 0],
               LOAD(&tmp[PARALLEL_COLS_53_NEON * i + 0]));
        STOREU(&tiledp_col[(OPJ_SIZE_T)i * stride + VREG_INT_COUNT],
               LOAD(&tmp[PARALLEL_COLS_53_NEON * i + VREG_INT_COUNT]));
    }
}

/* ========================================================================
 * Horizontal DWT 5/3 (Reversible) - NEON optimized
 * ======================================================================== */

/**
 * Horizontal inverse 5x3 wavelet transform - NEON optimized
 * This function processes the data in-place
 */
void opj_idwt53_h_cas0_neon(
    OPJ_INT32* tmp,
    const OPJ_INT32 sn,
    const OPJ_INT32 len,
    OPJ_INT32* tiledp)
{
    OPJ_INT32 i;
    const OPJ_INT32* in_even = tiledp;
    const OPJ_INT32* in_odd = tiledp + sn;

    /* Process in chunks of 4 with NEON */
    for (i = 0; i < (len & ~3); i += 4) {
        if (i < len - 3) {
            int32x4_t s1n, d1n, s0n;
            const int32x4_t two = vdupq_n_s32(2);

            /* Load 4 values */
            s1n = vld1q_s32(in_even + i / 2);
            d1n = vld1q_s32(in_odd + i / 2);

            /* Predict: s0n = s1n - ((d1n + 1) >> 1) */
            s0n = vsubq_s32(s1n, vshrq_n_s32(
                vaddq_s32(vaddq_s32(d1n, d1n), two), 2));

            /* Store low-pass */
            vst1q_s32(tmp + i, s0n);

            /* Update: d0n = d1n + ((s0[i] + s0[i+1]) >> 1) */
            if (i + 4 < len) {
                int32x4_t s0n_next = vld1q_s32(tmp + i + 4);
                int32x4_t d0n = vaddq_s32(d1n,
                    vshrq_n_s32(vaddq_s32(s0n, s0n_next), 1));
                vst1q_s32(tmp + i + 1, d0n);
            }
        }
    }

    /* Handle remainder with scalar code */
    for (; i < len; i++) {
        OPJ_INT32 s1n = in_even[i / 2];
        OPJ_INT32 d1n = in_odd[i / 2];
        tmp[i] = s1n - ((d1n + 1) >> 1);
    }
}

/**
 * Horizontal inverse 5x3 when starting on odd coordinate
 */
void opj_idwt53_h_cas1_neon(
    OPJ_INT32* tmp,
    const OPJ_INT32 sn,
    const OPJ_INT32 len,
    OPJ_INT32* tiledp)
{
    OPJ_INT32 i;
    const OPJ_INT32* in_even = tiledp;
    const OPJ_INT32* in_odd = tiledp + sn;

    /* First sample is special */
    tmp[0] = in_odd[0] + in_even[0];

    /* Process with NEON */
    for (i = 1; i < (len & ~3); i += 4) {
        int32x4_t s1, s2, dc, dn;
        const int32x4_t two = vdupq_n_s32(2);

        s1 = vld1q_s32(in_even + (i - 1) / 2);
        s2 = vld1q_s32(in_even + (i + 3) / 2);
        dc = vld1q_s32(in_odd + i / 2);

        /* d0 = dc - ((s1 + s2 + 2) >> 2) */
        int32x4_t d0 = vsubq_s32(dc,
            vshrq_n_s32(vaddq_s32(vaddq_s32(s1, s2), two), 2));

        vst1q_s32(tmp + i, d0);
    }

    /* Scalar remainder */
    for (; i < len; i++) {
        OPJ_INT32 s1 = in_even[(i - 1) / 2];
        OPJ_INT32 s2 = in_even[(i + 1) / 2];
        OPJ_INT32 dc = in_odd[i / 2];
        tmp[i] = dc - ((s1 + s2 + 2) >> 2);
    }
}

/* ========================================================================
 * Forward DWT 9/7 (Irreversible) - Float NEON
 * ======================================================================== */

/**
 * Forward 9/7 wavelet transform using float32 NEON
 * Uses lifting scheme with float operations
 */
void opj_dwt_encode_1_real_neon(
    OPJ_FLOAT32* a,
    OPJ_INT32 dn,
    OPJ_INT32 sn,
    OPJ_INT32 cas)
{
    OPJ_INT32 i;
    
    /* CDF 9/7 coefficients */
    const float32x4_t alpha = vdupq_n_f32(-1.586134342f);
    const float32x4_t beta  = vdupq_n_f32(-0.052980118f);
    const float32x4_t gamma = vdupq_n_f32(0.882911075f);
    const float32x4_t delta = vdupq_n_f32(0.443506852f);

    if (!cas) {
        /* Lifting steps with NEON */
        /* Step 1: Predict - Update odd samples */
        for (i = 0; i < (dn & ~3); i += 4) {
            float32x4_t odd = vld1q_f32(&a[2 * i + 1]);
            float32x4_t even_left = vld1q_f32(&a[2 * i]);
            float32x4_t even_right = vld1q_f32(&a[2 * i + 2]);
            
            /* d[i] += alpha * (s[i] + s[i+1]) */
            float32x4_t sum = vaddq_f32(even_left, even_right);
            odd = vmlaq_f32(odd, alpha, sum);  /* Fused multiply-add */
            vst1q_f32(&a[2 * i + 1], odd);
        }

        /* Step 2: Update - Update even samples */
        for (i = 0; i < (sn & ~3); i += 4) {
            float32x4_t even = vld1q_f32(&a[2 * i]);
            float32x4_t odd_left = vld1q_f32(&a[2 * i - 1]);
            float32x4_t odd_right = vld1q_f32(&a[2 * i + 1]);
            
            /* s[i] += beta * (d[i-1] + d[i]) */
            float32x4_t sum = vaddq_f32(odd_left, odd_right);
            even = vmlaq_f32(even, beta, sum);
            vst1q_f32(&a[2 * i], even);
        }

        /* Scalar code for remainder */
        for (; i < dn; i++) {
            a[2 * i + 1] += alpha.n128_f32[0] * (a[2 * i] + a[2 * i + 2]);
        }
    }
}

#endif /* __ARM_NEON */
