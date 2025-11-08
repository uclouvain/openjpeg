/*
 * Metal GPU Manager for OpenJPEG - Header
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 */

#ifndef METAL_MANAGER_H
#define METAL_MANAGER_H

#include "opj_includes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque Metal context structure */
typedef struct opj_metal_context opj_metal_context;

/* ========================================================================
 * Initialization and Cleanup
 * ======================================================================== */

/**
 * Initialize Metal GPU context
 * @return Metal context or NULL if Metal is not available
 */
opj_metal_context* opj_metal_init(void);

/**
 * Cleanup Metal context and free resources
 * @param ctx Metal context to cleanup
 */
void opj_metal_cleanup(opj_metal_context* ctx);

/* ========================================================================
 * MCT Operations (Multi-Component Transform)
 * ======================================================================== */

/**
 * Perform RGB to YUV MCT encoding on GPU
 * @param ctx Metal context
 * @param c0 Component 0 (R -> Y)
 * @param c1 Component 1 (G -> U)
 * @param c2 Component 2 (B -> V)
 * @param n Number of samples
 * @return true if GPU was used, false if CPU fallback should be used
 */
bool opj_metal_mct_encode(
    opj_metal_context* ctx,
    OPJ_INT32* c0,
    OPJ_INT32* c1,
    OPJ_INT32* c2,
    OPJ_SIZE_T n);

/**
 * Perform YUV to RGB MCT decoding on GPU
 * @param ctx Metal context
 * @param c0 Component 0 (Y -> R)
 * @param c1 Component 1 (U -> G)
 * @param c2 Component 2 (V -> B)
 * @param n Number of samples
 * @return true if GPU was used, false if CPU fallback should be used
 */
bool opj_metal_mct_decode(
    opj_metal_context* ctx,
    OPJ_INT32* c0,
    OPJ_INT32* c1,
    OPJ_INT32* c2,
    OPJ_SIZE_T n);

/* ========================================================================
 * DWT Operations (Discrete Wavelet Transform)
 * ======================================================================== */

/**
 * Perform forward DWT on GPU
 * @param ctx Metal context
 * @param data Input/output data
 * @param width Width of data
 * @param height Height of data
 * @return true if GPU was used, false otherwise
 */
bool opj_metal_dwt_forward(
    opj_metal_context* ctx,
    OPJ_INT32* data,
    OPJ_INT32 width,
    OPJ_INT32 height);

/**
 * Perform inverse DWT on GPU
 * @param ctx Metal context
 * @param data Input/output data
 * @param width Width of data
 * @param height Height of data
 * @return true if GPU was used, false otherwise
 */
bool opj_metal_dwt_inverse(
    opj_metal_context* ctx,
    OPJ_INT32* data,
    OPJ_INT32 width,
    OPJ_INT32 height);

/* ========================================================================
 * Configuration
 * ======================================================================== */

/**
 * Enable or disable GPU acceleration
 * @param ctx Metal context
 * @param enabled true to enable, false to disable
 */
void opj_metal_set_enabled(opj_metal_context* ctx, bool enabled);

/**
 * Check if Metal GPU is available and enabled
 * @param ctx Metal context
 * @return true if GPU is available, false otherwise
 */
bool opj_metal_is_available(opj_metal_context* ctx);

/**
 * Set minimum image size for GPU usage
 * Images smaller than this will use CPU
 * @param ctx Metal context
 * @param min_pixels Minimum number of pixels (e.g., 512*512)
 */
void opj_metal_set_min_size(opj_metal_context* ctx, size_t min_pixels);

/**
 * Get the name of the GPU device
 * @param ctx Metal context
 * @return Device name string or "No GPU"
 */
const char* opj_metal_get_device_name(opj_metal_context* ctx);

#ifdef __cplusplus
}
#endif

#endif /* METAL_MANAGER_H */
