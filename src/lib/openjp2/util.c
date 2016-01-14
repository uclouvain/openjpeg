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


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void opj_rect_print(opj_rect_t* r) {
	if (!r)
		printf("Null rect\n");
	else
		printf("Rectangle:  [%d,%d,%d,%d] \n", r->x0, r->y0, r->x1, r->y1);
}

void opj_rect_init(opj_rect_t* r, OPJ_INT32 x0, OPJ_INT32 y0, OPJ_INT32 x1, OPJ_INT32 y1) {
	if (!r)
		return;
	r->x0 = x0;
	r->y0 = y0;
	r->x1 = x1;
	r->y1 = y1;
}

OPJ_BOOL opj_rect_is_valid(opj_rect_t* rect) {
	if (!rect)
		return OPJ_FALSE;

	return rect->x0 <= rect->x1 && rect->y0 <= rect->y1;
}

OPJ_BOOL opj_rect_is_non_degenerate(opj_rect_t* rect) {
	if (!rect)
		return OPJ_FALSE;

	return rect->x0 < rect->x1 && rect->y0 < rect->y1;
}

OPJ_BOOL opj_rect_are_equal(opj_rect_t* r1, opj_rect_t* r2) {
	return r1->x0 == r2->x0 && 
			r1->y0 == r2->y0 &&
			r1->x1 == r2->x1 &&
			r1->y1 == r2->y1;
}

OPJ_BOOL opj_rect_get_overlap(opj_rect_t* r1, opj_rect_t* r2, opj_rect_t* result) {
	OPJ_BOOL rc;
	opj_rect_t temp;
	
	if (!r1 || !r2 || !result)
		return OPJ_FALSE;

	temp.x0 = MAX(r1->x0, r2->x0);
	temp.y0 = MAX(r1->y0, r2->y0);
	
	temp.x1 = MIN(r1->x1, r2->x1);
	temp.y1 = MIN(r1->y1, r2->y1);

	rc = opj_rect_is_valid(&temp);
	
	if (rc)
		*result = temp;
	return rc;
}


void opj_rect_zoom(opj_rect_t* r, OPJ_FLOAT32 factor) {
	if (!r)
		return;
	
	r->x0 = (OPJ_INT32)(r->x0*factor);
	r->y0 = (OPJ_INT32)(r->y0*factor);

	/* round up bottom right hand corner */
	r->x1 = (OPJ_INT32)(r->x1*factor + 0.5f);
	r->y1 = (OPJ_INT32)(r->y1*factor + 0.5f);

}


void opj_rect_pan(opj_rect_t* r, opj_pt_t* shift) {
	if (!r)
		return;
	
	r->x0 += shift->x;
	r->y0 += shift->y;
	r->x1 += shift->x;
	r->y1 += shift->y;
}


void opj_rect_grow(opj_rect_t* r, OPJ_INT32 boundary) {
	if (!r)
		return;

	opj_rect_grow2(r, boundary, boundary);
}

void opj_rect_grow2(opj_rect_t* r, OPJ_INT32 boundaryx, OPJ_INT32 boundaryy) {
	if (!r)
		return;

	r->x0 -= boundaryx;
	r->y0 -= boundaryy;
	r->x1 += boundaryx;
	r->y1 += boundaryy;
}