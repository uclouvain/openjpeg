/* Copyright (c) 2003-2004, François-Olivier Devaux
* Copyright (c) 2003-2004,  Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
* All rights reserved.
*
* All rights reserved. 
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

#include <openjpeg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mj2.h"
#include "mj2_convert.h"

int ceildiv(int a, int b)
{
  return (a + b - 1) / b;
}

int main(int argc, char **argv)
{
  FILE *f=NULL;
  char *src=NULL, *src_name=NULL;
  char *dest=NULL, S1, S2, S3;
  int len;
  j2k_cp_t cp;
  mj2_movie_t mj2_movie;


  if (argc < 3) {
    fprintf(stderr,
	    "usage: %s j2k-file image-file -reduce n (<- optional)\n",
	    argv[0]);
    return 1;
  }

  f = fopen(argv[1], "rb");
  if (!f) {
    fprintf(stderr, "failed to open %s for reading\n", argv[1]);
    return 1;
  }

  dest = argv[2];

  cp.reduce_on = 0;
  cp.reduce_value = 0;

  /* OPTION REDUCE IS ACTIVE */
  if (argc == 5) {
    if (strcmp(argv[3], "-reduce")) {
      fprintf(stderr,
	      "usage: options " "-reduce n"
	      " where n is the factor of reduction [%s]\n", argv[3]);
      return 1;
    }
    cp.reduce_on = 1;
    sscanf(argv[4], "%d", &cp.reduce_value);
  }

  while (*dest) {
    dest++;
  }
  dest--;
  S3 = *dest;
  dest--;
  S2 = *dest;
  dest--;
  S1 = *dest;

  if (!((S1 == 'y' && S2 == 'u' && S3 == 'v')
	|| (S1 == 'Y' && S2 == 'U' && S3 == 'V'))) {
    fprintf(stderr,
	    "!! Unrecognized format for outfile : %c%c%c [accept only *.yuv] !!\n",
	    S1, S2, S3);
    fprintf(stderr,
	    "usage: j2k-file image-file -reduce n (<- optional)\n\n");

    return 1;
  }

  fseek(f, 0, SEEK_END);
  len = ftell(f);
  fseek(f, 0, SEEK_SET);
  src = (char *) malloc(len);
  fread(src, 1, len, f);
  fclose(f);

  src_name = argv[1];
  while (*src_name) {
    src_name++;
  }
  src_name--;
  S3 = *src_name;
  src_name--;
  S2 = *src_name;
  src_name--;
  S1 = *src_name;

  /* MJ2 format */
  if ((S1 == 'm' && S2 == 'j' && S3 == '2')
      || (S1 == 'M' && S2 == 'J' && S3 == '2')) {
    mj2_movie.num_stk = 0;
    mj2_movie.num_htk = 0;
    mj2_movie.num_vtk = 0;
    mj2_movie.mj2file = argv[1];
    if (mj2_decode(src, len, &mj2_movie, &cp, argv[2])) {
      fprintf(stderr, "mj2_to_frames: failed to decode image!\n");
      return 1;
    }
    mj2_memory_free(&mj2_movie);
  } else {
    fprintf(stderr,
	    "mj2_to_frames : Unknown format image *.%c%c%c [only *.mj2]!! \n",
	    S1, S2, S3);
    fprintf(stderr,
	    "usage: j2k-file image-file -reduce n (<- optional)\n\n");

    return 1;
  }

  free(src);

  return 0;
}
