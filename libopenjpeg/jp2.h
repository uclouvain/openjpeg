/*
 * Copyright (c) 2003, Yannick Verschueren
 * Copyright (c) 2003,  Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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
#ifndef __JP2_H
#define __JP2_H

#include "j2k.h"

typedef struct {
  int depth;
  int sgnd;
  int bpcc;
} jp2_comps_t;

typedef struct {
  unsigned int w;
  unsigned int h;
  unsigned int numcomps;
  unsigned int bpc;
  unsigned int C;
  unsigned int UnkC;
  unsigned int IPR;
  unsigned int meth;
  unsigned int approx;
  unsigned int enumcs;
  unsigned int precedence;
  unsigned int brand;
  unsigned int minversion;
  unsigned int numcl;
  unsigned int *cl;
  jp2_comps_t *comps;
  j2k_image_t *image;
} jp2_struct_t;

typedef struct {
  int length;
  int type;
  int init_pos;
} jp2_box_t;

int jp2_init_stdjp2(jp2_struct_t * jp2_struct, j2k_image_t * img);

int jp2_write_jp2c(j2k_image_t * img, j2k_cp_t * cp, char *jp2_buffer,
		   char *index);

void jp2_write_jp2h(jp2_struct_t * jp2_struct);

int jp2_read_jp2h(jp2_struct_t * jp2_struct);

int jp2_encode(jp2_struct_t * jp2_struct, j2k_cp_t * cp, char *output,
	       char *index);

int jp2_decode(unsigned char *src, int len, jp2_struct_t * jp2_struct,
	       j2k_cp_t * cp);

#endif
