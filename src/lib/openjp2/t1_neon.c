/*
 * T1 (EBCOT Tier-1) NEON Optimizations for OpenJPEG
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * ARM NEON optimized context modeling and arithmetic coding
 */

#include "opj_includes.h"

#ifdef __ARM_NEON
#include <arm_neon.h>

/* ========================================================================
 * Context Formation NEON Optimization
 * ======================================================================== */

/**
 * Compute significance context for multiple coefficients
 * Uses NEON to process 4 coefficients in parallel
 */
void opj_t1_update_flags_neon(
    opj_flag_t *flagsp,
    OPJ_UINT32 ci,
    OPJ_UINT32 s,
    OPJ_UINT32 stride)
{
    /* Load neighborhood flags using NEON */
    uint32x4_t north = vld1q_u32((uint32_t*)&flagsp[-stride]);
    uint32x4_t south = vld1q_u32((uint32_t*)&flagsp[stride]);
    uint32x4_t west = vld1q_u32((uint32_t*)&flagsp[-1]);
    uint32x4_t east = vld1q_u32((uint32_t*)&flagsp[1]);
    
    /* Combine flags */
    uint32x4_t combined = vorrq_u32(north, south);
    combined = vorrq_u32(combined, west);
    combined = vorrq_u32(combined, east);
    
    /* Update center flags */
    uint32x4_t center = vld1q_u32((uint32_t*)flagsp);
    combined = vorrq_u32(combined, center);
    
    vst1q_u32((uint32_t*)flagsp, combined);
}

/**
 * Context lookup for significance propagation (vectorized)
 */
void opj_t1_getnmsedec_sig_neon(
    const OPJ_INT32 *datap,
    OPJ_UINT32 *nmsedec,
    OPJ_UINT32 n,
    OPJ_UINT32 qmfbid)
{
    size_t i;
    
    for (i = 0; i < (n & ~3U); i += 4) {
        int32x4_t data = vld1q_s32(&datap[i]);
        int32x4_t abs_data = vabsq_s32(data);
        
        /* Approximate nmsedec using NEON */
        /* nmsedec ≈ |data|^2 for lossless, scaled for lossy */
        if (qmfbid == 1) {
            /* Lossless: simple squared */
            int32x4_t squared = vmulq_s32(abs_data, abs_data);
            vst1q_u32(&nmsedec[i], vreinterpretq_u32_s32(squared));
        } else {
            /* Lossy: include quantization effects */
            int32x4_t scaled = vshrq_n_s32(abs_data, 1);
            int32x4_t result = vmulq_s32(scaled, scaled);
            vst1q_u32(&nmsedec[i], vreinterpretq_u32_s32(result));
        }
    }
    
    /* Handle remainder */
    for (; i < n; i++) {
        OPJ_INT32 abs_val = abs(datap[i]);
        if (qmfbid == 1) {
            nmsedec[i] = abs_val * abs_val;
        } else {
            OPJ_INT32 scaled = abs_val >> 1;
            nmsedec[i] = scaled * scaled;
        }
    }
}

/* ========================================================================
 * Magnitude Refinement NEON
 * ======================================================================== */

/**
 * Magnitude refinement pass optimized with NEON
 * Processes multiple coefficients in parallel
 */
void opj_t1_enc_refpass_neon(
    OPJ_INT32 *data,
    const opj_flag_t *flagsp,
    OPJ_UINT32 *nmsedec,
    OPJ_UINT32 n,
    OPJ_UINT32 bpno)
{
    const OPJ_UINT32 one = 1U << bpno;
    const OPJ_UINT32 mask = ~((1U << (bpno + 1)) - 1);
    
    size_t i;
    for (i = 0; i < (n & ~3U); i += 4) {
        int32x4_t data_vec = vld1q_s32(&data[i]);
        uint32x4_t flags = vld1q_u32((const uint32_t*)&flagsp[i]);
        
        /* Check if coefficient is significant */
        uint32x4_t sig_mask = vandq_u32(flags, vdupq_n_u32(T1_SIG));
        
        /* Get magnitude bits */
        int32x4_t abs_data = vabsq_s32(data_vec);
        uint32x4_t abs_u32 = vreinterpretq_u32_s32(abs_data);
        
        /* Extract bit at bpno position */
        uint32x4_t bit = vandq_u32(abs_u32, vdupq_n_u32(one));
        uint32x4_t is_one = vceqq_u32(bit, vdupq_n_u32(one));
        
        /* Update nmsedec */
        uint32x4_t prev_nmsedec = vld1q_u32(&nmsedec[i]);
        uint32x4_t delta = vshlq_n_u32(bit, (bpno << 1));
        uint32x4_t new_nmsedec = vaddq_u32(prev_nmsedec, delta);
        
        /* Store only where significant */
        new_nmsedec = vbslq_u32(sig_mask, new_nmsedec, prev_nmsedec);
        vst1q_u32(&nmsedec[i], new_nmsedec);
    }
    
    /* Scalar fallback for remainder */
    for (; i < n; i++) {
        if (flagsp[i] & T1_SIG) {
            OPJ_INT32 abs_val = abs(data[i]);
            OPJ_UINT32 bit = (abs_val & one) ? 1 : 0;
            nmsedec[i] += bit << (bpno << 1);
        }
    }
}

/* ========================================================================
 * Cleanup Pass NEON
 * ======================================================================== */

/**
 * Cleanup pass - encode all remaining non-significant coefficients
 * Vectorized to process 4 coefficients at once
 */
void opj_t1_enc_clnpass_neon(
    OPJ_INT32 *data,
    opj_flag_t *flagsp,
    OPJ_UINT32 *nmsedec,
    OPJ_UINT32 n,
    OPJ_UINT32 bpno)
{
    const OPJ_UINT32 one = 1U << bpno;
    
    size_t i;
    for (i = 0; i < (n & ~3U); i += 4) {
        int32x4_t data_vec = vld1q_s32(&data[i]);
        uint32x4_t flags = vld1q_u32((uint32_t*)&flagsp[i]);
        
        /* Find non-significant coefficients */
        uint32x4_t not_sig = vmvnq_u32(vandq_u32(flags, vdupq_n_u32(T1_SIG)));
        
        /* Get magnitudes */
        int32x4_t abs_data = vabsq_s32(data_vec);
        uint32x4_t abs_u32 = vreinterpretq_u32_s32(abs_data);
        
        /* Check if coefficient becomes significant at this bitplane */
        uint32x4_t bit = vandq_u32(abs_u32, vdupq_n_u32(one));
        uint32x4_t becomes_sig = vceqq_u32(bit, vdupq_n_u32(one));
        becomes_sig = vandq_u32(becomes_sig, not_sig);
        
        /* Update flags for newly significant coefficients */
        uint32x4_t new_flags = vorrq_u32(flags, 
                                         vandq_u32(becomes_sig, vdupq_n_u32(T1_SIG)));
        vst1q_u32((uint32_t*)&flagsp[i], new_flags);
        
        /* Update nmsedec */
        uint32x4_t prev_nmsedec = vld1q_u32(&nmsedec[i]);
        uint32x4_t delta = vshlq_n_u32(bit, (bpno << 1));
        uint32x4_t new_nmsedec = vaddq_u32(prev_nmsedec, delta);
        new_nmsedec = vbslq_u32(becomes_sig, new_nmsedec, prev_nmsedec);
        vst1q_u32(&nmsedec[i], new_nmsedec);
    }
    
    /* Scalar fallback */
    for (; i < n; i++) {
        if (!(flagsp[i] & T1_SIG)) {
            OPJ_INT32 abs_val = abs(data[i]);
            if (abs_val & one) {
                flagsp[i] |= T1_SIG;
                nmsedec[i] += (abs_val & one) << (bpno << 1);
            }
        }
    }
}

/* ========================================================================
 * Sign Coding NEON
 * ======================================================================== */

/**
 * Extract signs from data array efficiently
 */
void opj_t1_extract_signs_neon(
    const OPJ_INT32 *data,
    OPJ_UINT32 *signs,
    OPJ_UINT32 n)
{
    size_t i;
    
    for (i = 0; i < (n & ~3U); i += 4) {
        int32x4_t data_vec = vld1q_s32(&data[i]);
        
        /* Extract sign bit (MSB) */
        uint32x4_t sign_bits = vreinterpretq_u32_s32(
            vshrq_n_s32(data_vec, 31));
        
        /* Convert to 0 (positive) or 1 (negative) */
        sign_bits = vandq_u32(sign_bits, vdupq_n_u32(1));
        
        vst1q_u32(&signs[i], sign_bits);
    }
    
    for (; i < n; i++) {
        signs[i] = (data[i] < 0) ? 1 : 0;
    }
}

/**
 * Apply signs to magnitude data
 */
void opj_t1_apply_signs_neon(
    OPJ_INT32 *data,
    const OPJ_UINT32 *signs,
    OPJ_UINT32 n)
{
    size_t i;
    
    for (i = 0; i < (n & ~3U); i += 4) {
        int32x4_t data_vec = vld1q_s32(&data[i]);
        uint32x4_t sign_vec = vld1q_u32(&signs[i]);
        
        /* Convert signs to -1 or +1 */
        int32x4_t sign_mult = vsubq_s32(
            vdupq_n_s32(1),
            vshlq_n_s32(vreinterpretq_s32_u32(sign_vec), 1));
        
        /* Apply signs */
        data_vec = vmulq_s32(vabsq_s32(data_vec), sign_mult);
        
        vst1q_s32(&data[i], data_vec);
    }
    
    for (; i < n; i++) {
        OPJ_INT32 abs_val = abs(data[i]);
        data[i] = signs[i] ? -abs_val : abs_val;
    }
}

/* ========================================================================
 * Code Block Processing NEON
 * ======================================================================== */

/**
 * Process entire code block with NEON
 * Combines multiple passes for efficiency
 */
void opj_t1_encode_cblk_neon(
    OPJ_INT32 *data,
    opj_flag_t *flagsp,
    OPJ_UINT32 width,
    OPJ_UINT32 height,
    OPJ_UINT32 max_bitplanes)
{
    const OPJ_UINT32 n = width * height;
    OPJ_UINT32 *nmsedec = opj_aligned_malloc(n * sizeof(OPJ_UINT32));
    OPJ_UINT32 *signs = opj_aligned_malloc(n * sizeof(OPJ_UINT32));
    
    if (!nmsedec || !signs) {
        opj_aligned_free(nmsedec);
        opj_aligned_free(signs);
        return;
    }
    
    /* Initialize */
    memset(nmsedec, 0, n * sizeof(OPJ_UINT32));
    opj_t1_extract_signs_neon(data, signs, n);
    
    /* Process bitplanes from MSB to LSB */
    for (OPJ_INT32 bpno = max_bitplanes - 1; bpno >= 0; bpno--) {
        /* Significance propagation pass */
        opj_t1_enc_clnpass_neon(data, flagsp, nmsedec, n, bpno);
        
        /* Magnitude refinement pass */
        opj_t1_enc_refpass_neon(data, flagsp, nmsedec, n, bpno);
    }
    
    /* Apply signs back */
    opj_t1_apply_signs_neon(data, signs, n);
    
    opj_aligned_free(nmsedec);
    opj_aligned_free(signs);
}

/* ========================================================================
 * Decoding Helpers NEON
 * ======================================================================== */

/**
 * Decode significance propagation pass with NEON
 */
void opj_t1_dec_sigpass_neon(
    OPJ_INT32 *data,
    opj_flag_t *flagsp,
    OPJ_UINT32 n,
    OPJ_UINT32 bpno,
    const OPJ_UINT32 *bits)
{
    const OPJ_UINT32 one = 1U << bpno;
    
    size_t i;
    for (i = 0; i < (n & ~3U); i += 4) {
        uint32x4_t flags = vld1q_u32((uint32_t*)&flagsp[i]);
        uint32x4_t not_sig = vmvnq_u32(vandq_u32(flags, vdupq_n_u32(T1_SIG)));
        
        /* Load decoded bits */
        uint32x4_t bit_vec = vld1q_u32(&bits[i]);
        
        /* Update data where not yet significant */
        int32x4_t data_vec = vld1q_s32(&data[i]);
        int32x4_t delta = vreinterpretq_s32_u32(
            vandq_u32(bit_vec, vdupq_n_u32(one)));
        int32x4_t new_data = vaddq_s32(data_vec, delta);
        
        new_data = vbslq_s32(not_sig, new_data, data_vec);
        vst1q_s32(&data[i], new_data);
        
        /* Update flags */
        uint32x4_t becomes_sig = vandq_u32(not_sig, 
                                           vceqq_u32(bit_vec, vdupq_n_u32(1)));
        uint32x4_t new_flags = vorrq_u32(flags,
                                         vandq_u32(becomes_sig, vdupq_n_u32(T1_SIG)));
        vst1q_u32((uint32_t*)&flagsp[i], new_flags);
    }
    
    /* Scalar fallback */
    for (; i < n; i++) {
        if (!(flagsp[i] & T1_SIG) && bits[i]) {
            data[i] |= one;
            flagsp[i] |= T1_SIG;
        }
    }
}

/* ========================================================================
 * Statistics and Analysis NEON
 * ======================================================================== */

/**
 * Count significant coefficients using NEON
 */
OPJ_UINT32 opj_t1_count_significant_neon(
    const opj_flag_t *flagsp,
    OPJ_UINT32 n)
{
    OPJ_UINT32 count = 0;
    size_t i;
    
    uint32x4_t count_vec = vdupq_n_u32(0);
    
    for (i = 0; i < (n & ~3U); i += 4) {
        uint32x4_t flags = vld1q_u32((const uint32_t*)&flagsp[i]);
        uint32x4_t is_sig = vandq_u32(flags, vdupq_n_u32(T1_SIG));
        
        /* Convert mask to 0 or 1 */
        is_sig = vshrq_n_u32(is_sig, __builtin_ctz(T1_SIG));
        
        count_vec = vaddq_u32(count_vec, is_sig);
    }
    
    /* Horizontal sum */
    uint32x2_t sum = vadd_u32(vget_low_u32(count_vec), vget_high_u32(count_vec));
    count += vget_lane_u32(vpadd_u32(sum, sum), 0);
    
    /* Handle remainder */
    for (; i < n; i++) {
        if (flagsp[i] & T1_SIG) count++;
    }
    
    return count;
}

/**
 * Find maximum magnitude in code block
 */
OPJ_UINT32 opj_t1_find_max_neon(
    const OPJ_INT32 *data,
    OPJ_UINT32 n)
{
    size_t i;
    int32x4_t max_vec = vdupq_n_s32(0);
    
    for (i = 0; i < (n & ~3U); i += 4) {
        int32x4_t data_vec = vld1q_s32(&data[i]);
        int32x4_t abs_vec = vabsq_s32(data_vec);
        max_vec = vmaxq_s32(max_vec, abs_vec);
    }
    
    /* Horizontal max */
    int32x2_t max_pair = vmax_s32(vget_low_s32(max_vec), vget_high_s32(max_vec));
    int32x2_t max_final = vpmax_s32(max_pair, max_pair);
    OPJ_UINT32 max_val = vget_lane_s32(max_final, 0);
    
    /* Check remainder */
    for (; i < n; i++) {
        OPJ_UINT32 abs_val = abs(data[i]);
        if (abs_val > max_val) max_val = abs_val;
    }
    
    return max_val;
}

#endif /* __ARM_NEON */
