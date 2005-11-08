/*
 * Copyright (c) 2001-2002, David Janssens
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

#ifndef __MCT_H
#define __MCT_H

/*
 * Apply a reversible multi-component transform to an image
 * R: samples for red component
 * G: samples for green component
 * B: samples blue component
 * n: number of samples for each component
 */
void mct_encode(int *R, int *G, int *B, int n);
/*
 * Apply a reversible multi-component inverse transform to an image
 * Y: samples for luminance component
 * U: samples for red chrominance component
 * V: samples for blue chrominance component
 * n: number of samples for each component
 */
void mct_decode(int *V, int *U, int *Y, int n);
/*
 * Get norm of the basis function used for the reversible multi-component transform
 * compno: number of the component (0->Y, 1->U, 2->V)
 */
double mct_getnorm(int compno);

/*
 * Apply an irreversible multi-component transform to an image
 * R: samples for red component
 * G: samples for green component
 * B: samples blue component
 * n: number of samples for each component
 */
void mct_encode_real(int *c0, int *c1, int *c2, int n);
/*
 * Apply an irreversible multi-component inverse transform to an image
 * Y: samples for luminance component
 * U: samples for red chrominance component
 * V: samples for blue chrominance component
 * n: number of samples for each component
 */
void mct_decode_real(int *c0, int *c1, int *c2, int n);
/*
 * Get norm of the basis function used for the irreversible multi-component transform
 * compno: number of the component (0->Y, 1->U, 2->V)
 */
double mct_getnorm_real(int compno);

#endif
