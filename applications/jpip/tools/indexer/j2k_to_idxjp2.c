/*
 * $Id$
 *
 * Copyright (c) 2002-2011, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2011, Professor Benoit Macq
 * Copyright (c) 2010-2011, Kaori Hagihara
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

/*! \file
 *  \brief j2k_to_idxjp2 is a program to make jp2 file with index box from j2k file
 *
 *  \section impinst Implementing instructions
 *  This program takes two arguments. \n
 *   -# Input  J2K image file\n
 *   -# Output JP2 file name\n
 *   % ./j2k_to_idxjp2 image.j2k image.jp2\n
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openjpeg.h"
#include "j2k_to_idxjp2.h"


/* 
 * Read a binary file
 *
 * @param[in]  filename  file name
 * @param[out] lenoffile pointer to feed file length
 * @return               byte code
 */
unsigned char * read_binaryfile( char filename[], int *lenoffile);

int main(int argc, char **argv)
{ 
  opj_image_t *image = NULL;
  opj_codestream_info_t cstr_info;  /* Codestream information structure */
  unsigned char *j2kstream;
  int j2klen;

  if (argc < 3){
    fprintf(stderr,"\nERROR in entry : j2k_to_idxjp2 J2K-file JP2-file\n");
    return 1;
  }

  j2kstream = read_binaryfile( argv[1], &j2klen);

  image = decode_j2k( j2kstream, j2klen, &cstr_info);
  if( !image){
    free(j2kstream);
    return -1;
  }
  
  fwrite_idxjp2( argv[2], j2kstream, j2klen, image, cstr_info);
  
  free(j2kstream);
  
  /* free image data structure */
  opj_image_destroy(image);
  
  return 0;
}

unsigned char * read_binaryfile( char filename[], int *lenoffile)
{
  FILE *fp;
  unsigned char *bytecode;

  fp = fopen( filename, "rb");
  if (!fp) {
    fprintf(stderr, "Failed to open %s for reading !!\n", filename);
    exit (-1);
  }

  /* length of the codestream */
  fseek( fp, 0, SEEK_END);
  *lenoffile = ftell(fp);
  fseek( fp, 0, SEEK_SET);
  
  bytecode = (unsigned char*)malloc(*lenoffile);
  fread( bytecode, *lenoffile, 1, fp);
  fclose(fp);

  return bytecode;
}
