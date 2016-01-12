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

OPJ_BOOL opj_rect_get_overlap(opj_rect_t* r1, opj_rect_t* r2, opj_rect_t* result);

void opj_rect_zoom(opj_rect_t* r, OPJ_FLOAT32 factor);

void opj_rect_grow(opj_rect_t* r, OPJ_INT32 boundary);

void opj_rect_pan(opj_rect_t* r, opj_pt_t* shift);

void opj_rect_print(opj_rect_t* r);

#endif