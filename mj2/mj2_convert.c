#include <stdio.h>
#include <malloc.h>

#include "mj2.h"
#include <j2k.h>
#include <int.h>

/*  -----------------------	      */
/*				      */
/*				      */
/*  Count the number of frames	      */
/*  in a YUV file		      */
/*				      */
/*  -----------------------	      */

int yuv_num_frames(mj2_tk_t * tk, FILE *f)
{
  int numimages, frame_size;
  long end_of_f;

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

int yuvtoimage(FILE *yuvfile, mj2_tk_t * tk, j2k_image_t * img, 
	       int frame_num, int subsampling_dx, int subsampling_dy)
{
  int i, j;
  int offset;
  long end_of_f, position;
  FILE *Compo;

  offset =
    (int) ((double) (frame_num * tk->w * tk->h) *
	   (1.0 +
	    1.0 * (double) 2 / (double) (tk->CbCr_subsampling_dx *
					 tk->CbCr_subsampling_dy)));
  fseek(yuvfile, 0, SEEK_END);
  end_of_f = ftell(yuvfile);
  fseek(yuvfile, sizeof(unsigned char) * offset, SEEK_SET);
  position = ftell(yuvfile);
  if (position >= end_of_f) {
    fprintf(stderr, "Cannot reach frame number %d in yuv file !!\n",
	    frame_num);
    return 0;
  }

  img->x0 = tk->Dim[0];
  img->y0 = tk->Dim[1];
  img->x1 = !tk->Dim[0] ? (tk->w - 1) * subsampling_dx + 1 : tk->Dim[0] +
    (tk->w - 1) * subsampling_dx + 1;
  img->y1 = !tk->Dim[1] ? (tk->h - 1) * subsampling_dy + 1 : tk->Dim[1] +
    (tk->h - 1) * subsampling_dy + 1;
  img->numcomps = 3;
  img->color_space = 3;
  img->comps = (j2k_comp_t *) malloc(img->numcomps * sizeof(j2k_comp_t));

  for (i = 0; i < img->numcomps; i++) {
    img->comps[i].data = (int *) malloc(sizeof(int) * tk->w * tk->h);
    img->comps[i].prec = 8;
    img->comps[i].bpp = 8;
    img->comps[i].sgnd = 0;
    if (i == 0) {
      img->comps[i].dx = subsampling_dx;
      img->comps[i].dy = subsampling_dy;
    } else {
      img->comps[i].dx = subsampling_dx * tk->CbCr_subsampling_dx;
      img->comps[i].dy = subsampling_dy * tk->CbCr_subsampling_dy;
    }
  }

  Compo = fopen("Compo0", "wb");
  if (!Compo) {
    fprintf(stderr, "Failed to open Compo0 for writing !\n");
  }

  for (i = 0; i < (tk->w * tk->h / (img->comps[0].dx * img->comps[0].dy))
       && !feof(yuvfile); i++) {
    unsigned char y;
    j = fread(&y, 1, 1, yuvfile);
    fwrite(&y, 1, 1, Compo);
  }

  fclose(Compo);

  Compo = fopen("Compo1", "wb");
  if (!Compo) {
    fprintf(stderr, "Failed to open Compo1 for writing !\n");
  }


  for (i = 0; i < (tk->w * tk->h / (img->comps[1].dx * img->comps[1].dy))
       && !feof(yuvfile); i++) {
    unsigned char cb;
    j = fread(&cb, sizeof(unsigned char), 1, yuvfile);
    fwrite(&cb, 1, 1, Compo);
  }

  fclose(Compo);

  Compo = fopen("Compo2", "wb");
  if (!Compo) {
    fprintf(stderr, "Failed to open Compo2 for writing !\n");
  }


  for (i = 0; i < (tk->w * tk->h / (img->comps[2].dx * img->comps[2].dy))
       && !feof(yuvfile); i++) {
    unsigned char cr;
    j = fread(&cr, sizeof(unsigned char), 1, yuvfile);
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
  
  if (img->numcomps == 3) {
    if (img->comps[0].dx != img->comps[1].dx / 2
      || img->comps[1].dx != img->comps[2].dx) {
      fprintf(stderr,
	"Error with the input image components size: cannot create yuv file)\n");
      return 1;
    }
  } else if (!(img->numcomps == 1)) {
    fprintf(stderr,
      "Error with the number of image components(must be one or three)\n");
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
  
  
  if (img->numcomps == 3) {
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
      unsigned char cb = 125;
      fwrite(&cb, 1, 1, f);
    }
    
    
    for (i = 0; i < (img->comps[0].w * img->comps[0].h * 0.25); i++) {
      unsigned char cr = 125;
      fwrite(&cr, 1, 1, f);
    }
  }  
  fclose(f);
  return 0;
}

//  -----------------------
//
//
//  IMAGE to BMP
//
//  -----------------------

int imagetobmp(j2k_image_t * img, j2k_cp_t * cp, char *outfile) {
  int w,wr,h,hr,i,pad;
  FILE *f;
  
  if (img->numcomps == 3 && img->comps[0].dx == img->comps[1].dx
    && img->comps[1].dx == img->comps[2].dx
    && img->comps[0].dy == img->comps[1].dy
    && img->comps[1].dy == img->comps[2].dy
    && img->comps[0].prec == img->comps[1].prec
    && img->comps[1].prec == img->comps[2].prec) {
    /* -->> -->> -->> -->>
    
      24 bits color
      
    <<-- <<-- <<-- <<-- */
    
    f = fopen(outfile, "wb");
    if (!f) {
      fprintf(stderr, "failed to open %s for writing\n", outfile);
      return 1;
    }   
    
    w = img->comps[0].w;
    wr = int_ceildivpow2(img->comps[0].w, img->comps[0].factor);
    
    h = img->comps[0].h;
    hr = int_ceildivpow2(img->comps[0].h, img->comps[0].factor);
    
    fprintf(f, "BM");
    
    /* FILE HEADER */
    /* ------------- */
    fprintf(f, "%c%c%c%c",
      (unsigned char) (hr * wr * 3 + 3 * hr * (wr % 2) +
      54) & 0xff,
      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)
      >> 8) & 0xff,
      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)
      >> 16) & 0xff,
      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)
      >> 24) & 0xff);
    fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
      ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
    fprintf(f, "%c%c%c%c", (54) & 0xff, ((54) >> 8) & 0xff,
      ((54) >> 16) & 0xff, ((54) >> 24) & 0xff);
    
    /* INFO HEADER   */
    /* ------------- */
    fprintf(f, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,
      ((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
    fprintf(f, "%c%c%c%c", (unsigned char) ((wr) & 0xff),
      (unsigned char) ((wr) >> 8) & 0xff,
      (unsigned char) ((wr) >> 16) & 0xff,
      (unsigned char) ((wr) >> 24) & 0xff);
    fprintf(f, "%c%c%c%c", (unsigned char) ((hr) & 0xff),
      (unsigned char) ((hr) >> 8) & 0xff,
      (unsigned char) ((hr) >> 16) & 0xff,
      (unsigned char) ((hr) >> 24) & 0xff);
    fprintf(f, "%c%c", (1) & 0xff, ((1) >> 8) & 0xff);
    fprintf(f, "%c%c", (24) & 0xff, ((24) >> 8) & 0xff);
    fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
      ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
    fprintf(f, "%c%c%c%c",
      (unsigned char) (3 * hr * wr +
      3 * hr * (wr % 2)) & 0xff,
      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >>
      8) & 0xff,
      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >>
      16) & 0xff,
      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >>
      24) & 0xff);
    fprintf(f, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
      ((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
    fprintf(f, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
      ((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
    fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
      ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
    fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
      ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
    
    for (i = 0; i < wr * hr; i++) {
      unsigned char R, G, B;
      /* a modifier */
      // R = img->comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
      R = img->comps[0].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
      // G = img->comps[1].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
      G = img->comps[1].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
      // B = img->comps[2].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
      B = img->comps[2].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
      fprintf(f, "%c%c%c", B, G, R);
      
      if ((i + 1) % wr == 0) {
	for (pad = (3 * wr) % 4 ? 4 - (3 * wr) % 4 : 0; pad > 0; pad--)	/* ADD */
	  fprintf(f, "%c", 0);
      }
    }
    fclose(f);
    free(img->comps[1].data);
    free(img->comps[2].data);
  }
  return 0;
}