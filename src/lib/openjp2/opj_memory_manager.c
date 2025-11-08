/*
 * Cache-Aligned Memory Manager for OpenJPEG
 *
 * Copyright (c) 2025 Jakub Jirák (ThinkDifferent.blog)
 * 
 * Optimized memory allocation for SIMD and cache efficiency
 */

#include "opj_includes.h"
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

/* ========================================================================
 * Cache Line Detection
 * ======================================================================== */

static size_t g_cache_line_size = 0;
static size_t g_page_size = 0;

void opj_detect_cache_params(void)
{
    if (g_cache_line_size != 0) return;
    
#ifdef __APPLE__
    size_t size = sizeof(g_cache_line_size);
    sysctlbyname("hw.cachelinesize", &g_cache_line_size, &size, NULL, 0);
    
    size = sizeof(g_page_size);
    sysctlbyname("hw.pagesize", &g_page_size, &size, NULL, 0);
#elif defined(__linux__)
    g_cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    g_page_size = sysconf(_SC_PAGESIZE);
#endif
    
    /* Fallback defaults */
    if (g_cache_line_size == 0) g_cache_line_size = 64;
    if (g_page_size == 0) g_page_size = 4096;
}

size_t opj_get_cache_line_size(void)
{
    if (g_cache_line_size == 0) opj_detect_cache_params();
    return g_cache_line_size;
}

size_t opj_get_page_size(void)
{
    if (g_page_size == 0) opj_detect_cache_params();
    return g_page_size;
}

/* ========================================================================
 * Memory Pool for Fast Allocation
 * ======================================================================== */

#define POOL_SIZE 16
#define MAX_BLOCK_SIZE (4 * 1024 * 1024)  /* 4 MB */

typedef struct memory_block {
    void *ptr;
    size_t size;
    size_t alignment;
    bool in_use;
    struct memory_block *next;
} memory_block_t;

typedef struct {
    memory_block_t *blocks;
    size_t num_blocks;
    size_t total_allocated;
    size_t peak_usage;
    atomic_uint alloc_count;
    atomic_uint free_count;
    pthread_mutex_t lock;
} memory_pool_t;

static memory_pool_t g_pool = {0};
static bool g_pool_initialized = false;

void opj_memory_pool_init(void)
{
    if (g_pool_initialized) return;
    
    pthread_mutex_init(&g_pool.lock, NULL);
    g_pool.blocks = NULL;
    g_pool.num_blocks = 0;
    g_pool.total_allocated = 0;
    g_pool.peak_usage = 0;
    atomic_store(&g_pool.alloc_count, 0);
    atomic_store(&g_pool.free_count, 0);
    
    g_pool_initialized = true;
}

void opj_memory_pool_cleanup(void)
{
    if (!g_pool_initialized) return;
    
    pthread_mutex_lock(&g_pool.lock);
    
    memory_block_t *block = g_pool.blocks;
    while (block) {
        memory_block_t *next = block->next;
        free(block->ptr);
        free(block);
        block = next;
    }
    
    g_pool.blocks = NULL;
    g_pool.num_blocks = 0;
    
    pthread_mutex_unlock(&g_pool.lock);
    pthread_mutex_destroy(&g_pool.lock);
    
    g_pool_initialized = false;
}

/* ========================================================================
 * Aligned Allocation
 * ======================================================================== */

/**
 * Allocate aligned memory
 * Alignment must be power of 2
 */
void* opj_aligned_malloc(size_t size, size_t alignment)
{
    if (!g_pool_initialized) opj_memory_pool_init();
    
    atomic_fetch_add(&g_pool.alloc_count, 1);
    
    /* Ensure alignment is at least cache line size */
    size_t cache_line = opj_get_cache_line_size();
    if (alignment < cache_line) alignment = cache_line;
    
    /* Try to find free block in pool */
    pthread_mutex_lock(&g_pool.lock);
    
    memory_block_t *block = g_pool.blocks;
    while (block) {
        if (!block->in_use && 
            block->size >= size && 
            block->alignment == alignment) {
            block->in_use = true;
            pthread_mutex_unlock(&g_pool.lock);
            memset(block->ptr, 0, size);
            return block->ptr;
        }
        block = block->next;
    }
    
    pthread_mutex_unlock(&g_pool.lock);
    
    /* Allocate new block */
    void *ptr = NULL;
    
#ifdef _WIN32
    ptr = _aligned_malloc(size, alignment);
#else
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
#endif
    
    if (!ptr) return NULL;
    
    /* Add to pool */
    memory_block_t *new_block = malloc(sizeof(memory_block_t));
    if (new_block) {
        new_block->ptr = ptr;
        new_block->size = size;
        new_block->alignment = alignment;
        new_block->in_use = true;
        
        pthread_mutex_lock(&g_pool.lock);
        new_block->next = g_pool.blocks;
        g_pool.blocks = new_block;
        g_pool.num_blocks++;
        g_pool.total_allocated += size;
        if (g_pool.total_allocated > g_pool.peak_usage) {
            g_pool.peak_usage = g_pool.total_allocated;
        }
        pthread_mutex_unlock(&g_pool.lock);
    }
    
    memset(ptr, 0, size);
    return ptr;
}

void opj_aligned_free(void *ptr)
{
    if (!ptr) return;
    
    atomic_fetch_add(&g_pool.free_count, 1);
    
    /* Mark block as free in pool */
    pthread_mutex_lock(&g_pool.lock);
    
    memory_block_t *block = g_pool.blocks;
    while (block) {
        if (block->ptr == ptr) {
            block->in_use = false;
            g_pool.total_allocated -= block->size;
            pthread_mutex_unlock(&g_pool.lock);
            return;
        }
        block = block->next;
    }
    
    pthread_mutex_unlock(&g_pool.lock);
    
    /* Not in pool, free directly */
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

/* ========================================================================
 * Prefetch Hints
 * ======================================================================== */

void opj_prefetch_read(const void *addr)
{
#if defined(__GNUC__) || defined(__clang__)
    __builtin_prefetch(addr, 0, 3);  /* Read, high temporal locality */
#elif defined(_MSC_VER)
    _mm_prefetch((const char*)addr, _MM_HINT_T0);
#endif
}

void opj_prefetch_write(const void *addr)
{
#if defined(__GNUC__) || defined(__clang__)
    __builtin_prefetch(addr, 1, 3);  /* Write, high temporal locality */
#elif defined(_MSC_VER)
    _mm_prefetch((const char*)addr, _MM_HINT_T0);
#endif
}

/**
 * Prefetch multiple cache lines ahead
 */
void opj_prefetch_range(const void *addr, size_t size)
{
    size_t cache_line = opj_get_cache_line_size();
    const char *ptr = (const char*)addr;
    const char *end = ptr + size;
    
    while (ptr < end) {
        opj_prefetch_read(ptr);
        ptr += cache_line;
    }
}

/* ========================================================================
 * Cache-Aware Data Structures
 * ======================================================================== */

/**
 * Allocate array with padding to avoid false sharing
 */
void* opj_aligned_array_malloc(size_t elem_size, size_t num_elems)
{
    size_t cache_line = opj_get_cache_line_size();
    
    /* Pad element size to cache line */
    size_t padded_elem_size = ((elem_size + cache_line - 1) / cache_line) * cache_line;
    size_t total_size = padded_elem_size * num_elems;
    
    return opj_aligned_malloc(total_size, cache_line);
}

/**
 * Get element with padding
 */
void* opj_aligned_array_get(void *array, size_t elem_size, size_t index)
{
    size_t cache_line = opj_get_cache_line_size();
    size_t padded_elem_size = ((elem_size + cache_line - 1) / cache_line) * cache_line;
    
    return (char*)array + (padded_elem_size * index);
}

/* ========================================================================
 * Memory Copy with Prefetch
 * ======================================================================== */

void opj_memcpy_prefetch(void *dst, const void *src, size_t size)
{
    size_t cache_line = opj_get_cache_line_size();
    const char *src_ptr = (const char*)src;
    char *dst_ptr = (char*)dst;
    
    /* Prefetch ahead */
    const size_t prefetch_distance = 8 * cache_line;
    
    size_t i = 0;
    while (i + prefetch_distance < size) {
        opj_prefetch_read(src_ptr + i + prefetch_distance);
        opj_prefetch_write(dst_ptr + i + prefetch_distance);
        
        memcpy(dst_ptr + i, src_ptr + i, cache_line);
        i += cache_line;
    }
    
    /* Copy remainder */
    if (i < size) {
        memcpy(dst_ptr + i, src_ptr + i, size - i);
    }
}

/* ========================================================================
 * Tile Memory Layout Optimization
 * ======================================================================== */

typedef struct {
    void *base_ptr;
    size_t tile_width;
    size_t tile_height;
    size_t elem_size;
    size_t row_stride;  /* Aligned to cache line */
} opj_tile_buffer_t;

opj_tile_buffer_t* opj_tile_buffer_create(
    size_t tile_width,
    size_t tile_height,
    size_t elem_size)
{
    opj_tile_buffer_t *buf = malloc(sizeof(opj_tile_buffer_t));
    if (!buf) return NULL;
    
    size_t cache_line = opj_get_cache_line_size();
    
    /* Align row stride to cache line */
    buf->row_stride = ((tile_width * elem_size + cache_line - 1) / cache_line) * cache_line;
    
    size_t total_size = buf->row_stride * tile_height;
    buf->base_ptr = opj_aligned_malloc(total_size, cache_line);
    
    if (!buf->base_ptr) {
        free(buf);
        return NULL;
    }
    
    buf->tile_width = tile_width;
    buf->tile_height = tile_height;
    buf->elem_size = elem_size;
    
    return buf;
}

void* opj_tile_buffer_get_row(opj_tile_buffer_t *buf, size_t row)
{
    return (char*)buf->base_ptr + (row * buf->row_stride);
}

void opj_tile_buffer_destroy(opj_tile_buffer_t *buf)
{
    if (!buf) return;
    opj_aligned_free(buf->base_ptr);
    free(buf);
}

/* ========================================================================
 * NUMA-Aware Allocation (for multi-socket systems)
 * ======================================================================== */

#ifdef __linux__
#include <numa.h>

void* opj_numa_alloc_local(size_t size, size_t alignment)
{
    if (numa_available() < 0) {
        return opj_aligned_malloc(size, alignment);
    }
    
    /* Allocate on current NUMA node */
    void *ptr = numa_alloc_local(size);
    if (!ptr) return NULL;
    
    /* Check alignment */
    if ((uintptr_t)ptr % alignment != 0) {
        numa_free(ptr, size);
        return opj_aligned_malloc(size, alignment);
    }
    
    return ptr;
}

void opj_numa_free(void *ptr, size_t size)
{
    if (numa_available() < 0) {
        opj_aligned_free(ptr);
    } else {
        numa_free(ptr, size);
    }
}
#else
void* opj_numa_alloc_local(size_t size, size_t alignment)
{
    return opj_aligned_malloc(size, alignment);
}

void opj_numa_free(void *ptr, size_t size)
{
    (void)size;
    opj_aligned_free(ptr);
}
#endif

/* ========================================================================
 * Statistics and Reporting
 * ======================================================================== */

void opj_memory_print_stats(void)
{
    if (!g_pool_initialized) return;
    
    pthread_mutex_lock(&g_pool.lock);
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              Memory Manager Statistics                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Pool Statistics:\n");
    printf("    Total Blocks:     %zu\n", g_pool.num_blocks);
    printf("    Currently Used:   %zu bytes (%.1f MB)\n", 
           g_pool.total_allocated, g_pool.total_allocated / (1024.0 * 1024.0));
    printf("    Peak Usage:       %zu bytes (%.1f MB)\n",
           g_pool.peak_usage, g_pool.peak_usage / (1024.0 * 1024.0));
    
    /* Count free blocks */
    size_t free_blocks = 0;
    size_t free_bytes = 0;
    memory_block_t *block = g_pool.blocks;
    while (block) {
        if (!block->in_use) {
            free_blocks++;
            free_bytes += block->size;
        }
        block = block->next;
    }
    
    printf("    Free Blocks:      %zu (%zu bytes)\n", free_blocks, free_bytes);
    printf("\n");
    
    printf("  Allocation Statistics:\n");
    printf("    Total Allocs:     %u\n", atomic_load(&g_pool.alloc_count));
    printf("    Total Frees:      %u\n", atomic_load(&g_pool.free_count));
    printf("    Active Allocs:    %u\n", 
           atomic_load(&g_pool.alloc_count) - atomic_load(&g_pool.free_count));
    printf("\n");
    
    printf("  Cache Parameters:\n");
    printf("    Cache Line Size:  %zu bytes\n", opj_get_cache_line_size());
    printf("    Page Size:        %zu bytes\n", opj_get_page_size());
    printf("\n");
    
    pthread_mutex_unlock(&g_pool.lock);
}

void opj_memory_get_stats(
    size_t *total_allocated,
    size_t *peak_usage,
    size_t *num_blocks)
{
    if (!g_pool_initialized) {
        if (total_allocated) *total_allocated = 0;
        if (peak_usage) *peak_usage = 0;
        if (num_blocks) *num_blocks = 0;
        return;
    }
    
    pthread_mutex_lock(&g_pool.lock);
    
    if (total_allocated) *total_allocated = g_pool.total_allocated;
    if (peak_usage) *peak_usage = g_pool.peak_usage;
    if (num_blocks) *num_blocks = g_pool.num_blocks;
    
    pthread_mutex_unlock(&g_pool.lock);
}

/* ========================================================================
 * Memory Advise (for large allocations)
 * ======================================================================== */

void opj_memory_advise_sequential(void *addr, size_t size)
{
#ifdef __linux__
    madvise(addr, size, MADV_SEQUENTIAL);
#elif defined(__APPLE__)
    madvise(addr, size, MADV_SEQUENTIAL);
#endif
}

void opj_memory_advise_random(void *addr, size_t size)
{
#ifdef __linux__
    madvise(addr, size, MADV_RANDOM);
#elif defined(__APPLE__)
    madvise(addr, size, MADV_RANDOM);
#endif
}

void opj_memory_advise_willneed(void *addr, size_t size)
{
#ifdef __linux__
    madvise(addr, size, MADV_WILLNEED);
#elif defined(__APPLE__)
    madvise(addr, size, MADV_WILLNEED);
#endif
}
