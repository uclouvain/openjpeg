/*
* Copyright (c) 2016, Aaron Boxer
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
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

OPJ_BOOL opj_tile_buf_alloc_component_data_decode(opj_tile_buf_component_t* buf);

OPJ_BOOL opj_tile_buf_alloc_component_data_encode(opj_tile_buf_component_t* buf);

OPJ_BOOL opj_tile_buf_is_decode_region(opj_tile_buf_component_t* buf);

void opj_tile_buf_destroy_component(opj_tile_buf_component_t* comp);

/* check if rect overlaps with resolutions in region component */
OPJ_BOOL opj_tile_buf_hit_test(opj_tile_buf_component_t* comp, opj_rect_t* rect);

/* convenience method */
opj_tile_buf_resolution_t* opj_tile_buf_get_component_resolution(opj_tile_buf_component_t* comp,
																OPJ_INT32 resno);

#endif