/*
 * Thread Pool for OpenJPEG
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Efficient multi-threaded tile processing
 */

#include "opj_includes.h"
#include <pthread.h>
#include <stdatomic.h>

/* ========================================================================
 * Work Queue Structure
 * ======================================================================== */

typedef enum {
    WORK_MCT,
    WORK_DWT,
    WORK_T1,
    WORK_CUSTOM
} work_type_t;

typedef struct work_item {
    work_type_t type;
    void *data;
    void (*func)(void *);
    struct work_item *next;
    atomic_bool completed;
} work_item_t;

typedef struct {
    work_item_t *head;
    work_item_t *tail;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    size_t count;
} work_queue_t;

/* ========================================================================
 * Thread Pool Structure
 * ======================================================================== */

typedef struct {
    pthread_t *threads;
    int num_threads;
    work_queue_t queue;
    atomic_bool shutdown;
    atomic_int active_threads;
    
    /* Statistics */
    atomic_ulong tasks_completed;
    atomic_ulong tasks_submitted;
    double total_wait_time;
    
    /* CPU affinity control */
    bool use_affinity;
    int *cpu_ids;
} opj_thread_pool_t;

/* ========================================================================
 * Work Queue Operations
 * ======================================================================== */

void work_queue_init(work_queue_t *queue)
{
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void work_queue_destroy(work_queue_t *queue)
{
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->cond);
}

void work_queue_push(work_queue_t *queue, work_item_t *item)
{
    pthread_mutex_lock(&queue->lock);
    
    item->next = NULL;
    atomic_store(&item->completed, false);
    
    if (queue->tail) {
        queue->tail->next = item;
    } else {
        queue->head = item;
    }
    queue->tail = item;
    queue->count++;
    
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->lock);
}

work_item_t* work_queue_pop(work_queue_t *queue, bool wait)
{
    pthread_mutex_lock(&queue->lock);
    
    while (queue->head == NULL && wait) {
        pthread_cond_wait(&queue->cond, &queue->lock);
    }
    
    work_item_t *item = queue->head;
    if (item) {
        queue->head = item->next;
        if (queue->head == NULL) {
            queue->tail = NULL;
        }
        queue->count--;
    }
    
    pthread_mutex_unlock(&queue->lock);
    return item;
}

/* ========================================================================
 * Worker Thread
 * ======================================================================== */

void* worker_thread(void *arg)
{
    opj_thread_pool_t *pool = (opj_thread_pool_t*)arg;
    
    atomic_fetch_add(&pool->active_threads, 1);
    
    while (!atomic_load(&pool->shutdown)) {
        work_item_t *item = work_queue_pop(&pool->queue, true);
        
        if (!item) continue;
        
        /* Execute work */
        if (item->func) {
            item->func(item->data);
        }
        
        /* Mark as completed */
        atomic_store(&item->completed, true);
        atomic_fetch_add(&pool->tasks_completed, 1);
    }
    
    atomic_fetch_sub(&pool->active_threads, 1);
    return NULL;
}

/* ========================================================================
 * Thread Pool Management
 * ======================================================================== */

opj_thread_pool_t* opj_thread_pool_create(int num_threads)
{
    if (num_threads <= 0) {
        /* Auto-detect */
        num_threads = sysconf(_SC_NPROCESSORS_ONLN);
        if (num_threads <= 0) num_threads = 4;
    }
    
    opj_thread_pool_t *pool = calloc(1, sizeof(opj_thread_pool_t));
    if (!pool) return NULL;
    
    pool->num_threads = num_threads;
    pool->threads = calloc(num_threads, sizeof(pthread_t));
    
    if (!pool->threads) {
        free(pool);
        return NULL;
    }
    
    work_queue_init(&pool->queue);
    atomic_store(&pool->shutdown, false);
    atomic_store(&pool->active_threads, 0);
    atomic_store(&pool->tasks_completed, 0);
    atomic_store(&pool->tasks_submitted, 0);
    
    /* Create worker threads */
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            /* Cleanup on failure */
            atomic_store(&pool->shutdown, true);
            for (int j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            work_queue_destroy(&pool->queue);
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }
    
    return pool;
}

void opj_thread_pool_destroy(opj_thread_pool_t *pool)
{
    if (!pool) return;
    
    /* Signal shutdown */
    atomic_store(&pool->shutdown, true);
    
    /* Wake up all threads */
    pthread_mutex_lock(&pool->queue.lock);
    pthread_cond_broadcast(&pool->queue.cond);
    pthread_mutex_unlock(&pool->queue.lock);
    
    /* Wait for threads to finish */
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    work_queue_destroy(&pool->queue);
    free(pool->threads);
    free(pool);
}

/* ========================================================================
 * Work Submission
 * ======================================================================== */

work_item_t* opj_thread_pool_submit(
    opj_thread_pool_t *pool,
    void (*func)(void*),
    void *data)
{
    work_item_t *item = malloc(sizeof(work_item_t));
    if (!item) return NULL;
    
    item->type = WORK_CUSTOM;
    item->func = func;
    item->data = data;
    
    work_queue_push(&pool->queue, item);
    atomic_fetch_add(&pool->tasks_submitted, 1);
    
    return item;
}

void opj_thread_pool_wait(opj_thread_pool_t *pool, work_item_t *item)
{
    if (!item) return;
    
    /* Spin-wait for completion */
    while (!atomic_load(&item->completed)) {
        sched_yield();
    }
    
    free(item);
}

void opj_thread_pool_wait_all(opj_thread_pool_t *pool)
{
    /* Wait until queue is empty and all threads idle */
    while (1) {
        pthread_mutex_lock(&pool->queue.lock);
        size_t count = pool->queue.count;
        pthread_mutex_unlock(&pool->queue.lock);
        
        if (count == 0 && 
            atomic_load(&pool->tasks_completed) == atomic_load(&pool->tasks_submitted)) {
            break;
        }
        
        sched_yield();
    }
}

/* ========================================================================
 * Tile-Based Parallel Processing
 * ======================================================================== */

typedef struct {
    OPJ_INT32 *data;
    size_t tile_offset;
    size_t tile_size;
    void (*process_func)(OPJ_INT32*, size_t);
} tile_work_t;

void tile_worker(void *arg)
{
    tile_work_t *work = (tile_work_t*)arg;
    work->process_func(work->data + work->tile_offset, work->tile_size);
}

void opj_thread_pool_process_tiles(
    opj_thread_pool_t *pool,
    OPJ_INT32 *data,
    size_t total_size,
    size_t tile_size,
    void (*process_func)(OPJ_INT32*, size_t))
{
    size_t num_tiles = (total_size + tile_size - 1) / tile_size;
    work_item_t **items = calloc(num_tiles, sizeof(work_item_t*));
    
    if (!items) return;
    
    /* Submit all tiles */
    for (size_t i = 0; i < num_tiles; i++) {
        tile_work_t *work = malloc(sizeof(tile_work_t));
        if (!work) continue;
        
        work->data = data;
        work->tile_offset = i * tile_size;
        work->tile_size = (i == num_tiles - 1) ? 
                          (total_size - work->tile_offset) : tile_size;
        work->process_func = process_func;
        
        items[i] = opj_thread_pool_submit(pool, tile_worker, work);
    }
    
    /* Wait for all tiles */
    for (size_t i = 0; i < num_tiles; i++) {
        if (items[i]) {
            opj_thread_pool_wait(pool, items[i]);
        }
    }
    
    free(items);
}

/* ========================================================================
 * Parallel MCT
 * ======================================================================== */

typedef struct {
    OPJ_INT32 *c0, *c1, *c2;
    size_t start;
    size_t count;
} mct_work_t;

void mct_worker(void *arg)
{
    mct_work_t *work = (mct_work_t*)arg;
    
    for (size_t i = 0; i < work->count; i++) {
        size_t idx = work->start + i;
        OPJ_INT32 r = work->c0[idx];
        OPJ_INT32 g = work->c1[idx];
        OPJ_INT32 b = work->c2[idx];
        
        OPJ_INT32 y = (r + (g << 1) + b) >> 2;
        OPJ_INT32 u = b - g;
        OPJ_INT32 v = r - g;
        
        work->c0[idx] = y;
        work->c1[idx] = u;
        work->c2[idx] = v;
    }
    
    free(work);
}

void opj_thread_pool_mct_encode(
    opj_thread_pool_t *pool,
    OPJ_INT32 *c0,
    OPJ_INT32 *c1,
    OPJ_INT32 *c2,
    size_t n)
{
    size_t chunk_size = (n + pool->num_threads - 1) / pool->num_threads;
    work_item_t **items = calloc(pool->num_threads, sizeof(work_item_t*));
    
    if (!items) return;
    
    int actual_chunks = 0;
    for (size_t i = 0; i < n; i += chunk_size) {
        mct_work_t *work = malloc(sizeof(mct_work_t));
        if (!work) continue;
        
        work->c0 = c0;
        work->c1 = c1;
        work->c2 = c2;
        work->start = i;
        work->count = (i + chunk_size > n) ? (n - i) : chunk_size;
        
        items[actual_chunks++] = opj_thread_pool_submit(pool, mct_worker, work);
    }
    
    /* Wait for completion */
    for (int i = 0; i < actual_chunks; i++) {
        if (items[i]) {
            opj_thread_pool_wait(pool, items[i]);
        }
    }
    
    free(items);
}

/* ========================================================================
 * Statistics
 * ======================================================================== */

void opj_thread_pool_get_stats(
    opj_thread_pool_t *pool,
    unsigned long *submitted,
    unsigned long *completed,
    int *active_threads)
{
    if (submitted) {
        *submitted = atomic_load(&pool->tasks_submitted);
    }
    
    if (completed) {
        *completed = atomic_load(&pool->tasks_completed);
    }
    
    if (active_threads) {
        *active_threads = atomic_load(&pool->active_threads);
    }
}

void opj_thread_pool_print_stats(opj_thread_pool_t *pool)
{
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              Thread Pool Statistics                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Thread Pool Size:     %d\n", pool->num_threads);
    printf("  Active Threads:       %d\n", atomic_load(&pool->active_threads));
    printf("  Tasks Submitted:      %lu\n", atomic_load(&pool->tasks_submitted));
    printf("  Tasks Completed:      %lu\n", atomic_load(&pool->tasks_completed));
    
    pthread_mutex_lock(&pool->queue.lock);
    printf("  Pending Tasks:        %zu\n", pool->queue.count);
    pthread_mutex_unlock(&pool->queue.lock);
    
    unsigned long submitted = atomic_load(&pool->tasks_submitted);
    unsigned long completed = atomic_load(&pool->tasks_completed);
    
    if (submitted > 0) {
        double completion_rate = (completed * 100.0) / submitted;
        printf("  Completion Rate:      %.1f%%\n", completion_rate);
    }
    
    printf("\n");
}

/* ========================================================================
 * CPU Affinity (Platform-specific)
 * ======================================================================== */

#ifdef __APPLE__
#include <mach/thread_policy.h>
#include <mach/thread_act.h>

void opj_thread_set_affinity(pthread_t thread, int cpu_id)
{
    thread_affinity_policy_data_t policy = { cpu_id };
    thread_policy_set(pthread_mach_thread_np(thread),
                     THREAD_AFFINITY_POLICY,
                     (thread_policy_t)&policy,
                     THREAD_AFFINITY_POLICY_COUNT);
}
#elif defined(__linux__)
void opj_thread_set_affinity(pthread_t thread, int cpu_id)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}
#else
void opj_thread_set_affinity(pthread_t thread, int cpu_id)
{
    /* Not supported on this platform */
    (void)thread;
    (void)cpu_id;
}
#endif

void opj_thread_pool_set_affinity(opj_thread_pool_t *pool, bool enable)
{
    pool->use_affinity = enable;
    
    if (enable) {
        for (int i = 0; i < pool->num_threads; i++) {
            opj_thread_set_affinity(pool->threads[i], i % pool->num_threads);
        }
    }
}
