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

#ifndef __REGION_MGR_H
#define __REGION_MGR_H

#include "openjpeg.h"

typedef struct opj_rgn_resolution {
	opj_rect_t band[3];
	OPJ_UINT32 num_bands;
}opj_rgn_resolution_t;

typedef struct opj_rgn_component {
	opj_vec_t* resolutions;
} opj_rgn_component_t;

typedef struct opj_rgn_mgr {
	opj_vec_t* comps;
} opj_rgn_mgr_t;

/* destroy region manager */
void opj_rgn_mgr_destroy(opj_rgn_mgr_t* mgr);

void opj_rgn_mgr_destroy_component(opj_rgn_component_t* comp);

/* check if rect overlaps with resolutions in region component */
OPJ_BOOL opj_rgn_mgr_hit_test(opj_rgn_component_t* comp, opj_rect_t* rect);

/* convenience method */
opj_rgn_component_t* opj_rgn_mgr_get_region_component(opj_rgn_mgr_t* mgr, OPJ_INT32 compno);

opj_rgn_resolution_t* opj_rgn_get_region_resolution(opj_rgn_mgr_t* mgr, 
													OPJ_INT32 compno,
													OPJ_INT32 resno);

#endif