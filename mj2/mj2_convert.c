/*
* Copyright (c) 2003-2004, François-Olivier Devaux
* Copyright (c) 2003-2004,  Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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

#include <openjpeg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include "mj2.h"

// -->> -->> -->> -->>

//  YUV IMAGE FORMAT

//-- <<-- <<-- <<-- */


/*  -----------------------	      */
/*				      */
/*				      */
/*  Count the number of frames	      */
/*  in a YUV file		      */
/*				      */
/*  -----------------------	      */

int yuv_num_frames(mj2_tk_t * tk)
{
  FILE *f;
  int numimages, frame_size;
  long end_of_f;

  f = fopen(tk->imagefile, "rb");
  if (!f) {
    fprintf(stderr, "Failed to open %s for reading !!\n", tk->imagefile);
    return 0;
  }

  frame_size = (int) (tk->w * tk->h * (1.0 + (double) 2 / (double) (tk->CbCr_subsampling_dx * tk->CbCr_subsampling_dy)));	/* Calculate frame size */

  fseek(f, 0, SEEK_END);
  end_of_f = ftell(f);		/* Calculate file size */

  if (end_of_f < frame_size) {
    fprintf(stderr,
	    "YUV does not contains any frame of %d x %d size\n", tk->w,
	    tk->h);
    return 0;
  }

  numimages = end_of_f / frame_size;	/* Calculate number of images */

  return numimages;
  fclose(f);
}

//  -----------------------
//
//
//  YUV to IMAGE
//
//  -----------------------

int yuvtoimage(mj2_tk_t * tk, j2k_image_t * img, int frame_num)
{
  FILE *f;
  int i, j;
  int offset;
  long end_of_f, position;
  FILE *Compo;



  f = fopen(tk->imagefile, "rb");
  if (!f) {
    fprintf(stderr, "Failed to open %s for reading !!\n", tk->imagefile);
    return 0;
  }

  offset =
    (int) ((double) (frame_num * tk->w * tk->h) *
	   (1.0 +
	    1.0 * (double) 2 / (double) (tk->CbCr_subsampling_dx *
					 tk->CbCr_subsampling_dy)));
  fseek(f, 0, SEEK_END);
  end_of_f = ftell(f);
  fseek(f, sizeof(unsigned char) * offset, SEEK_SET);
  position = ftell(f);
  if (position >= end_of_f) {
    fprintf(stderr, "Cannot reach frame number %d in %s file !!\n",
	    frame_num, tk->imagefile);
    return 0;
  }

  img->x0 = tk->Dim[0];
  img->y0 = tk->Dim[1];
  img->x1 =
    !tk->Dim[0] ? (tk->w - 1) * tk->subsampling_dx + 1 : tk->Dim[0] +
    (tk->w - 1) * tk->subsampling_dx + 1;
  img->y1 =
    !tk->Dim[1] ? (tk->h - 1) * tk->subsampling_dy + 1 : tk->Dim[1] +
    (tk->h - 1) * tk->subsampling_dy + 1;
  img->numcomps = 3;
  img->color_space = 3;
  img->comps = (j2k_comp_t *) malloc(img->numcomps * sizeof(j2k_comp_t));

  for (i = 0; i < img->numcomps; i++) {
    img->comps[i].data = (int *) malloc(sizeof(int) * tk->w * tk->h);
    img->comps[i].prec = 8;
    img->comps[i].bpp = 8;
    img->comps[i].sgnd = 0;
    if (i == 0) {
      img->comps[i].dx = tk->subsampling_dx;
      img->comps[i].dy = tk->subsampling_dy;
    } else {
      img->comps[i].dx = tk->subsampling_dx * tk->CbCr_subsampling_dx;
      img->comps[i].dy = tk->subsampling_dy * tk->CbCr_subsampling_dy;
    }
  }

  Compo = fopen("Compo0", "wb");
  if (!Compo) {
    fprintf(stderr, "Failed to open Compo0 for writing !\n");
  }

  for (i = 0; i < (tk->w * tk->h / (img->comps[0].dx * img->comps[0].dy))
       && !feof(f); i++) {
    unsigned char y;
    j = fread(&y, 1, 1, f);
    fwrite(&y, 1, 1, Compo);
  }

  fclose(Compo);

  Compo = fopen("Compo1", "wb");
  if (!Compo) {
    fprintf(stderr, "Failed to open Compo1 for writing !\n");
  }


  for (i = 0; i < (tk->w * tk->h / (img->comps[1].dx * img->comps[1].dy))
       && !feof(f); i++) {
    unsigned char cb;
    j = fread(&cb, sizeof(unsigned char), 1, f);
    fwrite(&cb, 1, 1, Compo);
  }

  fclose(Compo);

  Compo = fopen("Compo2", "wb");
  if (!Compo) {
    fprintf(stderr, "Failed to open Compo2 for writing !\n");
  }


  for (i = 0; i < (tk->w * tk->h / (img->comps[2].dx * img->comps[2].dy))
       && !feof(f); i++) {
    unsigned char cr;
    j = fread(&cr, sizeof(unsigned char), 1, f);
    fwrite(&cr, 1, 1, Compo);
  }

  fclose(Compo);

  return 1;
}


//  -----------------------
//
//
//  IMAGE to YUV
//
//  -----------------------


int imagetoyuv(j2k_image_t * img, j2k_cp_t * cp, char *outfile)
{
  FILE *f;
  int i;

  if (img->numcomps != 3 || img->comps[0].dx != img->comps[1].dx / 2
      || img->comps[1].dx != img->comps[2].dx) {
    fprintf(stderr, "Error with image components sizes\n");
    return 1;
  }

  f = fopen(outfile, "a+b");
  if (!f) {
    fprintf(stderr, "failed to open %s for writing\n", outfile);
    return 1;
  }


  for (i = 0; i < (img->comps[0].w * img->comps[0].h); i++) {
    unsigned char y;
    y = img->comps[0].data[i];
    fwrite(&y, 1, 1, f);
  }


  if (img->numcomps > 2) {
    for (i = 0; i < (img->comps[1].w * img->comps[1].h); i++) {
      unsigned char cb;
      cb = img->comps[1].data[i];
      fwrite(&cb, 1, 1, f);
    }


    for (i = 0; i < (img->comps[2].w * img->comps[2].h); i++) {
      unsigned char cr;
      cr = img->comps[2].data[i];
      fwrite(&cr, 1, 1, f);
    }
  } else if (img->numcomps == 1) {
    for (i = 0; i < (img->comps[0].w * img->comps[0].h * 0.25); i++) {
      unsigned char cb = 0;
      fwrite(&cb, 1, 1, f);
    }


    for (i = 0; i < (img->comps[0].w * img->comps[0].h * 0.25); i++) {
      unsigned char cr = 0;
      fwrite(&cr, 1, 1, f);
    }
  }



  fclose(f);
  return 0;


}
