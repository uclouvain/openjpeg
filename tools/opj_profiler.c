/*
 * OpenJPEG Performance Profiler
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Detailed profiling tool for optimization analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>
#include <math.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach/mach_time.h>
#include <sys/sysctl.h>
#endif

/* ========================================================================
 * High-Resolution Timer
 * ======================================================================== */

static double g_timer_scale = 0.0;

void init_timer(void)
{
#ifdef __APPLE__
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    g_timer_scale = 1e-9 * info.numer / info.denom;
#else
    g_timer_scale = 1e-9;
#endif
}

double get_time_ns(void)
{
    if (g_timer_scale == 0.0) init_timer();
    
#ifdef __APPLE__
    return mach_absolute_time() * g_timer_scale * 1e9;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
#endif
}

double get_time_us(void)
{
    return get_time_ns() / 1000.0;
}

double get_time_ms(void)
{
    return get_time_ns() / 1e6;
}

/* ========================================================================
 * Profiling Event Structure
 * ======================================================================== */

#define MAX_EVENTS 1024
#define MAX_NAME_LEN 64

typedef struct {
    char name[MAX_NAME_LEN];
    double start_time;
    double end_time;
    double duration;
    uint32_t thread_id;
    uint32_t call_count;
    double min_duration;
    double max_duration;
    double total_duration;
} profile_event_t;

typedef struct {
    profile_event_t events[MAX_EVENTS];
    atomic_uint event_count;
    atomic_bool enabled;
    pthread_mutex_t lock;
    double start_time;
} profiler_context_t;

static profiler_context_t g_profiler = {0};
static __thread uint32_t t_thread_id = 0;

/* ========================================================================
 * Profiler Control
 * ======================================================================== */

void opj_profiler_init(void)
{
    atomic_store(&g_profiler.event_count, 0);
    atomic_store(&g_profiler.enabled, true);
    pthread_mutex_init(&g_profiler.lock, NULL);
    g_profiler.start_time = get_time_ms();
    
    /* Initialize all events */
    for (int i = 0; i < MAX_EVENTS; i++) {
        g_profiler.events[i].call_count = 0;
        g_profiler.events[i].total_duration = 0.0;
        g_profiler.events[i].min_duration = 1e9;
        g_profiler.events[i].max_duration = 0.0;
    }
}

void opj_profiler_enable(bool enable)
{
    atomic_store(&g_profiler.enabled, enable);
}

void opj_profiler_reset(void)
{
    pthread_mutex_lock(&g_profiler.lock);
    atomic_store(&g_profiler.event_count, 0);
    g_profiler.start_time = get_time_ms();
    pthread_mutex_unlock(&g_profiler.lock);
}

/* ========================================================================
 * Event Recording
 * ======================================================================== */

typedef struct {
    const char *name;
    double start_time;
    uint32_t event_idx;
} profile_scope_t;

profile_scope_t opj_profiler_begin(const char *name)
{
    profile_scope_t scope = {0};
    
    if (!atomic_load(&g_profiler.enabled)) {
        return scope;
    }
    
    scope.name = name;
    scope.start_time = get_time_us();
    
    /* Find or create event */
    pthread_mutex_lock(&g_profiler.lock);
    
    uint32_t idx = 0;
    uint32_t count = atomic_load(&g_profiler.event_count);
    
    for (idx = 0; idx < count; idx++) {
        if (strcmp(g_profiler.events[idx].name, name) == 0) {
            break;
        }
    }
    
    if (idx == count && count < MAX_EVENTS) {
        strncpy(g_profiler.events[idx].name, name, MAX_NAME_LEN - 1);
        atomic_fetch_add(&g_profiler.event_count, 1);
    }
    
    scope.event_idx = idx;
    
    pthread_mutex_unlock(&g_profiler.lock);
    
    if (t_thread_id == 0) {
        t_thread_id = (uint32_t)(uintptr_t)pthread_self();
    }
    
    return scope;
}

void opj_profiler_end(profile_scope_t scope)
{
    if (!atomic_load(&g_profiler.enabled)) {
        return;
    }
    
    double end_time = get_time_us();
    double duration = end_time - scope.start_time;
    
    pthread_mutex_lock(&g_profiler.lock);
    
    if (scope.event_idx < MAX_EVENTS) {
        profile_event_t *event = &g_profiler.events[scope.event_idx];
        
        event->call_count++;
        event->total_duration += duration;
        
        if (duration < event->min_duration) {
            event->min_duration = duration;
        }
        if (duration > event->max_duration) {
            event->max_duration = duration;
        }
        
        event->thread_id = t_thread_id;
    }
    
    pthread_mutex_unlock(&g_profiler.lock);
}

/* ========================================================================
 * Automatic Scope Profiling (RAII style)
 * ======================================================================== */

#define PROFILE_SCOPE(name) \
    profile_scope_t __profile_scope_##__LINE__ = opj_profiler_begin(name); \
    (void)__profile_scope_##__LINE__

#define PROFILE_END(name) \
    opj_profiler_end(__profile_scope_##__LINE__)

/* ========================================================================
 * System Information
 * ======================================================================== */

typedef struct {
    char cpu_brand[128];
    int num_cores;
    int num_performance_cores;
    int num_efficiency_cores;
    size_t l1_cache_size;
    size_t l2_cache_size;
    size_t l3_cache_size;
    size_t memory_size;
} system_info_t;

void get_system_info(system_info_t *info)
{
    memset(info, 0, sizeof(system_info_t));
    
#ifdef __APPLE__
    size_t size = sizeof(info->cpu_brand);
    sysctlbyname("machdep.cpu.brand_string", info->cpu_brand, &size, NULL, 0);
    
    size = sizeof(info->num_cores);
    sysctlbyname("hw.ncpu", &info->num_cores, &size, NULL, 0);
    
    size = sizeof(info->num_performance_cores);
    sysctlbyname("hw.perflevel0.logicalcpu", &info->num_performance_cores, &size, NULL, 0);
    
    size = sizeof(info->num_efficiency_cores);
    sysctlbyname("hw.perflevel1.logicalcpu", &info->num_efficiency_cores, &size, NULL, 0);
    
    size = sizeof(info->l1_cache_size);
    sysctlbyname("hw.l1dcachesize", &info->l1_cache_size, &size, NULL, 0);
    
    size = sizeof(info->l2_cache_size);
    sysctlbyname("hw.l2cachesize", &info->l2_cache_size, &size, NULL, 0);
    
    size = sizeof(info->l3_cache_size);
    sysctlbyname("hw.l3cachesize", &info->l3_cache_size, &size, NULL, 0);
    
    size = sizeof(info->memory_size);
    sysctlbyname("hw.memsize", &info->memory_size, &size, NULL, 0);
#else
    strcpy(info->cpu_brand, "Unknown");
    info->num_cores = sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

void print_system_info(void)
{
    system_info_t info;
    get_system_info(&info);
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              System Information                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  CPU:               %s\n", info.cpu_brand);
    printf("  Total Cores:       %d\n", info.num_cores);
    
    if (info.num_performance_cores > 0) {
        printf("  Performance Cores: %d\n", info.num_performance_cores);
        printf("  Efficiency Cores:  %d\n", info.num_efficiency_cores);
    }
    
    if (info.l1_cache_size > 0) {
        printf("  L1 Cache:          %zu KB\n", info.l1_cache_size / 1024);
    }
    if (info.l2_cache_size > 0) {
        printf("  L2 Cache:          %zu KB\n", info.l2_cache_size / 1024);
    }
    if (info.l3_cache_size > 0) {
        printf("  L3 Cache:          %zu MB\n", info.l3_cache_size / (1024 * 1024));
    }
    if (info.memory_size > 0) {
        printf("  RAM:               %zu GB\n", info.memory_size / (1024 * 1024 * 1024));
    }
    
    printf("\n");
}

/* ========================================================================
 * Report Generation
 * ======================================================================== */

int compare_events_by_total(const void *a, const void *b)
{
    const profile_event_t *ea = (const profile_event_t*)a;
    const profile_event_t *eb = (const profile_event_t*)b;
    
    if (ea->total_duration > eb->total_duration) return -1;
    if (ea->total_duration < eb->total_duration) return 1;
    return 0;
}

void opj_profiler_print_report(void)
{
    if (atomic_load(&g_profiler.event_count) == 0) {
        printf("No profiling data available.\n");
        return;
    }
    
    double total_time = get_time_ms() - g_profiler.start_time;
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              Performance Profile Report                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Total Profiling Time: %.3f ms\n\n", total_time);
    
    /* Sort events by total time */
    uint32_t count = atomic_load(&g_profiler.event_count);
    profile_event_t *sorted = malloc(count * sizeof(profile_event_t));
    memcpy(sorted, g_profiler.events, count * sizeof(profile_event_t));
    qsort(sorted, count, sizeof(profile_event_t), compare_events_by_total);
    
    /* Print table header */
    printf("%-30s │ Calls │   Total   │   Avg    │   Min    │   Max    │ %%Time\n", "Function");
    printf("───────────────────────────────┼───────┼───────────┼──────────┼──────────┼──────────┼───────\n");
    
    double cumulative_percent = 0.0;
    
    for (uint32_t i = 0; i < count; i++) {
        profile_event_t *e = &sorted[i];
        
        if (e->call_count == 0) continue;
        
        double avg = e->total_duration / e->call_count;
        double percent = (e->total_duration / (total_time * 1000.0)) * 100.0;
        cumulative_percent += percent;
        
        printf("%-30s │ %5u │ %7.2f ms │ %6.2f μs │ %6.2f μs │ %6.2f μs │ %5.1f%%\n",
               e->name,
               e->call_count,
               e->total_duration / 1000.0,
               avg,
               e->min_duration,
               e->max_duration,
               percent);
        
        /* Show top 10 only, then summarize rest */
        if (i == 9 && count > 10) {
            printf("... and %u more functions (%.1f%% of time)\n", 
                   count - 10, 100.0 - cumulative_percent);
            break;
        }
    }
    
    printf("\n");
    
    free(sorted);
}

void opj_profiler_export_csv(const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Failed to open %s for writing\n", filename);
        return;
    }
    
    fprintf(f, "Function,Calls,Total_us,Avg_us,Min_us,Max_us\n");
    
    uint32_t count = atomic_load(&g_profiler.event_count);
    for (uint32_t i = 0; i < count; i++) {
        profile_event_t *e = &g_profiler.events[i];
        
        if (e->call_count == 0) continue;
        
        double avg = e->total_duration / e->call_count;
        
        fprintf(f, "%s,%u,%.2f,%.2f,%.2f,%.2f\n",
                e->name,
                e->call_count,
                e->total_duration,
                avg,
                e->min_duration,
                e->max_duration);
    }
    
    fclose(f);
    printf("Profile data exported to %s\n", filename);
}

/* ========================================================================
 * Flame Graph Generation
 * ======================================================================== */

void opj_profiler_export_flamegraph(const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    uint32_t count = atomic_load(&g_profiler.event_count);
    
    for (uint32_t i = 0; i < count; i++) {
        profile_event_t *e = &g_profiler.events[i];
        if (e->call_count > 0) {
            /* Flame graph format: function_name count */
            fprintf(f, "%s %u\n", e->name, (uint32_t)(e->total_duration));
        }
    }
    
    fclose(f);
    printf("Flame graph data exported to %s\n", filename);
}

/* ========================================================================
 * Demo Usage
 * ======================================================================== */

void example_function_1(void)
{
    profile_scope_t scope = opj_profiler_begin("example_function_1");
    
    /* Simulate work */
    volatile int sum = 0;
    for (int i = 0; i < 100000; i++) {
        sum += i;
    }
    
    opj_profiler_end(scope);
}

void example_function_2(void)
{
    profile_scope_t scope = opj_profiler_begin("example_function_2");
    
    /* Simulate work */
    volatile double result = 0.0;
    for (int i = 0; i < 50000; i++) {
        result += sqrt(i);
    }
    
    opj_profiler_end(scope);
}

void example_nested(void)
{
    profile_scope_t scope = opj_profiler_begin("example_nested");
    
    example_function_1();
    example_function_2();
    
    opj_profiler_end(scope);
}

int main(int argc, char **argv)
{
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                              ║\n");
    printf("║              OpenJPEG Performance Profiler                  ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    print_system_info();
    
    printf("Running profiling demo...\n\n");
    
    opj_profiler_init();
    
    /* Simulate profiling session */
    for (int iter = 0; iter < 100; iter++) {
        example_function_1();
        example_function_2();
        example_nested();
    }
    
    opj_profiler_print_report();
    
    if (argc > 1) {
        opj_profiler_export_csv(argv[1]);
    } else {
        opj_profiler_export_csv("profile_data.csv");
    }
    
    printf("\n");
    printf("💡 Usage Tips:\n");
    printf("   • Wrap functions with opj_profiler_begin/end\n");
    printf("   • Export CSV for detailed analysis\n");
    printf("   • Use flame graphs for visualization\n");
    printf("   • Profile both debug and release builds\n");
    printf("\n");
    
    return 0;
}
