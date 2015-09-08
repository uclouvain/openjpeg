/*
 * The copyright in this software is being made available under the 2-clauses 
 * BSD License, included below. This software may be subject to other third 
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
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

opj_image_t* opj_image_create0(opj_manager_t manager) {
	opj_image_t *image = NULL;

	/* preconditions */
	assert(manager != NULL);

	image = (opj_image_t*)opj_manager_calloc(manager, 1, sizeof(opj_image_t));
	if (image == NULL) {
		opj_event_msg(&(manager->event_mgr), EVT_ERROR, "Unable to allocate memory for image.");
		return NULL;
	}
	return image;
}

opj_image_t* OPJ_CALLCONV opj_image_create(OPJ_UINT32 numcmpts, opj_image_cmptparm_t *cmptparms, OPJ_COLOR_SPACE clrspc)
{
	opj_manager_t l_manager = opj_manager_get_global_manager();

	return opj_manager_image_create(l_manager, numcmpts, cmptparms, clrspc);
}

OPJ_API opj_image_t* OPJ_CALLCONV opj_manager_image_create(opj_manager_t manager, OPJ_UINT32 numcmpts, opj_image_cmptparm_t *cmptparms, OPJ_COLOR_SPACE clrspc)
{
	OPJ_UINT32 compno;
	opj_image_t *image = NULL;

	if (manager == NULL) {
		return NULL;
	}

	image = (opj_image_t*) opj_manager_calloc(manager, 1, sizeof(opj_image_t));
	if (image == NULL) {
		opj_event_msg(&(manager->event_mgr), EVT_ERROR, "Unable to allocate memory for image.");
		return NULL;
	}
	image->color_space = clrspc;
	image->numcomps = numcmpts;
	/* allocate memory for the per-component information */
	image->comps = (opj_image_comp_t*)opj_manager_calloc(manager, 1,image->numcomps * sizeof(opj_image_comp_t));
	if(!image->comps) {
		opj_event_msg(&(manager->event_mgr), EVT_ERROR, "Unable to allocate memory for image.");
		opj_manager_image_destroy(manager, image);
		return NULL;
	}
	/* create the individual image components */
	for(compno = 0; compno < numcmpts; compno++) {
		opj_image_comp_t *comp = &image->comps[compno];
		comp->dx = cmptparms[compno].dx;
		comp->dy = cmptparms[compno].dy;
		comp->w = cmptparms[compno].w;
		comp->h = cmptparms[compno].h;
		comp->x0 = cmptparms[compno].x0;
		comp->y0 = cmptparms[compno].y0;
		comp->prec = cmptparms[compno].prec;
		comp->bpp = cmptparms[compno].bpp;
		comp->sgnd = cmptparms[compno].sgnd;
		comp->data = (OPJ_INT32*) opj_manager_calloc(manager, comp->w * comp->h, sizeof(OPJ_INT32));
		if(!comp->data) {
			opj_event_msg(&(manager->event_mgr), EVT_ERROR, "Unable to allocate memory for image.");
			opj_manager_image_destroy(manager, image);
			return NULL;
		}
	}

	return image;
}

void OPJ_CALLCONV opj_image_destroy(opj_image_t *image)
{
	opj_manager_t l_manager = opj_manager_get_global_manager();

	opj_manager_image_destroy(l_manager, image);
}

OPJ_API void OPJ_CALLCONV opj_manager_image_destroy(opj_manager_t manager, opj_image_t *image)
{
	if (manager == NULL) {
		return;
	}

	if(image) {
		if(image->comps) {
			OPJ_UINT32 compno;

			/* image components */
			for(compno = 0; compno < image->numcomps; compno++) {
				opj_image_comp_t *image_comp = &(image->comps[compno]);
				if(image_comp->data) {
					opj_manager_free(manager, image_comp->data);
				}
			}
			opj_manager_free(manager, image->comps);
		}

		if(image->icc_profile_buf) {
			opj_manager_free(manager, image->icc_profile_buf);
		}
		opj_manager_free(manager, image);
	}
}

/**
 * Updates the components characteristics of the image from the coding parameters.
 *
 * @param p_image_header	the image header to update.
 * @param p_cp				the coding parameters from which to update the image.
 */
void opj_image_comp_header_update(opj_image_t * p_image_header, const struct opj_cp * p_cp)
{
	OPJ_UINT32 i, l_width, l_height;
	OPJ_UINT32 l_x0, l_y0, l_x1, l_y1;
	OPJ_UINT32 l_comp_x0, l_comp_y0, l_comp_x1, l_comp_y1;
	opj_image_comp_t* l_img_comp = NULL;

	l_x0 = opj_uint_max(p_cp->tx0 , p_image_header->x0);
	l_y0 = opj_uint_max(p_cp->ty0 , p_image_header->y0);
	l_x1 = p_cp->tx0 + (p_cp->tw - 1U) * p_cp->tdx; /* validity of p_cp members used here checked in opj_j2k_read_siz. Can't overflow. */
	l_y1 = p_cp->ty0 + (p_cp->th - 1U) * p_cp->tdy; /* can't overflow */
	l_x1 = opj_uint_min(opj_uint_adds(l_x1, p_cp->tdx), p_image_header->x1); /* use add saturated to prevent overflow */
	l_y1 = opj_uint_min(opj_uint_adds(l_y1, p_cp->tdy), p_image_header->y1); /* use add saturated to prevent overflow */

	l_img_comp = p_image_header->comps;
	for	(i = 0; i < p_image_header->numcomps; ++i) {
		l_comp_x0 = opj_uint_ceildiv(l_x0, l_img_comp->dx);
		l_comp_y0 = opj_uint_ceildiv(l_y0, l_img_comp->dy);
		l_comp_x1 = opj_uint_ceildiv(l_x1, l_img_comp->dx);
		l_comp_y1 = opj_uint_ceildiv(l_y1, l_img_comp->dy);
		l_width   = opj_uint_ceildivpow2(l_comp_x1 - l_comp_x0, l_img_comp->factor);
		l_height  = opj_uint_ceildivpow2(l_comp_y1 - l_comp_y0, l_img_comp->factor);
		l_img_comp->w = l_width;
		l_img_comp->h = l_height;
		l_img_comp->x0 = l_comp_x0;
		l_img_comp->y0 = l_comp_y0;
		++l_img_comp;
	}
}


/**
 * Copy only header of image and its component header (no data are copied)
 * if dest image have data, they will be freed
 *
 * @param manager      OpenJpeg memory/event manager
 * @param	p_image_src  the src image
 * @param	p_image_dest the dest image
 * @return OPJ_TRUE on success, OPJ_FALSE on failure
 * */
OPJ_BOOL opj_copy_image_header(opj_manager_t manager, const opj_image_t* p_image_src, opj_image_t* p_image_dest)
{
	OPJ_UINT32 compno;

	/* preconditions */
	assert(manager != NULL);
	assert(p_image_src != 00);
	assert(p_image_dest != 00);

	p_image_dest->x0 = p_image_src->x0;
	p_image_dest->y0 = p_image_src->y0;
	p_image_dest->x1 = p_image_src->x1;
	p_image_dest->y1 = p_image_src->y1;

	if (p_image_dest->comps){
		for(compno = 0; compno < p_image_dest->numcomps; compno++) {
			opj_image_comp_t *image_comp = &(p_image_dest->comps[compno]);
			if(image_comp->data) {
				opj_manager_free(manager, image_comp->data);
			}
		}
		opj_manager_free(manager, p_image_dest->comps);
		p_image_dest->comps = NULL;
	}

	p_image_dest->numcomps = p_image_src->numcomps;

	p_image_dest->comps = (opj_image_comp_t*) opj_manager_malloc(manager, p_image_dest->numcomps * sizeof(opj_image_comp_t));
	if (p_image_dest->comps == NULL) {
		opj_event_msg(&(manager->event_mgr), EVT_ERROR, "Unable to allocate memory for image.");
		p_image_dest->comps = NULL;
		p_image_dest->numcomps = 0;
		return OPJ_FALSE;
	}

	for (compno=0; compno < p_image_dest->numcomps; compno++){
		memcpy( &(p_image_dest->comps[compno]), &(p_image_src->comps[compno]), sizeof(opj_image_comp_t));
		p_image_dest->comps[compno].data = NULL;
	}

	p_image_dest->color_space = p_image_src->color_space;
	p_image_dest->icc_profile_len = p_image_src->icc_profile_len;

	if (p_image_dest->icc_profile_len) {
		p_image_dest->icc_profile_buf = (OPJ_BYTE*)opj_manager_malloc(manager, p_image_dest->icc_profile_len);
		if (p_image_dest->icc_profile_buf == NULL) {
			opj_event_msg(&(manager->event_mgr), EVT_ERROR, "Unable to allocate memory for image.");
			p_image_dest->icc_profile_buf = NULL;
			p_image_dest->icc_profile_len = 0;
			return OPJ_FALSE;
		}
		memcpy( p_image_dest->icc_profile_buf, p_image_src->icc_profile_buf, p_image_src->icc_profile_len);
	} else {
		p_image_dest->icc_profile_buf = NULL;
	}

	return OPJ_TRUE;
}

opj_image_t* OPJ_CALLCONV opj_image_tile_create(OPJ_UINT32 numcmpts, opj_image_cmptparm_t *cmptparms, OPJ_COLOR_SPACE clrspc)
{
	opj_manager_t l_manager = opj_manager_get_global_manager();

	return opj_manager_image_tile_create(l_manager, numcmpts, cmptparms, clrspc);
}
OPJ_API opj_image_t* OPJ_CALLCONV opj_manager_image_tile_create(opj_manager_t manager, OPJ_UINT32 numcmpts, opj_image_cmptparm_t *cmptparms, OPJ_COLOR_SPACE clrspc)
{
	OPJ_UINT32 compno;
	opj_image_t *image = 00;

	if (manager == NULL) {
		return NULL;
	}

	image = (opj_image_t*) opj_manager_calloc(manager, 1, sizeof(opj_image_t));
	if (image == NULL) {
		opj_event_msg(&(manager->event_mgr), EVT_ERROR, "Unable to allocate memory for image.");
		return NULL;
	}
		
	image->color_space = clrspc;
	image->numcomps = numcmpts;
		
	/* allocate memory for the per-component information */
	image->comps = (opj_image_comp_t*)opj_manager_calloc(manager, image->numcomps, sizeof(opj_image_comp_t));
	if (!image->comps) {
		opj_event_msg(&(manager->event_mgr), EVT_ERROR, "Unable to allocate memory for image.");
		opj_manager_image_destroy(manager, image);
		return NULL;
	}
		
	/* create the individual image components */
	for(compno = 0; compno < numcmpts; compno++) {
		opj_image_comp_t *comp = &image->comps[compno];
		comp->dx = cmptparms[compno].dx;
		comp->dy = cmptparms[compno].dy;
		comp->w = cmptparms[compno].w;
		comp->h = cmptparms[compno].h;
		comp->x0 = cmptparms[compno].x0;
		comp->y0 = cmptparms[compno].y0;
		comp->prec = cmptparms[compno].prec;
		comp->sgnd = cmptparms[compno].sgnd;
		comp->data = NULL;
	}

	return image;
}
