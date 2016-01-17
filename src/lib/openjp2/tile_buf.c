/*
* Copyright (c) 2016, Aaron Boxer
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/


#include "opj_includes.h"

/*
Create region manager.

Note: because this method uses a tcd struct, and we can't forward declare the struct
in region_mgr.h header file, this method's declaration can be found in the tcd.h 
header file.

*/
OPJ_BOOL opj_tile_buf_create_component(opj_tcd_tilecomp_t* tilec,
									OPJ_BOOL irreversible,
									OPJ_UINT32 cblkw,
									OPJ_UINT32 cblkh,
									opj_image_t* output_image) {
	OPJ_INT32 resno = 0;
	opj_pt_t tile_offset;
	opj_rect_t	component_output_rect;
	opj_tile_buf_component_t* comp = NULL;

	if (!tilec)
		return OPJ_FALSE;

	/* create region component struct*/
	comp =	(opj_tile_buf_component_t*)opj_calloc(1, sizeof(opj_tile_buf_component_t));
	if (!comp) {
		return OPJ_FALSE;
	}

	opj_rect_init(&comp->tile_dim,
		tilec->x0,
		tilec->y0,
		tilec->x1,
		tilec->y1);

	if (output_image) {
		opj_rect_init(&comp->dim,
			output_image->x0,
			output_image->y0,
			output_image->x1,
			output_image->y1);

		opj_rect_get_overlap(&comp->tile_dim, &comp->dim, &comp->dim);

	}
	else {
		comp->dim = comp->tile_dim;
	}

	/* shift to origin of tile */
	tile_offset.x = -tilec->x0;
	tile_offset.y = -tilec->y0;
	opj_rect_pan(&comp->dim, &tile_offset);
	opj_rect_pan(&comp->tile_dim, &tile_offset);

	/* for encode, we don't need to allocate resolutions */
	if (!output_image) {
		opj_tile_buf_destroy_component(tilec->buf);
		tilec->buf = comp;
		return OPJ_TRUE;
	}


	component_output_rect = comp->dim;


	/* create resolutions vector */
	comp->resolutions = (opj_vec_t*)opj_calloc(1, sizeof(opj_vec_t));
	if (!comp->resolutions) {
		opj_tile_buf_destroy_component(comp);
		return OPJ_FALSE;

	}

	if (!opj_vec_init_with_capacity(comp->resolutions, 3 * tilec->numresolutions - 2, OPJ_TRUE)) {
		opj_tile_buf_destroy_component(comp);
		return OPJ_FALSE;
	}

	/* printf("\nRegion manager creation: component %d\n\n", compno); */


	/* fill resolutions vector */
	for (resno = (OPJ_INT32)(tilec->numresolutions-1); resno >= 0; --resno) {
		OPJ_UINT32 bandno = 0;
		opj_tcd_resolution_t*  resolution = tilec->resolutions + resno;
		opj_tcd_resolution_t* pres = resolution - 1;
		opj_tile_buf_resolution_t* res = (opj_tile_buf_resolution_t*)opj_calloc(1, sizeof(opj_tile_buf_resolution_t));
		if (!res) {
			opj_tile_buf_destroy_component(comp);
			return OPJ_FALSE;
		}

		/* shrink by 1/2 and add boundary, except for lowest resolution */
		if (resno > 0) {
			opj_rect_zoom(&component_output_rect, 0.5f);
			opj_rect_grow(&component_output_rect, irreversible ? 4 : 2);
		}
		for (bandno = 0; bandno < resolution->numbands; ++bandno) {
			opj_tcd_band_t* band = NULL;
			opj_pt_t offset;
				
			band = resolution->bands + bandno;

			res->band[bandno].dim = component_output_rect;

			/* start with zero offset (relative to tile origin) */
			offset.x = 0;
			offset.y = 0;

			/* add offset from 0th band (i.e. next lower resolution) to 2nd and 3rd band*/
			if (band->bandno & 1) {
				offset.x += pres->x1 - pres->x0;
			}
			if (band->bandno & 2) {
				offset.y += pres->y1 - pres->y0;
			}
				
			opj_rect_pan(&(res->band + bandno)->dim, &offset);

			/* add code block padding around region */
			(res->band + bandno)->data_dim = (res->band + bandno)->dim;
			opj_rect_grow2(&(res->band + bandno)->data_dim, cblkw, cblkh);
				
			/* opj_rect_print(res->band + bandno); */
		}
		res->num_bands = resolution->numbands;
		opj_vec_push_back(comp->resolutions, res);
	}
	
	opj_tile_buf_destroy_component(tilec->buf);
	tilec->buf = comp;

	return OPJ_TRUE;
}

OPJ_BOOL opj_tile_buf_is_decode_region(opj_tile_buf_component_t* buf) {
	if (!buf)
		return OPJ_FALSE;
	return !opj_rect_are_equal(&buf->dim, &buf->tile_dim);
}

OPJ_BOOL opj_tile_buf_alloc_component_data_encode(opj_tile_buf_component_t* buf)
{
	if (!buf)
		return OPJ_FALSE;

	if ((buf->data == 00) || ((buf->data_size_needed > buf->data_size) && (buf->owns_data == OPJ_FALSE))) {
		buf->data = (OPJ_INT32 *)opj_aligned_malloc(buf->data_size_needed);
		if (!buf->data) {
			return OPJ_FALSE;
		}
		/*fprintf(stderr, "tAllocate data of tilec (int): %d x OPJ_UINT32n",l_data_size);*/
		buf->data_size = buf->data_size_needed;
		buf->owns_data = OPJ_TRUE;
	}
	else if (buf->data_size_needed > buf->data_size) {
		/* We don't need to keep old data */
		opj_aligned_free(buf->data);
		buf->data = (OPJ_INT32 *)opj_aligned_malloc(buf->data_size_needed);
		if (!buf->data) {
			buf->data_size = 0;
			buf->data_size_needed = 0;
			buf->owns_data = OPJ_FALSE;
			return OPJ_FALSE;
		}
		/*fprintf(stderr, "tReallocate data of tilec (int): from %d to %d x OPJ_UINT32n", data_size, l_data_size);*/
		buf->data_size = buf->data_size_needed;
		buf->owns_data = OPJ_TRUE;
	}
	return OPJ_TRUE;
}


OPJ_BOOL opj_tile_buf_alloc_component_data_decode(opj_tile_buf_component_t* buf)
{
	if (!buf)
		return OPJ_FALSE;

	if (!buf->data ){
		OPJ_INT32 area = opj_rect_get_area(&buf->tile_dim);
		if (!area)
			return OPJ_FALSE;
		buf->data = (OPJ_INT32 *)opj_aligned_malloc( area * sizeof(OPJ_INT32));
		if (!buf->data) {
			return OPJ_FALSE;
		}
		/*fprintf(stderr, "tAllocate data of tilec (int): %d x OPJ_UINT32n",l_data_size);*/
		buf->data_size = area * sizeof(OPJ_INT32);
		buf->data_size_needed = buf->data_size;
		buf->owns_data = OPJ_TRUE;
	}
	
	return OPJ_TRUE;
}


void opj_tile_buf_destroy_component(opj_tile_buf_component_t* comp) {
	if (!comp)
		return;
	opj_vec_destroy(comp->resolutions);
	if (comp->data && comp->owns_data)
		opj_aligned_free(comp->data);
	comp->data = NULL;
	comp->data_size = 0;
	comp->data_size_needed = 0;
	opj_free(comp);
}

OPJ_BOOL opj_tile_buf_hit_test(opj_tile_buf_component_t* comp, opj_rect_t* rect) {
	OPJ_INT32 i;
	opj_tile_buf_resolution_t* res;

	if (!comp || !rect || !comp->resolutions)
		return OPJ_FALSE;
	for (i = 0; i < comp->resolutions->size; ++i) {
		opj_rect_t dummy;
		OPJ_UINT32 j;
		res = (opj_tile_buf_resolution_t*)opj_vec_get(comp->resolutions,i);
		for (j = 0; j < res->num_bands; ++j) {
			if (opj_rect_get_overlap(&(res->band + j)->dim, rect, &dummy))
				return OPJ_TRUE;
		}
	}
	return OPJ_FALSE;
}

opj_tile_buf_resolution_t* opj_tile_buf_get_component_resolution(opj_tile_buf_component_t* comp,
																OPJ_INT32 resno) {
	if (comp) {
		/* note: resolutions are stored in reverse order*/
		return (opj_tile_buf_resolution_t*)opj_vec_get(comp->resolutions, 
													comp->resolutions->size - 1 - resno);
	}
	return NULL;

}


