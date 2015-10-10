/*
 * The copyright in this software is being made available under the 2-clauses 
 * BSD License, included below. This software may be subject to other third 
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2015, Mathieu Malaterre <mathieu.malaterre@gmail.com>
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
#define OPJ_SKIP_POISON
#include "opj_malloc.h"
#include "opj_config_private.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static inline void *opj_aligned_alloc_n(size_t alignment, size_t size)
{
#if defined(HAVE_POSIX_MEMALIGN)
  // aligned_alloc requires c11, restrict to posix_memalign for now. Quote:
  // This function was introduced in POSIX 1003.1d. Although this function is
  // superseded by aligned_alloc, it is more portable to older POSIX systems
  // that do not support ISO C11.
  void* ptr;
  if (posix_memalign (&ptr, alignment, size))
  {
    ptr = NULL;
  }
  return ptr;
  /* older linux */
#elif defined(HAVE_MEMALIGN)
  assert( size % alignment == 0 );
  return memalign( alignment, size );
/* _MSC_VER */
#elif defined(HAVE__ALIGNED_MALLOC)
  return _aligned_malloc( alignment, size );
#else
/* TODO: _mm_malloc(x,y) */
#error missing aligned alloc function
#endif
}
static inline void *opj_aligned_realloc_n(void *ptr, size_t alignment, size_t size)
{
/* no portable aligned realloc */
#if defined(HAVE_POSIX_MEMALIGN) || defined(HAVE_MEMALIGN)
  /* glibc doc states one can mixed aligned malloc with realloc */
  void *r_ptr = realloc( ptr, size );
  /* fast path */
  if( ((uintptr_t)r_ptr & alignment) == 0 )
    return r_ptr;
  /* this is non-trivial to implement a portable aligned realloc, so use a
   * simple approach where we do not need a function that return the size of an
   * allocated array (eg. _msize on Windows, malloc_size on MacOS,
   * malloc_usable_size on systems with glibc) */
  void *a_ptr = opj_aligned_alloc_n(alignment, size);
  /* memory may overlap, do not use memcpy */
  memmove(a_ptr, r_ptr, size);
  free( r_ptr );
  return a_ptr;
/* _MSC_VER */
#elif defined(HAVE__ALIGNED_MALLOC)
  return _aligned_realloc( ptr, size, alignment );
#else
/* TODO: _mm_malloc(x,y) */
#error missing aligned realloc function
#endif
}
void * opj_malloc(size_t size)
{
  return malloc(size);
}
void * opj_calloc(size_t numOfElements, size_t sizeOfElements)
{
  return calloc(numOfElements, sizeOfElements);
}

void *opj_aligned_malloc(size_t size)
{
  return opj_aligned_alloc_n(16u,size);
}
void * opj_aligned_realloc(void *ptr, size_t size)
{
  return opj_aligned_realloc_n(ptr,16u,size);
}

void opj_aligned_free(void* ptr)
{
#ifdef HAVE__ALIGNED_MALLOC
  _aligned_free( ptr );
#else
  free( ptr );
#endif
}

void * opj_realloc(void * m, size_t s)
{
  return realloc(m,s);
}
void opj_free(void * m)
{
  free(m);
}
