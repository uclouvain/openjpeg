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

#ifndef __T2_H
#define __T2_H

#include "tcd.h"
#include "j2k.h"

/*
 * Encode the packets of a tile to a destination buffer
 *
 * img        : the source image
 * cp         : the image coding parameters
 * tileno     : number of the tile encoded
 * tile       : the tile for which to write the packets
 * maxlayers  : maximum number of layers
 * dest       : the destination buffer
 * len        : the length of the destination buffer
 * info_IM    : structure to create an index file
 */
int t2_encode_packets(j2k_image_t * img, j2k_cp_t * cp, int tileno, tcd_tile_t * tile, int maxlayers, unsigned char *dest, int len, info_image * info_IM);

/*
 * Decode the packets of a tile from a source buffer
 *
 * src: the source buffer
 * len: length of the source buffer
 * img: destination image
 * cp: image coding parameters
 * tileno: number that identifies the tile for which to decode the packets
 * tile: tile for which to decode the packets
 */
int t2_decode_packets(unsigned char *src, int len, j2k_image_t * img, j2k_cp_t * cp, int tileno, tcd_tile_t * tile);

#endif
