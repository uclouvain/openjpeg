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
  unsigned int j2k_codestream_offset;
  unsigned int j2k_codestream_len;
} jp2_struct_t;

typedef struct {
  int length;
  int type;
  int init_pos;
} jp2_box_t;

/* int jp2_init_stdjp2(jp2_struct_t * jp2_struct, j2k_image_t * img); 
 *
 * Create a standard jp2_structure
 * jp2_struct: the structure you are creating
 * img: a j2k_image_t wich will help you to create the jp2_structure
 */
int jp2_init_stdjp2(jp2_struct_t * jp2_struct);

/* int jp2_write_jp2c(int j2k_len, int *j2k_codestream_offset, char *j2k_codestream)
 *
 * Write the jp2c codestream box 
 * j2k_len: the j2k codestream length
 * j2k_codestream_offset: the function will return the j2k codestream offset
 * j2k_codestream: the j2k codestream to include in jp2 file
 */
int jp2_write_jp2c(int j2k_len, int *j2k_codestream_offset, char *j2k_codestream);

/* int jp2_write_jp2h(jp2_struct_t * jp2_struct);
 *
 * Write the jp2h header box 
 * jp2_struct: the jp2 structure you are working with
 */
void jp2_write_jp2h(jp2_struct_t * jp2_struct);

/* int jp2_read_jp2h(jp2_struct_t * jp2_struct);
 *
 * Read the jp2h header box 
 * jp2_struct: the jp2 structure you are working with
 */
int jp2_read_jp2h(jp2_struct_t * jp2_struct);

/* int jp2_wrap_j2k(jp2_struct_t * jp2_struct, char *j2k_codestream, 
	  int j2k_len, char *output)
 *
 * Wrap a J2K codestream in a JP2 file
 * jp2_struct: the jp2 structure used to create jp2 boxes
 * j2k_codestream: the j2k codestream to include in jp2 file
 * output: pointer to jp2 codestream that will be created
 */
int jp2_wrap_j2k(jp2_struct_t * jp2_struct, char *j2k_codestream, 
		  char *output);


/* int jp2_read_struct(unsigned char *src, jp2_struct_t * jp2_struct);
 *
 * Decode the structure of a JP2 file
 * src: pointer to memory where compressed data is stored
 * jp2_struct: the jp2 structure that will be created 
 * len: length of jp2 codestream
 */
int jp2_read_struct(unsigned char *src, jp2_struct_t * jp2_struct, int len);

#endif
