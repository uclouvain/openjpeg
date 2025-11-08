/*
 * Advanced T1 (EBCOT) NEON Optimizations
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Advanced context modeling, MQ coder bit-packing, and parallel scanning
 */

#include "opj_includes.h"

#ifdef __ARM_NEON
#include <arm_neon.h>

/* ========================================================================
 * Advanced Context Formation with NEON
 * ======================================================================== */

/**
 * Vectorized context formation for 8x8 coefficient blocks
 * Processes entire block in parallel using NEON
 */
void opj_t1_context_formation_block_neon(
    const OPJ_INT32* coeffs,
    uint8_t* contexts,
    OPJ_UINT32 stride,
    OPJ_UINT32 width,
    OPJ_UINT32 height)
{
    /* Process 4 rows at a time */
    for (OPJ_UINT32 y = 1; y < height - 1; y++) {
        for (OPJ_UINT32 x = 0; x < width; x += 4) {
            /* Load 3x4 neighborhood */
            int32x4_t north = vld1q_s32(&coeffs[(y-1)*stride + x]);
            int32x4_t center = vld1q_s32(&coeffs[y*stride + x]);
            int32x4_t south = vld1q_s32(&coeffs[(y+1)*stride + x]);
            
            int32x4_t west = vld1q_s32(&coeffs[y*stride + x - 1]);
            int32x4_t east = vld1q_s32(&coeffs[y*stride + x + 1]);
            
            /* Compute significance (non-zero) */
            uint32x4_t sig_n = vtstq_s32(north, north);
            uint32x4_t sig_s = vtstq_s32(south, south);
            uint32x4_t sig_w = vtstq_s32(west, west);
            uint32x4_t sig_e = vtstq_s32(east, east);
            
            /* Count significant neighbors (H + V + D patterns) */
            uint32x4_t h_count = vaddq_u32(vandq_u32(sig_w, vdupq_n_u32(1)),
                                           vandq_u32(sig_e, vdupq_n_u32(1)));
            uint32x4_t v_count = vaddq_u32(vandq_u32(sig_n, vdupq_n_u32(1)),
                                           vandq_u32(sig_s, vdupq_n_u32(1)));
            
            /* Context = H + V (0-4 neighbors) */
            uint32x4_t context = vaddq_u32(h_count, v_count);
            
            /* Store contexts (narrow to uint8) */
            uint16x4_t ctx16 = vmovn_u32(context);
            uint8x8_t ctx8 = vmovn_u16(vcombine_u16(ctx16, vdup_n_u16(0)));
            
            vst1_lane_u32((uint32_t*)&contexts[y*stride + x], 
                         vreinterpret_u32_u8(ctx8), 0);
        }
    }
}

/**
 * Sign coding context computation (vectorized)
 */
void opj_t1_compute_sign_context_neon(
    const OPJ_INT32* coeffs,
    uint8_t* sign_contexts,
    OPJ_UINT32 stride,
    OPJ_UINT32 width,
    OPJ_UINT32 height)
{
    const int32x4_t zero = vdupq_n_s32(0);
    
    for (OPJ_UINT32 y = 1; y < height - 1; y++) {
        for (OPJ_UINT32 x = 0; x < width; x += 4) {
            /* Load neighbors */
            int32x4_t north = vld1q_s32(&coeffs[(y-1)*stride + x]);
            int32x4_t south = vld1q_s32(&coeffs[(y+1)*stride + x]);
            int32x4_t west = vld1q_s32(&coeffs[y*stride + x - 1]);
            int32x4_t east = vld1q_s32(&coeffs[y*stride + x + 1]);
            
            /* Get sign bits (1 if positive, 0 if negative) */
            uint32x4_t sign_n = vcgtq_s32(north, zero);
            uint32x4_t sign_s = vcgtq_s32(south, zero);
            uint32x4_t sign_w = vcgtq_s32(west, zero);
            uint32x4_t sign_e = vcgtq_s32(east, zero);
            
            /* Compute horizontal and vertical contributions */
            /* H = (sign_w << 1) | sign_e */
            uint32x4_t h_ctx = vorrq_u32(vshlq_n_u32(sign_w, 1), sign_e);
            
            /* V = (sign_n << 1) | sign_s */
            uint32x4_t v_ctx = vorrq_u32(vshlq_n_u32(sign_n, 1), sign_s);
            
            /* Combined context = (V << 2) | H */
            uint32x4_t context = vorrq_u32(vshlq_n_u32(v_ctx, 2), h_ctx);
            
            /* Store (narrow to uint8) */
            uint16x4_t ctx16 = vmovn_u32(context);
            uint8x8_t ctx8 = vmovn_u16(vcombine_u16(ctx16, vdup_n_u16(0)));
            
            vst1_lane_u32((uint32_t*)&sign_contexts[y*stride + x],
                         vreinterpret_u32_u8(ctx8), 0);
        }
    }
}

/* ========================================================================
 * MQ Coder Bit-Packing Optimizations
 * ======================================================================== */

/**
 * Pack multiple MQ coder bits efficiently using NEON
 * Processes 32 bits at a time
 */
void opj_mqc_pack_bits_neon(
    const uint8_t* bits,
    uint8_t* packed,
    OPJ_UINT32 num_bits)
{
    OPJ_UINT32 num_bytes = (num_bits + 7) / 8;
    
    /* Process 32 bits (4 bytes) at a time */
    for (OPJ_UINT32 i = 0; i < num_bytes; i += 4) {
        if (i + 4 > num_bytes) break;
        
        /* Load 32 bits */
        uint8x16_t bits_vec = vld1q_u8(&bits[i * 8]);
        
        /* Create bit masks */
        uint8x16_t mask = vcreateq_u8_u64(0x0102040810204080ULL, 
                                          0x0102040810204080ULL);
        
        /* AND with mask and horizontal OR */
        uint8x16_t masked = vandq_u8(bits_vec, mask);
        
        /* Pairwise addition to combine bits */
        uint16x8_t pairs = vpaddlq_u8(masked);
        uint32x4_t quads = vpaddlq_u16(pairs);
        uint64x2_t bytes = vpaddlq_u32(quads);
        
        /* Store result */
        uint32_t result = vgetq_lane_u32(vreinterpretq_u32_u64(bytes), 0);
        *((uint32_t*)&packed[i]) = result;
    }
    
    /* Handle remainder */
    OPJ_UINT32 remainder = num_bytes % 4;
    if (remainder > 0) {
        OPJ_UINT32 base = (num_bytes / 4) * 4;
        for (OPJ_UINT32 i = 0; i < remainder; i++) {
            uint8_t byte = 0;
            for (int j = 0; j < 8; j++) {
                byte |= (bits[(base + i) * 8 + j] & 1) << (7 - j);
            }
            packed[base + i] = byte;
        }
    }
}

/**
 * Unpack MQ coder bits (for decoding)
 */
void opj_mqc_unpack_bits_neon(
    const uint8_t* packed,
    uint8_t* bits,
    OPJ_UINT32 num_bits)
{
    OPJ_UINT32 num_bytes = (num_bits + 7) / 8;
    
    /* Process 4 bytes at a time */
    for (OPJ_UINT32 i = 0; i < num_bytes; i += 4) {
        if (i + 4 > num_bytes) break;
        
        /* Load 4 packed bytes */
        uint32_t packed_word = *((uint32_t*)&packed[i]);
        
        /* Replicate each byte to 8 positions */
        uint8x16_t bytes = vdupq_n_u8(0);
        bytes = vsetq_lane_u8((packed_word >> 24) & 0xFF, bytes, 0);
        bytes = vsetq_lane_u8((packed_word >> 16) & 0xFF, bytes, 4);
        bytes = vsetq_lane_u8((packed_word >> 8) & 0xFF, bytes, 8);
        bytes = vsetq_lane_u8(packed_word & 0xFF, bytes, 12);
        
        /* Create shift mask */
        uint8x16_t shifts = {7,6,5,4,3,2,1,0, 7,6,5,4,3,2,1,0};
        
        /* Shift and mask to extract bits */
        uint8x16_t shifted = vshlq_u8(bytes, vnegq_s8(vreinterpretq_s8_u8(shifts)));
        uint8x16_t masked = vandq_u8(shifted, vdupq_n_u8(1));
        
        /* Store */
        vst1q_u8(&bits[i * 8], masked);
    }
    
    /* Handle remainder */
    OPJ_UINT32 remainder = num_bytes % 4;
    if (remainder > 0) {
        OPJ_UINT32 base = (num_bytes / 4) * 4;
        for (OPJ_UINT32 i = 0; i < remainder; i++) {
            uint8_t byte = packed[base + i];
            for (int j = 0; j < 8; j++) {
                bits[(base + i) * 8 + j] = (byte >> (7 - j)) & 1;
            }
        }
    }
}

/* ========================================================================
 * Parallel Coefficient Scanning (Zig-Zag)
 * ======================================================================== */

/**
 * NEON-optimized zig-zag scan for 8x8 blocks
 * Reorders coefficients for entropy coding
 */
void opj_t1_zigzag_scan_8x8_neon(
    const OPJ_INT32* block,
    OPJ_INT32* scanned)
{
    /* Zig-zag pattern for 8x8 block */
    static const uint8_t zigzag[64] = {
        0,  1,  8, 16,  9,  2,  3, 10,
       17, 24, 32, 25, 18, 11,  4,  5,
       12, 19, 26, 33, 40, 48, 41, 34,
       27, 20, 13,  6,  7, 14, 21, 28,
       35, 42, 49, 56, 57, 50, 43, 36,
       29, 22, 15, 23, 30, 37, 44, 51,
       58, 59, 52, 45, 38, 31, 39, 46,
       53, 60, 61, 54, 47, 55, 62, 63
    };
    
    /* Load zigzag indices as NEON vector */
    uint8x16_t idx0 = vld1q_u8(&zigzag[0]);
    uint8x16_t idx1 = vld1q_u8(&zigzag[16]);
    uint8x16_t idx2 = vld1q_u8(&zigzag[32]);
    uint8x16_t idx3 = vld1q_u8(&zigzag[48]);
    
    /* Use vtbl for lookup (ARM NEON table lookup) */
    /* Process in chunks of 4 */
    for (int i = 0; i < 64; i += 4) {
        int32x4_t vals;
        vals = vsetq_lane_s32(block[zigzag[i]], vals, 0);
        vals = vsetq_lane_s32(block[zigzag[i+1]], vals, 1);
        vals = vsetq_lane_s32(block[zigzag[i+2]], vals, 2);
        vals = vsetq_lane_s32(block[zigzag[i+3]], vals, 3);
        vst1q_s32(&scanned[i], vals);
    }
}

/**
 * Reverse zig-zag scan (decode)
 */
void opj_t1_inverse_zigzag_scan_8x8_neon(
    const OPJ_INT32* scanned,
    OPJ_INT32* block)
{
    static const uint8_t zigzag[64] = {
        0,  1,  8, 16,  9,  2,  3, 10,
       17, 24, 32, 25, 18, 11,  4,  5,
       12, 19, 26, 33, 40, 48, 41, 34,
       27, 20, 13,  6,  7, 14, 21, 28,
       35, 42, 49, 56, 57, 50, 43, 36,
       29, 22, 15, 23, 30, 37, 44, 51,
       58, 59, 52, 45, 38, 31, 39, 46,
       53, 60, 61, 54, 47, 55, 62, 63
    };
    
    /* Scatter scanned values back to block positions */
    for (int i = 0; i < 64; i += 4) {
        int32x4_t vals = vld1q_s32(&scanned[i]);
        block[zigzag[i]] = vgetq_lane_s32(vals, 0);
        block[zigzag[i+1]] = vgetq_lane_s32(vals, 1);
        block[zigzag[i+2]] = vgetq_lane_s32(vals, 2);
        block[zigzag[i+3]] = vgetq_lane_s32(vals, 3);
    }
}

/* ========================================================================
 * Magnitude Refinement Pass (NEON)
 * ======================================================================== */

/**
 * Vectorized magnitude refinement for multiple coefficients
 */
void opj_t1_magnitude_refinement_neon(
    OPJ_INT32* coeffs,
    const uint8_t* bits,
    OPJ_UINT32 num_coeffs,
    OPJ_UINT32 bitpos)
{
    uint32_t shift_val = bitpos;
    int32x4_t shift = vdupq_n_s32(shift_val);
    
    /* Process 4 coefficients at a time */
    for (OPJ_UINT32 i = 0; i < num_coeffs; i += 4) {
        /* Load coefficients */
        int32x4_t coeff = vld1q_s32(&coeffs[i]);
        
        /* Load refinement bits (0 or 1) */
        uint32x4_t bits_vec;
        bits_vec = vsetq_lane_u32(bits[i], bits_vec, 0);
        bits_vec = vsetq_lane_u32(bits[i+1], bits_vec, 1);
        bits_vec = vsetq_lane_u32(bits[i+2], bits_vec, 2);
        bits_vec = vsetq_lane_u32(bits[i+3], bits_vec, 3);
        
        /* Shift bits to correct position */
        int32x4_t bit_val = vshlq_s32(vreinterpretq_s32_u32(bits_vec), shift);
        
        /* Add to coefficient (preserving sign) */
        int32x4_t refined = vaddq_s32(coeff, bit_val);
        
        /* Store */
        vst1q_s32(&coeffs[i], refined);
    }
    
    /* Handle remainder */
    OPJ_UINT32 remainder = num_coeffs % 4;
    OPJ_UINT32 base = (num_coeffs / 4) * 4;
    for (OPJ_UINT32 i = 0; i < remainder; i++) {
        coeffs[base + i] += bits[base + i] << shift_val;
    }
}

/* ========================================================================
 * Cleanup Pass Optimization
 * ======================================================================== */

/**
 * NEON-optimized cleanup coding pass
 * Processes all insignificant coefficients in parallel
 */
void opj_t1_cleanup_pass_neon(
    OPJ_INT32* coeffs,
    uint8_t* flags,
    const uint8_t* coded_bits,
    OPJ_UINT32 width,
    OPJ_UINT32 height,
    OPJ_UINT32 stride)
{
    const int32x4_t zero = vdupq_n_s32(0);
    
    for (OPJ_UINT32 y = 0; y < height; y++) {
        for (OPJ_UINT32 x = 0; x < width; x += 4) {
            /* Load coefficients */
            int32x4_t coeff = vld1q_s32(&coeffs[y*stride + x]);
            
            /* Check if insignificant (zero) */
            uint32x4_t is_zero = vceqq_s32(coeff, zero);
            
            /* Load coded bits */
            uint32x4_t bits;
            bits = vsetq_lane_u32(coded_bits[y*stride + x], bits, 0);
            bits = vsetq_lane_u32(coded_bits[y*stride + x + 1], bits, 1);
            bits = vsetq_lane_u32(coded_bits[y*stride + x + 2], bits, 2);
            bits = vsetq_lane_u32(coded_bits[y*stride + x + 3], bits, 3);
            
            /* Update only insignificant coefficients */
            int32x4_t update = vandq_s32(vreinterpretq_s32_u32(bits),
                                         vreinterpretq_s32_u32(is_zero));
            coeff = vorrq_s32(coeff, update);
            
            /* Store */
            vst1q_s32(&coeffs[y*stride + x], coeff);
            
            /* Update flags (mark as significant if bit was 1) */
            uint32x4_t new_flags = bits;
            uint8x8_t flags8 = vmovn_u16(vcombine_u16(vmovn_u32(new_flags), 
                                                      vdup_n_u16(0)));
            vst1_lane_u32((uint32_t*)&flags[y*stride + x],
                         vreinterpret_u32_u8(flags8), 0);
        }
    }
}

/* ========================================================================
 * Performance Monitoring
 * ======================================================================== */

typedef struct {
    uint64_t context_formation_cycles;
    uint64_t mqc_packing_cycles;
    uint64_t scanning_cycles;
    uint64_t cleanup_cycles;
    uint32_t blocks_processed;
} opj_t1_advanced_stats_t;

static opj_t1_advanced_stats_t g_t1_advanced_stats = {0};

void opj_t1_advanced_get_stats(opj_t1_advanced_stats_t* stats)
{
    *stats = g_t1_advanced_stats;
}

void opj_t1_advanced_reset_stats(void)
{
    memset(&g_t1_advanced_stats, 0, sizeof(g_t1_advanced_stats));
}

#endif /* __ARM_NEON */
