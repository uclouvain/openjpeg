/*
 * Performance Monitoring System - Header
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 */

#ifndef OPJ_PERFORMANCE_MONITOR_H
#define OPJ_PERFORMANCE_MONITOR_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Timer structure for profiling */
typedef struct {
    double start_time;
    const char* operation_name;
} opj_timer;

/* ========================================================================
 * Monitoring Control
 * ======================================================================== */

/**
 * Enable or disable performance monitoring
 * @param enable true to enable, false to disable
 */
void opj_perf_monitor_enable(bool enable);

/**
 * Check if monitoring is enabled
 * @return true if enabled, false otherwise
 */
bool opj_perf_monitor_is_enabled(void);

/**
 * Reset all performance counters
 */
void opj_perf_monitor_reset(void);

/* ========================================================================
 * Recording Functions
 * ======================================================================== */

/**
 * Record MCT operation
 * @param time Time elapsed in seconds
 * @param pixels Number of pixels processed
 * @param used_neon Whether NEON was used
 * @param used_metal Whether Metal GPU was used
 */
void opj_perf_record_mct(double time, size_t pixels, bool used_neon, bool used_metal);

/**
 * Record DWT operation
 * @param time Time elapsed in seconds
 * @param pixels Number of pixels processed
 * @param used_neon Whether NEON was used
 * @param used_metal Whether Metal GPU was used
 */
void opj_perf_record_dwt(double time, size_t pixels, bool used_neon, bool used_metal);

/**
 * Record T1 (EBCOT) operation
 * @param time Time elapsed in seconds
 */
void opj_perf_record_t1(double time);

/**
 * Record tile processing
 * @param time Time elapsed in seconds
 */
void opj_perf_record_tile(double time);

/* ========================================================================
 * Adaptive Tuning
 * ======================================================================== */

/**
 * Record speedup for adaptive threshold tuning
 * @param is_neon true for NEON, false for Metal
 * @param speedup Speedup factor achieved
 */
void opj_perf_record_speedup(bool is_neon, double speedup);

/**
 * Adjust thresholds based on recent performance
 */
void opj_perf_adjust_thresholds(void);

/**
 * Get current NEON threshold
 * @return Current threshold in pixels
 */
size_t opj_perf_get_neon_threshold(void);

/**
 * Get current Metal threshold
 * @return Current threshold in pixels
 */
size_t opj_perf_get_metal_threshold(void);

/* ========================================================================
 * Reporting
 * ======================================================================== */

/**
 * Print performance report to stdout
 */
void opj_perf_print_report(void);

/**
 * Export performance data to CSV file
 * @param filename Output CSV file path
 */
void opj_perf_export_csv(const char* filename);

/* ========================================================================
 * Profiling Helpers
 * ======================================================================== */

/**
 * Start a timer
 * @param name Operation name for reporting
 * @return Timer structure
 */
opj_timer opj_timer_start(const char* name);

/**
 * Stop a timer and return elapsed time
 * @param t Timer to stop
 * @return Elapsed time in seconds
 */
double opj_timer_stop(opj_timer* t);

/**
 * Stop and print timer
 * @param t Timer to stop and print
 */
void opj_timer_print(opj_timer* t);

/* Convenience macros */
#define OPJ_TIMER_START(name) opj_timer __timer_##name = opj_timer_start(#name)
#define OPJ_TIMER_STOP(name) opj_timer_stop(&__timer_##name)
#define OPJ_TIMER_PRINT(name) opj_timer_print(&__timer_##name)

#ifdef __cplusplus
}
#endif

#endif /* OPJ_PERFORMANCE_MONITOR_H */
