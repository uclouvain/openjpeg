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

#ifndef __TILE_BUF_H
#define __TILE_BUF_H


typedef struct opj_tile_buf_band {
	opj_rect_t dim;			/* dimensions of buffer, relative to tile origin */
	opj_rect_t data_dim;	/* dimensions of data stored inside buffer, relative to tile origin */
} opj_tile_buf_band_t;

typedef struct opj_tile_buf_resolution {
	opj_tile_buf_band_t band[3];
	OPJ_UINT32 num_bands;
}opj_tile_buf_resolution_t;

typedef struct opj_tile_buf_component {
	opj_vec_t* resolutions;

	OPJ_INT32 *data;	/* internal array*/
	OPJ_UINT32 data_size_needed;   /* we may either need to allocate this amount of data, or re-use image data and ignore this value */
	OPJ_UINT32 data_size;         /* size of the data of the component */
	OPJ_BOOL owns_data;	/* OPJ_TRUE if tile buffer manages its data array */

	opj_rect_t dim;		/* relative to tile origin */
	opj_rect_t tile_dim;

} opj_tile_buf_component_t;


OPJ_BOOL opj_tile_buf_alloc_component_data(opj_tile_buf_component_t* buf);

void opj_tile_buf_destroy_component(opj_tile_buf_component_t* comp);

/* check if rect overlaps with resolutions in region component */
OPJ_BOOL opj_tile_buf_hit_test(opj_tile_buf_component_t* comp, opj_rect_t* rect);

/* convenience method */
opj_tile_buf_resolution_t* opj_tile_buf_get_component_resolution(opj_tile_buf_component_t* comp,
																OPJ_INT32 resno);

#endif