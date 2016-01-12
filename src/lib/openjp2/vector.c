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

#include "opj_includes.h"

const OPJ_INT32 VECTOR_INITIAL_CAPACITY = 4;


static OPJ_BOOL opj_vec_double_capacity_if_full(opj_vec_t *vec) {
	if (!vec)
		return OPJ_FALSE;
	if (vec->size == vec->capacity-1) {
		void** new_data = NULL;
		OPJ_INT32 new_capacity = (OPJ_INT32)(1.5*vec->capacity);
		new_data = opj_realloc(vec->data, sizeof(void*) * (OPJ_SIZE_T)new_capacity);
		if (new_data) {
			vec->capacity = new_capacity;
			vec->data = new_data;
			return OPJ_TRUE;
		}
		else {
			return  OPJ_FALSE;
		}
	}
	return OPJ_TRUE;
}

/*----------------------------------------------------------------------------------*/

OPJ_BOOL opj_vec_init(opj_vec_t *vec, OPJ_BOOL owns_data) {
	return opj_vec_init_with_capacity(vec,VECTOR_INITIAL_CAPACITY, owns_data);
}

OPJ_BOOL opj_vec_init_with_capacity(opj_vec_t *vec, OPJ_INT32 capacity, OPJ_BOOL owns_data) {
	if (!vec)
		return OPJ_FALSE;
	if (vec->data)
		return OPJ_TRUE;

	vec->size = 0;
	vec->capacity = capacity;
	vec->data = opj_malloc(sizeof(void*) * (OPJ_SIZE_T)vec->capacity);
	vec->owns_data = owns_data;
	return vec->data ? OPJ_TRUE : OPJ_FALSE;
}

OPJ_BOOL opj_vec_push_back(opj_vec_t *vec, void* value) {
	if (!opj_vec_double_capacity_if_full(vec))
		return OPJ_FALSE;
	vec->data[vec->size++] = value;
	return OPJ_TRUE;
}

void* opj_vec_get(opj_vec_t *vec, OPJ_INT32 index) {
	if (!vec->data || !vec->size)
		return NULL;
	assert(index < vec->size && index >= 0);
	if (index >= vec->size || index < 0) {
		return NULL;
	}
	return vec->data[index];
}

void* opj_vec_back(opj_vec_t *vec) {
	if (!vec || !vec->data || !vec->size)
		return NULL;
	return opj_vec_get(vec, vec->size - 1);
}

OPJ_BOOL opj_vec_set(opj_vec_t *vec, OPJ_INT32 index, void* value) {
	if (!vec)
		return OPJ_FALSE;
	while (index >= vec->size) {
		if (!opj_vec_push_back(vec, NULL))
			return OPJ_FALSE;
	}
	vec->data[index] = value;
	return OPJ_TRUE;
}

void opj_vec_cleanup(opj_vec_t *vec) {
	OPJ_INT32 i;
	if (!vec || !vec->data)
		return;

	if (vec->owns_data) {
		for (i = 0; i < vec->size; ++i) {
			if (vec->data[i])
				opj_free(vec->data[i]);
		}
	}
	opj_free(vec->data);
	vec->data = NULL;
	vec->size = 0;
}

void opj_vec_destroy(opj_vec_t *vec) {
	if (!vec)
		return;
	opj_vec_cleanup(vec);
	opj_free(vec);
}