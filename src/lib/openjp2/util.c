/*
* Copyright (c) 2016, Aaron Boxer
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
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

void opj_rect_subsample(opj_rect_t* r, OPJ_UINT32 dx, OPJ_UINT32 dy) {
	if (!r)
		return;

	r->x0 = opj_int_ceildiv(r->x0, (OPJ_INT32)dx);
	r->y0 = opj_int_ceildiv(r->y0, (OPJ_INT32)dy);
	r->x1 = opj_int_ceildiv(r->x1, (OPJ_INT32)dx);
	r->y1 = opj_int_ceildiv(r->y1, (OPJ_INT32)dy);
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