/*
 * Rate-Distortion Optimization with NEON
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Optimized RD calculations for encoding decisions
 */

#include "opj_includes.h"

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

#include <math.h>
#include <float.h>

/* ========================================================================
 * MSE/PSNR Calculation NEON
 * ======================================================================== */

#ifdef __ARM_NEON
/**
 * Calculate Mean Squared Error using NEON
 * Processes 4 pixels at a time
 */
double opj_calculate_mse_neon(
    const OPJ_INT32 *original,
    const OPJ_INT32 *reconstructed,
    size_t n)
{
    size_t i;
    int64x2_t sum_vec = vdupq_n_s64(0);
    
    /* Process 4 elements at a time */
    for (i = 0; i < (n & ~3U); i += 4) {
        int32x4_t orig = vld1q_s32(&original[i]);
        int32x4_t recon = vld1q_s32(&reconstructed[i]);
        
        /* Calculate difference */
        int32x4_t diff = vsubq_s32(orig, recon);
        
        /* Square the differences (using widening multiply) */
        int32x2_t diff_lo = vget_low_s32(diff);
        int32x2_t diff_hi = vget_high_s32(diff);
        
        int64x2_t sq_lo = vmull_s32(diff_lo, diff_lo);
        int64x2_t sq_hi = vmull_s32(diff_hi, diff_hi);
        
        /* Accumulate */
        sum_vec = vaddq_s64(sum_vec, sq_lo);
        sum_vec = vaddq_s64(sum_vec, sq_hi);
    }
    
    /* Horizontal sum */
    int64_t sum = vgetq_lane_s64(sum_vec, 0) + vgetq_lane_s64(sum_vec, 1);
    
    /* Handle remainder */
    for (; i < n; i++) {
        OPJ_INT64 diff = original[i] - reconstructed[i];
        sum += diff * diff;
    }
    
    return (double)sum / n;
}

/**
 * Calculate PSNR from MSE
 */
double opj_calculate_psnr(double mse, OPJ_UINT32 max_value)
{
    if (mse < 1e-10) return 100.0; /* Perfect match */
    
    double max_sq = (double)max_value * max_value;
    return 10.0 * log10(max_sq / mse);
}

#else
/* Scalar fallback */
double opj_calculate_mse_neon(
    const OPJ_INT32 *original,
    const OPJ_INT32 *reconstructed,
    size_t n)
{
    OPJ_INT64 sum = 0;
    for (size_t i = 0; i < n; i++) {
        OPJ_INT64 diff = original[i] - reconstructed[i];
        sum += diff * diff;
    }
    return (double)sum / n;
}

double opj_calculate_psnr(double mse, OPJ_UINT32 max_value)
{
    if (mse < 1e-10) return 100.0;
    double max_sq = (double)max_value * max_value;
    return 10.0 * log10(max_sq / mse);
}
#endif

/* ========================================================================
 * Rate Estimation
 * ======================================================================== */

/**
 * Estimate rate (bits) for encoding coefficients
 * Uses entropy estimation
 */
double opj_estimate_rate(
    const OPJ_INT32 *data,
    size_t n,
    OPJ_UINT32 num_bitplanes)
{
    /* Count significant coefficients per bitplane */
    OPJ_UINT32 *sig_counts = calloc(num_bitplanes, sizeof(OPJ_UINT32));
    if (!sig_counts) return 0.0;
    
#ifdef __ARM_NEON
    /* Vectorized significance counting */
    for (size_t i = 0; i < (n & ~3U); i += 4) {
        int32x4_t data_vec = vld1q_s32(&data[i]);
        int32x4_t abs_vec = vabsq_s32(data_vec);
        
        /* Check each bitplane */
        for (OPJ_UINT32 bp = 0; bp < num_bitplanes; bp++) {
            uint32x4_t mask = vdupq_n_u32(1U << bp);
            uint32x4_t masked = vandq_u32(vreinterpretq_u32_s32(abs_vec), mask);
            uint32x4_t is_set = vceqq_u32(masked, mask);
            
            /* Count set bits */
            sig_counts[bp] += vgetq_lane_u32(is_set, 0) ? 1 : 0;
            sig_counts[bp] += vgetq_lane_u32(is_set, 1) ? 1 : 0;
            sig_counts[bp] += vgetq_lane_u32(is_set, 2) ? 1 : 0;
            sig_counts[bp] += vgetq_lane_u32(is_set, 3) ? 1 : 0;
        }
    }
    
    /* Handle remainder */
    for (size_t i = (n & ~3U); i < n; i++) {
        OPJ_INT32 abs_val = abs(data[i]);
        for (OPJ_UINT32 bp = 0; bp < num_bitplanes; bp++) {
            if (abs_val & (1U << bp)) {
                sig_counts[bp]++;
            }
        }
    }
#else
    /* Scalar version */
    for (size_t i = 0; i < n; i++) {
        OPJ_INT32 abs_val = abs(data[i]);
        for (OPJ_UINT32 bp = 0; bp < num_bitplanes; bp++) {
            if (abs_val & (1U << bp)) {
                sig_counts[bp]++;
            }
        }
    }
#endif
    
    /* Estimate entropy */
    double total_bits = 0.0;
    for (OPJ_UINT32 bp = 0; bp < num_bitplanes; bp++) {
        if (sig_counts[bp] > 0) {
            double p = (double)sig_counts[bp] / n;
            double entropy = -p * log2(p) - (1.0 - p) * log2(1.0 - p);
            total_bits += n * entropy;
        }
    }
    
    free(sig_counts);
    return total_bits;
}

/* ========================================================================
 * RD Cost Calculation
 * ======================================================================== */

/**
 * Calculate Rate-Distortion cost
 * J = D + λ * R
 */
double opj_calculate_rd_cost(
    double distortion,
    double rate,
    double lambda)
{
    return distortion + lambda * rate;
}

/**
 * Optimal lambda calculation based on quantization
 */
double opj_calculate_lambda(OPJ_UINT32 qstep, double alpha)
{
    /* λ = α * Q²
     * Typical α ≈ 0.85 for JPEG 2000 */
    return alpha * qstep * qstep;
}

/* ========================================================================
 * Code Block RD Optimization
 * ======================================================================== */

typedef struct {
    OPJ_INT32 *data;
    size_t size;
    OPJ_UINT32 num_passes;
    double *pass_rates;      /* Rate for each pass */
    double *pass_distortions; /* Distortion for each pass */
    double *rd_slopes;       /* RD slope for each pass */
} opj_codeblock_rd_t;

/**
 * Calculate RD slopes for truncation point selection
 */
void opj_calculate_rd_slopes(opj_codeblock_rd_t *cblk, double lambda)
{
    /* Calculate slope between consecutive passes */
    for (OPJ_UINT32 i = 1; i < cblk->num_passes; i++) {
        double delta_r = cblk->pass_rates[i] - cblk->pass_rates[i-1];
        double delta_d = cblk->pass_distortions[i-1] - cblk->pass_distortions[i];
        
        if (delta_r > 0) {
            cblk->rd_slopes[i] = delta_d / delta_r;
        } else {
            cblk->rd_slopes[i] = DBL_MAX;
        }
    }
}

/**
 * Find optimal truncation point
 */
OPJ_UINT32 opj_find_optimal_truncation(
    opj_codeblock_rd_t *cblk,
    double lambda)
{
    opj_calculate_rd_slopes(cblk, lambda);
    
    /* Find last pass where slope >= lambda */
    OPJ_UINT32 best_pass = 0;
    for (OPJ_UINT32 i = 1; i < cblk->num_passes; i++) {
        if (cblk->rd_slopes[i] >= lambda) {
            best_pass = i;
        } else {
            break;
        }
    }
    
    return best_pass;
}

/* ========================================================================
 * PCRD-opt (Post-Compression Rate-Distortion Optimization)
 * ======================================================================== */

typedef struct {
    OPJ_UINT32 cblk_id;
    OPJ_UINT32 pass_id;
    double slope;
    double rate;
    double distortion;
} rd_pass_info_t;

int compare_slopes(const void *a, const void *b)
{
    const rd_pass_info_t *pa = (const rd_pass_info_t*)a;
    const rd_pass_info_t *pb = (const rd_pass_info_t*)b;
    
    if (pa->slope > pb->slope) return -1;
    if (pa->slope < pb->slope) return 1;
    return 0;
}

/**
 * PCRD-opt algorithm for multiple code blocks
 */
void opj_pcrd_opt(
    opj_codeblock_rd_t *codeblocks,
    OPJ_UINT32 num_codeblocks,
    double target_rate,
    OPJ_UINT32 *optimal_passes)
{
    /* Collect all passes from all code blocks */
    size_t total_passes = 0;
    for (OPJ_UINT32 i = 0; i < num_codeblocks; i++) {
        total_passes += codeblocks[i].num_passes;
    }
    
    rd_pass_info_t *all_passes = malloc(total_passes * sizeof(rd_pass_info_t));
    if (!all_passes) return;
    
    /* Populate pass information */
    size_t idx = 0;
    for (OPJ_UINT32 cb = 0; cb < num_codeblocks; cb++) {
        for (OPJ_UINT32 p = 0; p < codeblocks[cb].num_passes; p++) {
            all_passes[idx].cblk_id = cb;
            all_passes[idx].pass_id = p;
            all_passes[idx].slope = codeblocks[cb].rd_slopes[p];
            all_passes[idx].rate = codeblocks[cb].pass_rates[p];
            all_passes[idx].distortion = codeblocks[cb].pass_distortions[p];
            idx++;
        }
    }
    
    /* Sort by slope (descending) */
    qsort(all_passes, total_passes, sizeof(rd_pass_info_t), compare_slopes);
    
    /* Include passes until target rate reached */
    double cumulative_rate = 0.0;
    memset(optimal_passes, 0, num_codeblocks * sizeof(OPJ_UINT32));
    
    for (size_t i = 0; i < total_passes; i++) {
        rd_pass_info_t *pass = &all_passes[i];
        
        if (cumulative_rate + pass->rate <= target_rate) {
            cumulative_rate += pass->rate;
            if (pass->pass_id > optimal_passes[pass->cblk_id]) {
                optimal_passes[pass->cblk_id] = pass->pass_id;
            }
        }
    }
    
    free(all_passes);
}

/* ========================================================================
 * Quality Metrics
 * ======================================================================== */

/**
 * Calculate SSIM (Structural Similarity Index)
 * Simplified version for 8x8 blocks
 */
double opj_calculate_ssim_block(
    const OPJ_INT32 *orig,
    const OPJ_INT32 *recon,
    OPJ_UINT32 stride,
    OPJ_UINT32 block_size)
{
    const double C1 = 6.5025;   /* (K1*L)^2, K1=0.01, L=255 */
    const double C2 = 58.5225;  /* (K2*L)^2, K2=0.03 */
    
    /* Calculate means */
    double sum_orig = 0.0, sum_recon = 0.0;
    for (OPJ_UINT32 y = 0; y < block_size; y++) {
        for (OPJ_UINT32 x = 0; x < block_size; x++) {
            sum_orig += orig[y * stride + x];
            sum_recon += recon[y * stride + x];
        }
    }
    
    double n = block_size * block_size;
    double mu_orig = sum_orig / n;
    double mu_recon = sum_recon / n;
    
    /* Calculate variances and covariance */
    double var_orig = 0.0, var_recon = 0.0, covar = 0.0;
    for (OPJ_UINT32 y = 0; y < block_size; y++) {
        for (OPJ_UINT32 x = 0; x < block_size; x++) {
            double diff_orig = orig[y * stride + x] - mu_orig;
            double diff_recon = recon[y * stride + x] - mu_recon;
            var_orig += diff_orig * diff_orig;
            var_recon += diff_recon * diff_recon;
            covar += diff_orig * diff_recon;
        }
    }
    
    var_orig /= (n - 1);
    var_recon /= (n - 1);
    covar /= (n - 1);
    
    /* SSIM formula */
    double numerator = (2 * mu_orig * mu_recon + C1) * (2 * covar + C2);
    double denominator = (mu_orig * mu_orig + mu_recon * mu_recon + C1) *
                         (var_orig + var_recon + C2);
    
    return numerator / denominator;
}

/**
 * Average SSIM over entire image
 */
double opj_calculate_ssim_image(
    const OPJ_INT32 *orig,
    const OPJ_INT32 *recon,
    OPJ_UINT32 width,
    OPJ_UINT32 height)
{
    const OPJ_UINT32 block_size = 8;
    double total_ssim = 0.0;
    OPJ_UINT32 num_blocks = 0;
    
    for (OPJ_UINT32 y = 0; y + block_size <= height; y += block_size) {
        for (OPJ_UINT32 x = 0; x + block_size <= width; x += block_size) {
            const OPJ_INT32 *orig_block = orig + y * width + x;
            const OPJ_INT32 *recon_block = recon + y * width + x;
            
            double ssim = opj_calculate_ssim_block(orig_block, recon_block,
                                                   width, block_size);
            total_ssim += ssim;
            num_blocks++;
        }
    }
    
    return num_blocks > 0 ? total_ssim / num_blocks : 0.0;
}

/* ========================================================================
 * Bitrate Allocation
 * ======================================================================== */

/**
 * Allocate bitrate among components based on variance
 */
void opj_allocate_bitrate_by_variance(
    const OPJ_INT32 **components,
    size_t *component_sizes,
    OPJ_UINT32 num_components,
    double total_bitrate,
    double *allocated_rates)
{
    double *variances = malloc(num_components * sizeof(double));
    if (!variances) return;
    
    /* Calculate variance for each component */
    double total_variance = 0.0;
    for (OPJ_UINT32 c = 0; c < num_components; c++) {
        double mean = 0.0;
        size_t n = component_sizes[c];
        
        for (size_t i = 0; i < n; i++) {
            mean += components[c][i];
        }
        mean /= n;
        
        double var = 0.0;
        for (size_t i = 0; i < n; i++) {
            double diff = components[c][i] - mean;
            var += diff * diff;
        }
        var /= n;
        
        variances[c] = var;
        total_variance += var;
    }
    
    /* Allocate proportionally to variance */
    for (OPJ_UINT32 c = 0; c < num_components; c++) {
        if (total_variance > 0) {
            allocated_rates[c] = total_bitrate * (variances[c] / total_variance);
        } else {
            allocated_rates[c] = total_bitrate / num_components;
        }
    }
    
    free(variances);
}

/* ========================================================================
 * Statistics and Reporting
 * ======================================================================== */

void opj_print_rd_statistics(
    double mse,
    double psnr,
    double ssim,
    double rate_bpp,
    size_t compressed_size)
{
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║           Rate-Distortion Statistics                        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Distortion Metrics:\n");
    printf("    MSE:              %.4f\n", mse);
    printf("    PSNR:             %.2f dB\n", psnr);
    printf("    SSIM:             %.4f\n", ssim);
    printf("\n");
    printf("  Rate Metrics:\n");
    printf("    Compressed Size:  %zu bytes\n", compressed_size);
    printf("    Rate:             %.4f bpp\n", rate_bpp);
    printf("    Compression:      %.2f:1\n", 1.0 / rate_bpp * 8.0);
    printf("\n");
    printf("  Quality Assessment:\n");
    if (psnr >= 45.0) {
        printf("    Quality Level:    Excellent (Visually Lossless)\n");
    } else if (psnr >= 40.0) {
        printf("    Quality Level:    Very Good\n");
    } else if (psnr >= 35.0) {
        printf("    Quality Level:    Good\n");
    } else if (psnr >= 30.0) {
        printf("    Quality Level:    Fair\n");
    } else {
        printf("    Quality Level:    Poor\n");
    }
    
    if (ssim >= 0.95) {
        printf("    SSIM Level:       Excellent\n");
    } else if (ssim >= 0.90) {
        printf("    SSIM Level:       Very Good\n");
    } else if (ssim >= 0.80) {
        printf("    SSIM Level:       Good\n");
    } else {
        printf("    SSIM Level:       Fair\n");
    }
    
    printf("\n");
}
