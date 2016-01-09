/*
 * The copyright in this software is being made available under the 2-clauses 
 * BSD License, included below. This software may be subject to other third 
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2002-2016, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2016, OpenJPEG contributors
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

#ifndef __VECTOR_H
#define __VECTOR_H

/*
Vector - a dynamic array.

*/

typedef struct opj_vec{
	OPJ_INT32 size;     /* current size of vec */
	OPJ_INT32 capacity;  /* maximum size of vec */
	void* *data;		/* array of void* pointers */ 
	OPJ_BOOL owns_data;
} opj_vec_t;

/*
Initialize vector
*/
OPJ_BOOL opj_vec_init(opj_vec_t *vec);

/*
Add a value to the end of the vector
*/
OPJ_BOOL opj_vec_push_back(opj_vec_t *vec, void* value);

/*
Set a value at specified index. If index is greater then the size of the vector,
all intervening indices will be initialized to NULL
*/
OPJ_BOOL opj_vec_set(opj_vec_t *vec, OPJ_INT32 index, void* value);

/*
Get value at specified index
*/
void* opj_vec_get(opj_vec_t *vec, OPJ_INT32 index);

/*
Get value at end of vector
*/
void* opj_vec_back(opj_vec_t *vec);

/*
Clean up vector resources. Does NOT free vector itself
*/
void opj_vec_cleanup(opj_vec_t *vec);

#endif