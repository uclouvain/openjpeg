/*
 * Copyright (c) 2005, Hervé Drolon, FreeImage Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __J2K_LIB_H
#define __J2K_LIB_H
/**
@file j2k_lib.h
@brief Internal functions

The functions in J2K_LIB.C are internal utilities mainly used for memory management.
*/

#ifndef __GNUC__
#define __attribute__(x) /* */
#endif

/** @defgroup MISC MISC - Miscellaneous internal functions */
/*@{*/

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */

/**
Difference in successive opj_clock() calls tells you the elapsed time
@return Returns time in seconds
*/
double opj_clock(void);

/**
Allocate a memory block with elements initialized to 0
@param size Bytes to allocate
@return Returns a void pointer to the allocated space, or NULL if there is insufficient memory available
*/
void* __attribute__ ((malloc)) opj_malloc( size_t size );

/**
Allocate memory aligned to a 16 byte boundry
@param size Bytes to allocate
@return Returns a void pointer to the allocated space, or NULL if there is insufficient memory available
*/
#ifdef WIN32

#ifdef __GNUC__
#include <mm_malloc.h>
#else /* MSVC, Intel C++ */
#include <malloc.h>
#endif

#ifdef _mm_malloc
  #define opj_aligned_malloc(size) _mm_malloc(size, 16)
  #else
  #define opj_aligned_malloc(size) malloc(size)
 #endif
 
 #ifdef _mm_free
  #define opj_aligned_free(m) _mm_free(m)
  #else
  #define opj_aligned_free(m) free(m)
 #endif

#else /* Not WIN32 */

/* Linux x86_64 and OSX always align allocations to 16 bytes */
#if defined(__amd64__) || defined(__APPLE__)
#define opj_aligned_malloc(size) malloc(size)
#else
extern int posix_memalign (void **, size_t, size_t);

static INLINE void* __attribute__ ((malloc)) opj_aligned_malloc(size_t size){
	void* mem = NULL;
	posix_memalign(&mem, 16, size);
	return mem;
}
#endif

#define opj_aligned_free(m) free(m)

#endif

/**
Reallocate memory blocks.
@param memblock Pointer to previously allocated memory block
@param size New size in bytes
@return Returns a void pointer to the reallocated (and possibly moved) memory block
*/
void* __attribute__ ((malloc)) opj_realloc( void *memblock, size_t size );

/**
Deallocates or frees a memory block.
@param memblock Previously allocated memory block to be freed
*/
void opj_free( void *memblock );

/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __J2K_LIB_H */

