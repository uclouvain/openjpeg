/*
* Copyright (c) 2016, Aaron Boxer
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/

#ifndef __UTIL_H
#define __UTIL_H

typedef struct opj_pt {
	OPJ_INT32 x;
	OPJ_INT32 y;

} opj_pt_t;

typedef struct opj_rect {

	OPJ_INT32 x0;
	OPJ_INT32 y0;
	OPJ_INT32 x1;
	OPJ_INT32 y1;

} opj_rect_t;

void opj_rect_init(opj_rect_t* r, OPJ_INT32 x0, OPJ_INT32 y0, OPJ_INT32 x1, OPJ_INT32 y1);

/* valid if x0 <= x1 && y0 <= y1. Can included degenerate rectangles: line and point*/
OPJ_BOOL opj_rect_is_valid(opj_rect_t* rect);

OPJ_BOOL opj_rect_is_non_degenerate(opj_rect_t* rect);

OPJ_BOOL opj_rect_is_valid(opj_rect_t* rect);

OPJ_BOOL opj_rect_are_equal(opj_rect_t* r1, opj_rect_t* r2);

OPJ_BOOL opj_rect_get_overlap(opj_rect_t* r1, opj_rect_t* r2, opj_rect_t* result);

void opj_rect_zoom(opj_rect_t* r, OPJ_FLOAT32 factor);

void opj_rect_grow(opj_rect_t* r, OPJ_INT32 boundary);

void opj_rect_grow2(opj_rect_t* r, OPJ_INT32 boundaryx, OPJ_INT32 boundaryy);

void opj_rect_subsample(opj_rect_t* r, OPJ_UINT32 dx, OPJ_UINT32 dy);

void opj_rect_pan(opj_rect_t* r, opj_pt_t* shift);

void opj_rect_print(opj_rect_t* r);

typedef struct opj_buf {
	OPJ_BYTE *buf;		/* internal array*/
	OPJ_OFF_T offset;	/* current offset into array */
	OPJ_SIZE_T len;		/* length of array */
	OPJ_BOOL owns_data;	/* OPJ_TRUE if buffer manages the buf array */
} opj_buf_t;




#endif