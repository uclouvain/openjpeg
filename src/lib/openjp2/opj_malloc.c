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
#include <stdlib.h>

static inline void *opj_aligned_alloc(size_t alignment, size_t size)
{
#ifndef HAVE_ALIGNED_ALLOC
  /* older linux */
#ifdef HAVE_MEMALIGN
  assert( size % alignment == 0 );
  return memalign( alignment, size );
#endif /* HAVE_MEMALIGN */

/* _MSC_VER */
#ifdef HAVE__ALIGNED_MALLOC
  return _aligned_malloc( alignment, size );
#endif /* HAVE__ALIGNED_MALLOC */

/* MacOSX / clang */
#if defined(HAVE_POSIX_MEMALIGN) && !defined(HAVE_MEMALIGN)
  void* ptr;
  if (posix_memalign (&ptr, alignment, size))
  {
    ptr = NULL;
  }
  return ptr;
#endif /* HAVE_POSIX_MEMALIGN */

#else /* HAVE_ALIGNED_ALLOC */
  return aligned_alloc( alignment, size );
#endif /* HAVE_ALIGNED_ALLOC */
/* TODO: _mm_malloc(x,y) */
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
  return opj_aligned_alloc(size,16);
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
