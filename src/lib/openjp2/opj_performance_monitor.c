/*
 * Performance Monitoring System for OpenJPEG
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Real-time performance tracking and adaptive optimization
 */

#include "opj_includes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

/* Performance counters */
typedef struct {
    /* Timing */
    double total_time;
    double mct_time;
    double dwt_time;
    double t1_time;
    double t2_time;
    
    /* Operation counts */
    unsigned long mct_operations;
    unsigned long dwt_operations;
    unsigned long t1_operations;
    
    /* SIMD usage */
    unsigned long scalar_ops;
    unsigned long neon_ops;
    unsigned long metal_ops;
    
    /* Image statistics */
    unsigned long total_pixels;
    unsigned long total_tiles;
    
    /* Performance metrics */
    double pixels_per_second;
    double avg_tile_time;
    
    /* Adaptive thresholds */
    size_t current_neon_threshold;
    size_t current_metal_threshold;
    
    /* History for adaptive tuning */
    double recent_neon_speedup[10];
    double recent_metal_speedup[10];
    int history_index;
    
    bool monitoring_enabled;
} opj_perf_monitor;

static opj_perf_monitor g_monitor = {
    .monitoring_enabled = false,
    .current_neon_threshold = 256,
    .current_metal_threshold = 512 * 512,
    .history_index = 0
};

/* ========================================================================
 * Timing Functions
 * ======================================================================== */

static inline double opj_get_wall_time(void)
{
#ifdef __APPLE__
    static mach_timebase_info_data_t timebase;
    static bool timebase_initialized = false;
    
    if (!timebase_initialized) {
        mach_timebase_info(&timebase);
        timebase_initialized = true;
    }
    
    uint64_t time = mach_absolute_time();
    return (double)time * timebase.numer / timebase.denom / 1e9;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
#endif
}

/* ========================================================================
 * Monitoring Control
 * ======================================================================== */

void opj_perf_monitor_enable(bool enable)
{
    g_monitor.monitoring_enabled = enable;
}

bool opj_perf_monitor_is_enabled(void)
{
    return g_monitor.monitoring_enabled;
}

void opj_perf_monitor_reset(void)
{
    memset(&g_monitor, 0, sizeof(opj_perf_monitor));
    g_monitor.monitoring_enabled = true;
    g_monitor.current_neon_threshold = 256;
    g_monitor.current_metal_threshold = 512 * 512;
}

/* ========================================================================
 * Recording Functions
 * ======================================================================== */

void opj_perf_record_mct(double time, size_t pixels, bool used_neon, bool used_metal)
{
    if (!g_monitor.monitoring_enabled) return;
    
    g_monitor.mct_time += time;
    g_monitor.mct_operations++;
    g_monitor.total_pixels += pixels;
    
    if (used_metal) {
        g_monitor.metal_ops++;
    } else if (used_neon) {
        g_monitor.neon_ops++;
    } else {
        g_monitor.scalar_ops++;
    }
}

void opj_perf_record_dwt(double time, size_t pixels, bool used_neon, bool used_metal)
{
    if (!g_monitor.monitoring_enabled) return;
    
    g_monitor.dwt_time += time;
    g_monitor.dwt_operations++;
    
    if (used_metal) {
        g_monitor.metal_ops++;
    } else if (used_neon) {
        g_monitor.neon_ops++;
    } else {
        g_monitor.scalar_ops++;
    }
}

void opj_perf_record_t1(double time)
{
    if (!g_monitor.monitoring_enabled) return;
    
    g_monitor.t1_time += time;
    g_monitor.t1_operations++;
}

void opj_perf_record_tile(double time)
{
    if (!g_monitor.monitoring_enabled) return;
    
    g_monitor.total_tiles++;
    g_monitor.avg_tile_time = 
        (g_monitor.avg_tile_time * (g_monitor.total_tiles - 1) + time) / 
        g_monitor.total_tiles;
}

/* ========================================================================
 * Adaptive Threshold Tuning
 * ======================================================================== */

void opj_perf_record_speedup(bool is_neon, double speedup)
{
    if (!g_monitor.monitoring_enabled) return;
    
    int idx = g_monitor.history_index % 10;
    
    if (is_neon) {
        g_monitor.recent_neon_speedup[idx] = speedup;
    } else {
        g_monitor.recent_metal_speedup[idx] = speedup;
    }
    
    g_monitor.history_index++;
    
    /* Adaptive threshold adjustment every 10 operations */
    if (g_monitor.history_index % 10 == 0) {
        opj_perf_adjust_thresholds();
    }
}

void opj_perf_adjust_thresholds(void)
{
    /* Calculate average speedup from recent history */
    double avg_neon_speedup = 0.0;
    double avg_metal_speedup = 0.0;
    
    for (int i = 0; i < 10; i++) {
        avg_neon_speedup += g_monitor.recent_neon_speedup[i];
        avg_metal_speedup += g_monitor.recent_metal_speedup[i];
    }
    avg_neon_speedup /= 10.0;
    avg_metal_speedup /= 10.0;
    
    /* Adjust NEON threshold */
    if (avg_neon_speedup > 1.5) {
        /* Good speedup - lower threshold to use more often */
        g_monitor.current_neon_threshold = 
            (size_t)(g_monitor.current_neon_threshold * 0.8);
        if (g_monitor.current_neon_threshold < 64) {
            g_monitor.current_neon_threshold = 64;
        }
    } else if (avg_neon_speedup < 1.1) {
        /* Poor speedup - raise threshold */
        g_monitor.current_neon_threshold = 
            (size_t)(g_monitor.current_neon_threshold * 1.2);
        if (g_monitor.current_neon_threshold > 4096) {
            g_monitor.current_neon_threshold = 4096;
        }
    }
    
    /* Adjust Metal threshold */
    if (avg_metal_speedup > 10.0) {
        /* Excellent speedup - lower threshold */
        g_monitor.current_metal_threshold = 
            (size_t)(g_monitor.current_metal_threshold * 0.7);
        if (g_monitor.current_metal_threshold < 256 * 256) {
            g_monitor.current_metal_threshold = 256 * 256;
        }
    } else if (avg_metal_speedup < 3.0) {
        /* Poor speedup - raise threshold */
        g_monitor.current_metal_threshold = 
            (size_t)(g_monitor.current_metal_threshold * 1.3);
        if (g_monitor.current_metal_threshold > 2048 * 2048) {
            g_monitor.current_metal_threshold = 2048 * 2048;
        }
    }
}

size_t opj_perf_get_neon_threshold(void)
{
    return g_monitor.current_neon_threshold;
}

size_t opj_perf_get_metal_threshold(void)
{
    return g_monitor.current_metal_threshold;
}

/* ========================================================================
 * Reporting
 * ======================================================================== */

void opj_perf_print_report(void)
{
    if (!g_monitor.monitoring_enabled) {
        printf("Performance monitoring is disabled\n");
        return;
    }
    
    g_monitor.total_time = g_monitor.mct_time + g_monitor.dwt_time + 
                           g_monitor.t1_time + g_monitor.t2_time;
    
    if (g_monitor.total_time > 0) {
        g_monitor.pixels_per_second = g_monitor.total_pixels / g_monitor.total_time;
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║          OpenJPEG Performance Monitor Report                ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    printf("⏱️  TIMING BREAKDOWN:\n");
    printf("────────────────────────────────────────────────────────────\n");
    printf("  Total Time:      %8.3f ms (100.0%%)\n", g_monitor.total_time * 1000);
    if (g_monitor.total_time > 0) {
        printf("  MCT:             %8.3f ms (%5.1f%%)\n", 
               g_monitor.mct_time * 1000,
               (g_monitor.mct_time / g_monitor.total_time) * 100);
        printf("  DWT:             %8.3f ms (%5.1f%%)\n", 
               g_monitor.dwt_time * 1000,
               (g_monitor.dwt_time / g_monitor.total_time) * 100);
        printf("  T1 (EBCOT):      %8.3f ms (%5.1f%%)\n", 
               g_monitor.t1_time * 1000,
               (g_monitor.t1_time / g_monitor.total_time) * 100);
        printf("  T2 (MQ):         %8.3f ms (%5.1f%%)\n", 
               g_monitor.t2_time * 1000,
               (g_monitor.t2_time / g_monitor.total_time) * 100);
    }
    
    printf("\n📊 OPERATION COUNTS:\n");
    printf("────────────────────────────────────────────────────────────\n");
    printf("  MCT Operations:  %lu\n", g_monitor.mct_operations);
    printf("  DWT Operations:  %lu\n", g_monitor.dwt_operations);
    printf("  T1 Operations:   %lu\n", g_monitor.t1_operations);
    printf("  Total Tiles:     %lu\n", g_monitor.total_tiles);
    printf("  Total Pixels:    %lu (%.1f MP)\n", 
           g_monitor.total_pixels,
           g_monitor.total_pixels / 1e6);
    
    printf("\n🚀 IMPLEMENTATION USAGE:\n");
    printf("────────────────────────────────────────────────────────────\n");
    unsigned long total_ops = g_monitor.scalar_ops + g_monitor.neon_ops + g_monitor.metal_ops;
    if (total_ops > 0) {
        printf("  Scalar:  %6lu ops (%5.1f%%)\n", 
               g_monitor.scalar_ops,
               (g_monitor.scalar_ops * 100.0) / total_ops);
        printf("  NEON:    %6lu ops (%5.1f%%)\n", 
               g_monitor.neon_ops,
               (g_monitor.neon_ops * 100.0) / total_ops);
        printf("  Metal:   %6lu ops (%5.1f%%)\n", 
               g_monitor.metal_ops,
               (g_monitor.metal_ops * 100.0) / total_ops);
    }
    
    printf("\n⚡ PERFORMANCE METRICS:\n");
    printf("────────────────────────────────────────────────────────────\n");
    printf("  Throughput:      %.1f MP/s\n", g_monitor.pixels_per_second / 1e6);
    printf("  Avg Tile Time:   %.3f ms\n", g_monitor.avg_tile_time * 1000);
    
    printf("\n🎯 ADAPTIVE THRESHOLDS:\n");
    printf("────────────────────────────────────────────────────────────\n");
    printf("  NEON Threshold:  %zu pixels\n", g_monitor.current_neon_threshold);
    printf("  Metal Threshold: %zu pixels (%dx%d)\n", 
           g_monitor.current_metal_threshold,
           (int)sqrt(g_monitor.current_metal_threshold),
           (int)sqrt(g_monitor.current_metal_threshold));
    
    printf("\n═══════════════════════════════════════════════════════════\n\n");
}

void opj_perf_export_csv(const char* filename)
{
    if (!g_monitor.monitoring_enabled) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "Metric,Value,Unit\n");
    fprintf(f, "Total Time,%.6f,seconds\n", g_monitor.total_time);
    fprintf(f, "MCT Time,%.6f,seconds\n", g_monitor.mct_time);
    fprintf(f, "DWT Time,%.6f,seconds\n", g_monitor.dwt_time);
    fprintf(f, "T1 Time,%.6f,seconds\n", g_monitor.t1_time);
    fprintf(f, "MCT Operations,%lu,count\n", g_monitor.mct_operations);
    fprintf(f, "DWT Operations,%lu,count\n", g_monitor.dwt_operations);
    fprintf(f, "T1 Operations,%lu,count\n", g_monitor.t1_operations);
    fprintf(f, "Scalar Operations,%lu,count\n", g_monitor.scalar_ops);
    fprintf(f, "NEON Operations,%lu,count\n", g_monitor.neon_ops);
    fprintf(f, "Metal Operations,%lu,count\n", g_monitor.metal_ops);
    fprintf(f, "Total Pixels,%lu,pixels\n", g_monitor.total_pixels);
    fprintf(f, "Throughput,%.2f,MP/s\n", g_monitor.pixels_per_second / 1e6);
    fprintf(f, "NEON Threshold,%zu,pixels\n", g_monitor.current_neon_threshold);
    fprintf(f, "Metal Threshold,%zu,pixels\n", g_monitor.current_metal_threshold);
    
    fclose(f);
}

/* ========================================================================
 * Profiling Helpers
 * ======================================================================== */

typedef struct {
    double start_time;
    const char* operation_name;
} opj_timer;

opj_timer opj_timer_start(const char* name)
{
    opj_timer t;
    t.start_time = opj_get_wall_time();
    t.operation_name = name;
    return t;
}

double opj_timer_stop(opj_timer* t)
{
    double elapsed = opj_get_wall_time() - t->start_time;
    return elapsed;
}

void opj_timer_print(opj_timer* t)
{
    double elapsed = opj_timer_stop(t);
    printf("[TIMER] %s: %.3f ms\n", t->operation_name, elapsed * 1000);
}
