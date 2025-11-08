/*
 * Parallel Batch Processing Framework for OpenJPEG
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Multi-threaded CPU + GPU batch processing with load balancing
 */

#include "opj_includes.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_METAL
#import <Metal/Metal.h>
#include "metal_manager.h"
#endif

/* ========================================================================
 * Batch Job Structure
 * ======================================================================== */

typedef enum {
    JOB_MCT_ENCODE,
    JOB_MCT_DECODE,
    JOB_DWT_FORWARD,
    JOB_DWT_INVERSE,
    JOB_T1_ENCODE,
    JOB_T1_DECODE
} job_type_t;

typedef struct {
    job_type_t type;
    void *input_data[3];
    void *output_data[3];
    size_t data_size;
    int width;
    int height;
    int component;
    volatile int completed;
    double execution_time;
} batch_job_t;

/* ========================================================================
 * Batch Processor Context
 * ======================================================================== */

typedef struct {
    /* Job queue */
    batch_job_t *jobs;
    int num_jobs;
    int current_job;
    pthread_mutex_t queue_mutex;
    
    /* Worker threads */
    pthread_t *cpu_threads;
    int num_cpu_threads;
    volatile int shutdown;
    
    /* GPU resources */
#ifdef HAVE_METAL
    id<MTLDevice> metal_device;
    id<MTLCommandQueue> metal_queue;
    int use_metal;
#endif
    
    /* Statistics */
    int jobs_completed;
    double total_cpu_time;
    double total_gpu_time;
    int cpu_job_count;
    int gpu_job_count;
} batch_processor_t;

static batch_processor_t g_processor = {0};

/* ========================================================================
 * Job Execution Functions
 * ======================================================================== */

static void execute_mct_encode_cpu(batch_job_t *job)
{
#ifdef __ARM_NEON
    extern void opj_mct_encode_neon(OPJ_INT32*, OPJ_INT32*, OPJ_INT32*, OPJ_SIZE_T);
    opj_mct_encode_neon(
        (OPJ_INT32*)job->input_data[0],
        (OPJ_INT32*)job->input_data[1],
        (OPJ_INT32*)job->input_data[2],
        job->data_size
    );
#else
    /* Fallback to scalar */
    OPJ_INT32 *r = (OPJ_INT32*)job->input_data[0];
    OPJ_INT32 *g = (OPJ_INT32*)job->input_data[1];
    OPJ_INT32 *b = (OPJ_INT32*)job->input_data[2];
    
    for (size_t i = 0; i < job->data_size; i++) {
        OPJ_INT32 y = (r[i] + 2*g[i] + b[i]) >> 2;
        OPJ_INT32 u = b[i] - g[i];
        OPJ_INT32 v = r[i] - g[i];
        r[i] = y;
        g[i] = u;
        b[i] = v;
    }
#endif
}

static void execute_mct_encode_gpu(batch_job_t *job)
{
#ifdef HAVE_METAL
    if (g_processor.metal_device) {
        opj_metal_mct_encode(
            (OPJ_INT32*)job->input_data[0],
            (OPJ_INT32*)job->input_data[1],
            (OPJ_INT32*)job->input_data[2],
            job->data_size
        );
    }
#endif
}

static void execute_dwt_forward_cpu(batch_job_t *job)
{
    /* DWT implementation would go here */
    /* For now, placeholder */
}

static void execute_job(batch_job_t *job, int use_gpu)
{
    double start_time = opj_clock();
    
    switch (job->type) {
        case JOB_MCT_ENCODE:
            if (use_gpu) {
                execute_mct_encode_gpu(job);
            } else {
                execute_mct_encode_cpu(job);
            }
            break;
            
        case JOB_DWT_FORWARD:
            execute_dwt_forward_cpu(job);
            break;
            
        default:
            break;
    }
    
    job->execution_time = opj_clock() - start_time;
    job->completed = 1;
}

/* ========================================================================
 * Worker Thread Function
 * ======================================================================== */

static void* worker_thread_func(void *arg)
{
    int thread_id = *(int*)arg;
    
    while (!g_processor.shutdown) {
        batch_job_t *job = NULL;
        
        /* Get next job from queue */
        pthread_mutex_lock(&g_processor.queue_mutex);
        if (g_processor.current_job < g_processor.num_jobs) {
            job = &g_processor.jobs[g_processor.current_job];
            g_processor.current_job++;
        }
        pthread_mutex_unlock(&g_processor.queue_mutex);
        
        if (!job) {
            break; /* No more jobs */
        }
        
        /* Execute job on CPU */
        execute_job(job, 0);
        
        /* Update statistics */
        pthread_mutex_lock(&g_processor.queue_mutex);
        g_processor.jobs_completed++;
        g_processor.total_cpu_time += job->execution_time;
        g_processor.cpu_job_count++;
        pthread_mutex_unlock(&g_processor.queue_mutex);
    }
    
    return NULL;
}

/* ========================================================================
 * Batch Processor API
 * ======================================================================== */

/**
 * Initialize batch processor
 */
int opj_batch_init(int num_cpu_threads, int use_metal)
{
    memset(&g_processor, 0, sizeof(g_processor));
    
    /* Initialize mutex */
    pthread_mutex_init(&g_processor.queue_mutex, NULL);
    
    /* Set CPU threads (default to hardware concurrency) */
    if (num_cpu_threads <= 0) {
        num_cpu_threads = sysconf(_SC_NPROCESSORS_ONLN);
    }
    g_processor.num_cpu_threads = num_cpu_threads;
    
#ifdef HAVE_METAL
    /* Initialize Metal if requested */
    if (use_metal) {
        g_processor.metal_device = MTLCreateSystemDefaultDevice();
        if (g_processor.metal_device) {
            g_processor.metal_queue = [g_processor.metal_device newCommandQueue];
            g_processor.use_metal = 1;
            opj_metal_init();
        }
    }
#endif
    
    return 1;
}

/**
 * Add jobs to batch queue
 */
int opj_batch_add_jobs(batch_job_t *jobs, int num_jobs)
{
    pthread_mutex_lock(&g_processor.queue_mutex);
    
    /* Allocate or expand job array */
    batch_job_t *new_jobs = realloc(g_processor.jobs, 
        (g_processor.num_jobs + num_jobs) * sizeof(batch_job_t));
    
    if (!new_jobs) {
        pthread_mutex_unlock(&g_processor.queue_mutex);
        return 0;
    }
    
    /* Copy new jobs */
    memcpy(&new_jobs[g_processor.num_jobs], jobs, num_jobs * sizeof(batch_job_t));
    g_processor.jobs = new_jobs;
    g_processor.num_jobs += num_jobs;
    
    pthread_mutex_unlock(&g_processor.queue_mutex);
    return 1;
}

/**
 * Execute all queued jobs with load balancing
 */
int opj_batch_execute(void)
{
    if (g_processor.num_jobs == 0) {
        return 1;
    }
    
    double start_time = opj_clock();
    
    /* Analyze jobs and decide GPU vs CPU split */
    int large_job_count = 0;
    for (int i = 0; i < g_processor.num_jobs; i++) {
        if (g_processor.jobs[i].data_size >= 512*512) {
            large_job_count++;
        }
    }
    
#ifdef HAVE_METAL
    /* Submit large jobs to GPU */
    if (g_processor.use_metal && large_job_count > 0) {
        for (int i = 0; i < g_processor.num_jobs; i++) {
            if (g_processor.jobs[i].data_size >= 512*512) {
                execute_job(&g_processor.jobs[i], 1);
                g_processor.gpu_job_count++;
            }
        }
    }
#endif
    
    /* Reset job counter for CPU threads */
    g_processor.current_job = 0;
    g_processor.shutdown = 0;
    
    /* Create CPU worker threads */
    g_processor.cpu_threads = malloc(g_processor.num_cpu_threads * sizeof(pthread_t));
    int *thread_ids = malloc(g_processor.num_cpu_threads * sizeof(int));
    
    for (int i = 0; i < g_processor.num_cpu_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&g_processor.cpu_threads[i], NULL, worker_thread_func, &thread_ids[i]);
    }
    
    /* Wait for all CPU threads to complete */
    for (int i = 0; i < g_processor.num_cpu_threads; i++) {
        pthread_join(g_processor.cpu_threads[i], NULL);
    }
    
    free(g_processor.cpu_threads);
    free(thread_ids);
    g_processor.cpu_threads = NULL;
    
    double total_time = opj_clock() - start_time;
    
    /* Print statistics */
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║           Batch Processing Complete                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    printf("  Total Jobs:      %d\n", g_processor.num_jobs);
    printf("  CPU Jobs:        %d (%.1f%%)\n", 
           g_processor.cpu_job_count,
           100.0 * g_processor.cpu_job_count / g_processor.num_jobs);
    printf("  GPU Jobs:        %d (%.1f%%)\n", 
           g_processor.gpu_job_count,
           100.0 * g_processor.gpu_job_count / g_processor.num_jobs);
    printf("  Total Time:      %.2f ms\n", total_time * 1000.0);
    printf("  Throughput:      %.1f jobs/sec\n", g_processor.num_jobs / total_time);
    printf("\n");
    
    return 1;
}

/**
 * Clear all jobs
 */
void opj_batch_clear(void)
{
    pthread_mutex_lock(&g_processor.queue_mutex);
    
    if (g_processor.jobs) {
        free(g_processor.jobs);
        g_processor.jobs = NULL;
    }
    
    g_processor.num_jobs = 0;
    g_processor.current_job = 0;
    g_processor.jobs_completed = 0;
    g_processor.cpu_job_count = 0;
    g_processor.gpu_job_count = 0;
    
    pthread_mutex_unlock(&g_processor.queue_mutex);
}

/**
 * Cleanup batch processor
 */
void opj_batch_cleanup(void)
{
    g_processor.shutdown = 1;
    
    opj_batch_clear();
    
    pthread_mutex_destroy(&g_processor.queue_mutex);
    
#ifdef HAVE_METAL
    if (g_processor.use_metal) {
        opj_metal_cleanup();
        g_processor.metal_device = nil;
        g_processor.metal_queue = nil;
    }
#endif
}

/**
 * Get batch statistics
 */
void opj_batch_get_stats(
    int *total_jobs,
    int *cpu_jobs,
    int *gpu_jobs,
    double *cpu_time,
    double *gpu_time)
{
    pthread_mutex_lock(&g_processor.queue_mutex);
    
    if (total_jobs) *total_jobs = g_processor.num_jobs;
    if (cpu_jobs) *cpu_jobs = g_processor.cpu_job_count;
    if (gpu_jobs) *gpu_jobs = g_processor.gpu_job_count;
    if (cpu_time) *cpu_time = g_processor.total_cpu_time;
    if (gpu_time) *gpu_time = g_processor.total_gpu_time;
    
    pthread_mutex_unlock(&g_processor.queue_mutex);
}
