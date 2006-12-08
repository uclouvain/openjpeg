/*
 * Copyright (c) 2001-2002, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2002-2003,  Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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

#include "tcd.h"

#ifndef __DWT_H
#define __DWT_H

/*
 * Apply a reversible DWT transform to a component of an image  
 * tilec : tile component information (present tile)
 */
/* void dwt_encode(int* a, int w, int h, int l); */
void dwt_encode(tcd_tilecomp_t * tilec);
/*
 * Apply a reversible inverse DWT transform to a component of an image  
 * tilec : tile component information (present tile)
 */
void dwt_decode(tcd_tilecomp_t * tilec, int stop);

/*
 * Get the gain of a subband for the reversible DWT
 * orient: number that identifies the subband (0->LL, 1->HL, 2->LH, 3->HH)
 */
int dwt_getgain(int orient);

/*
 * Get the norm of a wavelet function of a subband at a specified level for the reversible DWT
 * level: level of the wavelet function
 * orient: band of the wavelet function
 */
double dwt_getnorm(int level, int orient);

/*
 * Apply an irreversible DWT transform to a component of an image  
 */
void dwt_encode_real(tcd_tilecomp_t * tilec);

/*
 * Apply an irreversible inverse DWT transform to a component of an image  
 */
void dwt_decode_real(tcd_tilecomp_t * tilec, int stop);
/*
 * Get the gain of a subband for the irreversible DWT
 * orient: number that identifies the subband (0->LL, 1->HL, 2->LH, 3->HH)
 */
int dwt_getgain_real(int orient);

/*
 * Get the norm of a wavelet function of a subband at a specified level for the irreversible DWT
 * level: level of the wavelet function
 * orient: band of the wavelet function
 */
double dwt_getnorm_real(int level, int orient);

#endif
