/*
 * The copyright in this software is being made available under the 2-clauses 
 * BSD License, included below. This software may be subject to other third 
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2015, Aaron Boxer
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

opj_rgn_mgr_t* opj_rgn_mgr_create(opj_tcd_tile_t * l_tile,
									OPJ_BOOL irreversible,
									opj_image_t* output_image) {
	OPJ_UINT32 compno = 0;
	OPJ_INT32 resno = 0;
	OPJ_UINT32 bandno = 0;
	opj_rect_t output_rect;
	opj_pt_t tile_offset;

	/* create manager */
	opj_rgn_mgr_t* mgr = (opj_rgn_mgr_t*)opj_calloc(1, sizeof(opj_rgn_mgr_t));
	if (!mgr)
		return NULL;

	mgr->comps = (opj_vec_t*)opj_calloc(1, sizeof(opj_vec_t));
	if (!mgr->comps) {
		opj_rgn_mgr_destroy(mgr);
		return NULL;
	}

	if (!opj_vec_init_with_capacity(mgr->comps, l_tile->numcomps, OPJ_FALSE)) {
		opj_rgn_mgr_destroy(mgr);
		return NULL;
	}

	tile_offset.x = -l_tile->x0;
	tile_offset.y = -l_tile->y0;
	opj_rect_init(&output_rect, 
					output_image->x0,
					output_image->y0,
					output_image->x1,
					output_image->y1);
	/* shift output rect to origin of tile */
	opj_rect_pan(&output_rect, &tile_offset);

	/* create one region per component   */
	for (compno = 0; compno < l_tile->numcomps; ++compno) {
		opj_rect_t component_output_rect = output_rect;
		opj_tcd_tilecomp_t* tilec = l_tile->comps + compno;

		/* create region component struct*/
		opj_rgn_component_t* comp =
			(opj_rgn_component_t*)opj_calloc(1, sizeof(opj_rgn_component_t));
		if (!comp) {
			opj_rgn_mgr_destroy(mgr);
			return OPJ_FALSE;

		}

		/* create regions vector */
		comp->regions = (opj_vec_t*)opj_calloc(1, sizeof(opj_vec_t));
		if (!comp->regions) {
			opj_rgn_mgr_destroy(mgr);
			return OPJ_FALSE;

		}

		opj_vec_push_back(mgr->comps, comp);
		if (!opj_vec_init_with_capacity(comp->regions, 3 * tilec->numresolutions - 2, OPJ_TRUE)) {
			opj_rgn_mgr_destroy(mgr);
			return OPJ_FALSE;
		}

		/* printf("\nRegion manager creation: component %d\n\n", compno); */


		/* fill regions vector */
		for (resno = tilec->numresolutions-1; resno >= 0; --resno) {
			opj_tcd_resolution_t*  resolution = tilec->resolutions + resno;
			opj_tcd_resolution_t* pres = resolution - 1;

			/* shrink by 1/2 and add boundary, except for lowest resolution */
			if (resno > 0) {
				opj_rect_zoom(&component_output_rect, 0.5f);
				opj_rect_grow(&component_output_rect, irreversible ? 4 : 2);
			}
			for (bandno = 0; bandno < resolution->numbands; ++bandno) {
				opj_tcd_band_t* band = NULL;
				opj_pt_t offset;
				opj_rect_t* region_rect  = (opj_rect_t*)opj_calloc(1, sizeof(opj_rect_t));
				if (!region_rect) {
					opj_rgn_mgr_destroy_component(comp);
					return OPJ_FALSE;
				}
				band = resolution->bands + bandno;

				*region_rect = component_output_rect;
				//opj_rect_init(region_rect,0,0, band->x1 - band->x0, band->y1-band->y0);

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
				
				opj_rect_pan(region_rect, &offset);
				/* opj_rect_print(region_rect); */
				opj_vec_push_back(comp->regions, region_rect);
			}
		}
	}
	return mgr;
}


void opj_rgn_mgr_destroy_component(opj_rgn_component_t* comp) {
	if (!comp)
		return;
	opj_vec_destroy(comp->regions);
	opj_free(comp);
}

void opj_rgn_mgr_destroy(opj_rgn_mgr_t* mgr) {
	OPJ_INT32 i;
	opj_rgn_component_t* region_comp=NULL;
	if (!mgr)
		return;
	
	/* destroy contents of mgr->region_components */
	if (mgr->comps) {
		for (i = 0; i < mgr->comps->size; ++i) {
			opj_rgn_mgr_destroy_component((opj_rgn_component_t*)opj_vec_get(mgr->comps, i));
		}
		/* destroy mgr->region_components */
		opj_vec_destroy(mgr->comps);
	}
	
	/* destroy manager */
	opj_free(mgr);
}

OPJ_BOOL opj_rgn_mgr_hit_test(opj_rgn_component_t* comp, opj_rect_t* rect) {
	OPJ_INT32 i;
	opj_rect_t* region_rect;

	if (!comp || !rect)
		return OPJ_FALSE;
	for (i = 0; i < comp->regions->size; ++i) {
		opj_rect_t dummy;
		region_rect = (opj_rect_t*)opj_vec_get(comp->regions,i);
		if (opj_rect_get_overlap(region_rect, rect, &dummy))
			return OPJ_TRUE;
	}
	return OPJ_FALSE;
}

opj_rgn_component_t* opj_rgn_mgr_get_region_component(opj_rgn_mgr_t* mgr, OPJ_INT32 index) {
	if (!mgr || !mgr->comps || !mgr->comps->size)
		return NULL;
	return (opj_rgn_component_t*)opj_vec_get(mgr->comps, index);
}


