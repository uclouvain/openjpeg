/*
 * SIMD and GPU Dispatcher for OpenJPEG
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Automatic selection between Scalar, NEON, and Metal GPU implementations
 */

#include "opj_includes.h"
#include <stdbool.h>

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

#ifdef HAVE_METAL
#include "metal_manager.h"
#endif

/* Global state */
typedef struct {
    bool cpu_features_detected;
    bool has_neon;
    bool has_metal;
    opj_metal_context* metal_ctx;
    
    /* Performance thresholds */
    size_t min_neon_size;      /* Minimum size to use NEON */
    size_t min_metal_size;     /* Minimum size to use Metal GPU */
    
    /* Statistics */
    unsigned long scalar_calls;
    unsigned long neon_calls;
    unsigned long metal_calls;
} opj_simd_state;

static opj_simd_state g_simd_state = {
    .cpu_features_detected = false,
    .has_neon = false,
    .has_metal = false,
    .metal_ctx = NULL,
    .min_neon_size = 256,        /* Use NEON for >= 256 pixels */
    .min_metal_size = 512 * 512, /* Use Metal for >= 512×512 */
    .scalar_calls = 0,
    .neon_calls = 0,
    .metal_calls = 0
};

/* ========================================================================
 * CPU Feature Detection
 * ======================================================================== */

void opj_detect_cpu_features(void)
{
    if (g_simd_state.cpu_features_detected) {
        return;
    }
    
#ifdef __ARM_NEON
    /* On ARM with NEON compiled in, we assume it's available */
    g_simd_state.has_neon = true;
#else
    g_simd_state.has_neon = false;
#endif
    
#ifdef HAVE_METAL
    /* Try to initialize Metal */
    g_simd_state.metal_ctx = opj_metal_init();
    g_simd_state.has_metal = (g_simd_state.metal_ctx != NULL);
#else
    g_simd_state.has_metal = false;
#endif
    
    g_simd_state.cpu_features_detected = true;
}

/* ========================================================================
 * Query Functions
 * ======================================================================== */

bool opj_has_neon(void)
{
    if (!g_simd_state.cpu_features_detected) {
        opj_detect_cpu_features();
    }
    return g_simd_state.has_neon;
}

bool opj_has_metal(void)
{
    if (!g_simd_state.cpu_features_detected) {
        opj_detect_cpu_features();
    }
    return g_simd_state.has_metal;
}

opj_metal_context* opj_get_metal_context(void)
{
    if (!g_simd_state.cpu_features_detected) {
        opj_detect_cpu_features();
    }
    return g_simd_state.metal_ctx;
}

/* ========================================================================
 * Configuration
 * ======================================================================== */

void opj_set_neon_threshold(size_t min_size)
{
    g_simd_state.min_neon_size = min_size;
}

void opj_set_metal_threshold(size_t min_size)
{
    g_simd_state.min_metal_size = min_size;
}

void opj_enable_metal(bool enable)
{
#ifdef HAVE_METAL
    if (g_simd_state.metal_ctx) {
        opj_metal_set_enabled(g_simd_state.metal_ctx, enable);
    }
#endif
}

/* ========================================================================
 * Statistics
 * ======================================================================== */

void opj_get_simd_stats(
    unsigned long* scalar_calls,
    unsigned long* neon_calls,
    unsigned long* metal_calls)
{
    if (scalar_calls) *scalar_calls = g_simd_state.scalar_calls;
    if (neon_calls) *neon_calls = g_simd_state.neon_calls;
    if (metal_calls) *metal_calls = g_simd_state.metal_calls;
}

void opj_reset_simd_stats(void)
{
    g_simd_state.scalar_calls = 0;
    g_simd_state.neon_calls = 0;
    g_simd_state.metal_calls = 0;
}

/* ========================================================================
 * Dispatch Decision Logic
 * ======================================================================== */

typedef enum {
    OPJ_IMPL_SCALAR,
    OPJ_IMPL_NEON,
    OPJ_IMPL_METAL
} opj_implementation;

static opj_implementation opj_choose_implementation(size_t data_size)
{
    if (!g_simd_state.cpu_features_detected) {
        opj_detect_cpu_features();
    }
    
    /* Decision tree:
     * 1. If Metal is available and size is large enough -> Metal
     * 2. If NEON is available and size is large enough -> NEON
     * 3. Otherwise -> Scalar
     */
    
    if (g_simd_state.has_metal && data_size >= g_simd_state.min_metal_size) {
        return OPJ_IMPL_METAL;
    }
    
    if (g_simd_state.has_neon && data_size >= g_simd_state.min_neon_size) {
        return OPJ_IMPL_NEON;
    }
    
    return OPJ_IMPL_SCALAR;
}

/* ========================================================================
 * MCT Dispatchers
 * ======================================================================== */

/* Forward declarations of implementations */
extern void opj_mct_encode_scalar(OPJ_INT32* c0, OPJ_INT32* c1, OPJ_INT32* c2, OPJ_SIZE_T n);
extern void opj_mct_decode_scalar(OPJ_INT32* c0, OPJ_INT32* c1, OPJ_INT32* c2, OPJ_SIZE_T n);

#ifdef __ARM_NEON
extern void opj_mct_encode_neon(OPJ_INT32* c0, OPJ_INT32* c1, OPJ_INT32* c2, OPJ_SIZE_T n);
extern void opj_mct_decode_neon(OPJ_INT32* c0, OPJ_INT32* c1, OPJ_INT32* c2, OPJ_SIZE_T n);
#endif

void opj_mct_encode_dispatch(
    OPJ_INT32* OPJ_RESTRICT c0,
    OPJ_INT32* OPJ_RESTRICT c1,
    OPJ_INT32* OPJ_RESTRICT c2,
    OPJ_SIZE_T n)
{
    opj_implementation impl = opj_choose_implementation(n);
    
    switch (impl) {
#ifdef HAVE_METAL
        case OPJ_IMPL_METAL:
            if (opj_metal_mct_encode(g_simd_state.metal_ctx, c0, c1, c2, n)) {
                g_simd_state.metal_calls++;
                return;
            }
            /* Fall through to NEON/Scalar if Metal fails */
#endif
            
#ifdef __ARM_NEON
        case OPJ_IMPL_NEON:
            opj_mct_encode_neon(c0, c1, c2, n);
            g_simd_state.neon_calls++;
            return;
#endif
            
        case OPJ_IMPL_SCALAR:
        default:
            opj_mct_encode_scalar(c0, c1, c2, n);
            g_simd_state.scalar_calls++;
            return;
    }
}

void opj_mct_decode_dispatch(
    OPJ_INT32* OPJ_RESTRICT c0,
    OPJ_INT32* OPJ_RESTRICT c1,
    OPJ_INT32* OPJ_RESTRICT c2,
    OPJ_SIZE_T n)
{
    opj_implementation impl = opj_choose_implementation(n);
    
    switch (impl) {
#ifdef HAVE_METAL
        case OPJ_IMPL_METAL:
            if (opj_metal_mct_decode(g_simd_state.metal_ctx, c0, c1, c2, n)) {
                g_simd_state.metal_calls++;
                return;
            }
#endif
            
#ifdef __ARM_NEON
        case OPJ_IMPL_NEON:
            opj_mct_decode_neon(c0, c1, c2, n);
            g_simd_state.neon_calls++;
            return;
#endif
            
        case OPJ_IMPL_SCALAR:
        default:
            opj_mct_decode_scalar(c0, c1, c2, n);
            g_simd_state.scalar_calls++;
            return;
    }
}

/* ========================================================================
 * Cleanup
 * ======================================================================== */

void opj_simd_cleanup(void)
{
#ifdef HAVE_METAL
    if (g_simd_state.metal_ctx) {
        opj_metal_cleanup(g_simd_state.metal_ctx);
        g_simd_state.metal_ctx = NULL;
    }
#endif
    
    g_simd_state.has_metal = false;
    g_simd_state.cpu_features_detected = false;
}

/* ========================================================================
 * Information/Debug Functions
 * ======================================================================== */

void opj_print_simd_info(void)
{
    if (!g_simd_state.cpu_features_detected) {
        opj_detect_cpu_features();
    }
    
    printf("OpenJPEG SIMD Configuration:\n");
    printf("  CPU Features:\n");
    printf("    NEON:  %s\n", g_simd_state.has_neon ? "Available" : "Not available");
    printf("    Metal: %s\n", g_simd_state.has_metal ? "Available" : "Not available");
    
#ifdef HAVE_METAL
    if (g_simd_state.has_metal) {
        printf("    GPU:   %s\n", opj_metal_get_device_name(g_simd_state.metal_ctx));
    }
#endif
    
    printf("\n  Thresholds:\n");
    printf("    NEON:  >= %zu pixels\n", g_simd_state.min_neon_size);
    printf("    Metal: >= %zu pixels\n", g_simd_state.min_metal_size);
    
    printf("\n  Call Statistics:\n");
    printf("    Scalar: %lu calls\n", g_simd_state.scalar_calls);
    printf("    NEON:   %lu calls\n", g_simd_state.neon_calls);
    printf("    Metal:  %lu calls\n", g_simd_state.metal_calls);
    
    unsigned long total = g_simd_state.scalar_calls + 
                          g_simd_state.neon_calls + 
                          g_simd_state.metal_calls;
    if (total > 0) {
        printf("\n  Usage Distribution:\n");
        printf("    Scalar: %.1f%%\n", (g_simd_state.scalar_calls * 100.0) / total);
        printf("    NEON:   %.1f%%\n", (g_simd_state.neon_calls * 100.0) / total);
        printf("    Metal:  %.1f%%\n", (g_simd_state.metal_calls * 100.0) / total);
    }
}

const char* opj_get_implementation_name(void)
{
    if (!g_simd_state.cpu_features_detected) {
        opj_detect_cpu_features();
    }
    
    if (g_simd_state.has_metal) {
        return "Metal GPU + NEON + Scalar";
    } else if (g_simd_state.has_neon) {
        return "NEON + Scalar";
    } else {
        return "Scalar";
    }
}
