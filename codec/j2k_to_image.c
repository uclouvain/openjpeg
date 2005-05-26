/* Copyright (c) 2001 David Janssens
* Copyright (c) 2002-2003 Yannick Verschueren
* Copyright (c) 2002-2003 Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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



//MEMORY LEAK

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>  // Must be included first

#include <crtdbg.h>

#endif

//MEM



#include <openjpeg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef DONT_HAVE_GETOPT
#include <getopt.h>
#else
#include "compat/getopt.h"
#endif

void usage_display(char *prgm)
{
  fprintf(stdout,"Usage:\n");
  fprintf(stdout,"  %s...\n",prgm);
  fprintf(stdout,"  -i <compressed file>\n");
  fprintf(stdout,"    REQUIRED\n");
  fprintf(stdout,"    Currently accepts J2K-files, JP2-files and JPT-files. The file type\n");
  fprintf(stdout,"    is identified based on its suffix.\n");
  fprintf(stdout,"  -o <decompressed file>\n");
  fprintf(stdout,"    REQUIRED\n");
  fprintf(stdout,"    Currently accepts PGM-files, PPM-files, PNM-files, PGX-files and\n");
  fprintf(stdout,"    BMP-files. Binary data is written to the file (not ascii). If a PGX\n");
  fprintf(stdout,"    filename is given, there will be as many output files as there are\n");
  fprintf(stdout,"    components: an indice starting from 0 will then be appended to the\n");
  fprintf(stdout,"    output filename, just before the \"pgx\" extension. If a PGM filename\n");
  fprintf(stdout,"    is given and there are more than one component, only the first component\n");
  fprintf(stdout,"    will be written to the file.\n");
  fprintf(stdout,"  -r <reduce factor>\n");
  fprintf(stdout,"    Set the number of highest resolution levels to be discarded. The\n");
  fprintf(stdout,"    image resolution is effectively divided by 2 to the power of the\n");
  fprintf(stdout,"    number of discarded levels. The reduce factor is limited by the\n");
  fprintf(stdout,"    smallest total number of decomposition levels among tiles.\n");
  fprintf(stdout,"  -l <number of quality layers to decode>\n");
  fprintf(stdout,"    Set the maximum number of quality layers to decode. If there are\n");
  fprintf(stdout,"    less quality layers than the specified number, all the quality layers\n");
  fprintf(stdout,"    are decoded.\n");
  fprintf(stdout,"  -u\n");
  fprintf(stdout,"    print an usage statement\n");
  fprintf(stdout,"\n");
}

int main(int argc, char **argv)
{
  FILE *fsrc=NULL;
  FILE *fdest=NULL;
  char *infile=NULL;
  char *outfile=NULL;
  char *tmp=NULL;
  char S1, S2, S3;
  
  char *src=NULL; 
  
  int len;
  
  j2k_image_t img;
  j2k_cp_t cp;
  jp2_struct_t *jp2_struct=NULL;
  
  int w, wr, wrr, h, hr, hrr, max;
  int i, compno, pad, j;
  int adjust;
  
  cp.layer=0;
  cp.reduce=0;
  cp.decod_format=-1;
  cp.cod_format=-1;
  
  while (1) {
    int c = getopt(argc, argv,"i:o:r:l:u");
    if (c == -1)
      break;
    switch (c) {
      
      //Input file
    case 'i':
      infile = optarg;
      tmp = optarg;
      while (*tmp) {
	tmp++;
      }
      tmp--;
      S3 = *tmp;
      tmp--;
      S2 = *tmp;
      tmp--;
      S1 = *tmp;
      
      /* J2K format */
      if ((S1 == 'j' && S2 == '2' && S3 == 'k')
	|| (S1 == 'J' && S2 == '2' && S3 == 'K') 
	|| (S1 == 'j' && S2 == '2' && S3 == 'c')
	|| (S1 == 'J' && S2 == '2' && S3 == 'C')) {
	cp.cod_format=J2K_CFMT;
	break;
      }
      
      /* JP2 format */
      if ((S1 == 'j' && S2 == 'p' && S3 == '2')
	|| (S1 == 'J' && S2 == 'P' && S3 == '2')) {
	cp.cod_format=JP2_CFMT;
	break;
      }
      
      /* JPT format */
      if ((S1 == 'j' && S2 == 'p' && S3 == 't')
	|| (S1 == 'J' && S2 == 'P' && S3 == 'T')) {
	cp.cod_format=JPT_CFMT;
	break;
      }
      
      fprintf(stderr,
	"j2k_to_image : Unknown input image format *.%c%c%c [only *.j2k, *.jp2, *.jpc or *.jpt]!! \n",
	S1, S2, S3);
      return 1;
      break;
      
      /* ----------------------------------------------------- */
      
      //Output file
    case 'o':
      outfile = optarg;
      tmp = optarg;
      while (*tmp) {
	tmp++;
      }
      tmp--;
      S3 = *tmp;
      tmp--;
      S2 = *tmp;
      tmp--;
      S1 = *tmp;
      
      // PGX format      
      if ((S1 == 'p' && S2 == 'g' && S3 == 'x')
	|| (S1 == 'P' && S2 == 'G' && S3 == 'X')) {
	cp.decod_format = PGX_DFMT;
	break;
      }
      
      // PxM format 
      if ((S1 == 'p' && S2 == 'n' && S3 == 'm')
	|| (S1 == 'P' && S2 == 'N' && S3 == 'M') 
	|| (S1 == 'p' && S2 == 'g' && S3 == 'm')
	|| (S1 == 'P' && S2 == 'G' && S3 == 'M') 
	|| (S1 == 'P' && S2 == 'P' && S3 == 'M')
	|| (S1 == 'p' && S2 == 'p' && S3 == 'm')) {
	cp.decod_format = PXM_DFMT;
	break;
      }
      
      // BMP format 
      if ((S1 == 'b' && S2 == 'm' && S3 == 'p')
	|| (S1 == 'B' && S2 == 'M' && S3 == 'P')) {
	cp.decod_format = BMP_DFMT;
	break;
      }
      
      // otherwise : error
      fprintf(stderr,
	"!! Unrecognized output image format *.%c%c%c [only *.pnm, *.pgm, *.ppm, *.pgx or *.bmp] !!\n",
	S1, S2, S3);
      
      return 1;
      break;
      
      /* ----------------------------------------------------- */
      
      //Reduce option
    case 'r':
      tmp=optarg;
      sscanf(tmp, "%d", &cp.reduce);
      break;
      
      /* ----------------------------------------------------- */
      
      //Layering option
    case 'l':
      tmp=optarg;
      sscanf(tmp, "%d", &cp.layer);
      break;
      
      /* ----------------------------------------------------- */
      
    case 'u':			
      usage_display(argv[0]);
      return 0;
      break;
      /* ----------------------------------------------------- */
      
    default:
      fprintf(stderr,"WARNING -> this option is not valid \"-%c %s\"\n",c,optarg);
      break;
      
    }
  }
  
  //Check required arguments
  //------------------------
  if (!infile || !outfile) {
    fprintf(stderr,"ERROR -> At least one required argument is missing\nCheck j2k_to_image -u for usage information\n");
    return 1;
  }
  
  //Read the input file and put it in memory
  //----------------------------------------
  fsrc = fopen(infile, "rb");
  if (!fsrc) {
    fprintf(stderr, "ERROR -> failed to open %s for reading\n", infile);
    return 1;
  }
  fseek(fsrc, 0, SEEK_END);
  len = ftell(fsrc);
  fseek(fsrc, 0, SEEK_SET);
  src = (char *) malloc(len);
  fread(src, 1, len, fsrc);
  fclose(fsrc);
  
  //Decode the code-stream
  //----------------------
  switch(cp.cod_format) {
    
  case J2K_CFMT:
    if (!j2k_decode(src, len, &img, &cp)) {
      fprintf(stderr, "ERROR -> j2k_to_image: failed to decode image!\n");
      return 1;
    }
    break;
    
  case JP2_CFMT:
    jp2_struct = (jp2_struct_t *) malloc(sizeof(jp2_struct_t));
    jp2_struct->image = &img;
    
    if (jp2_read_struct(src, jp2_struct, len)) {
      fprintf(stderr, "ERROR -> j2k_to_image: failed to decode jp2 structure!\n");
      return 1;
    }
    
    if (!j2k_decode(src + jp2_struct->j2k_codestream_offset, jp2_struct->j2k_codestream_len, &img, &cp)) {
      fprintf(stderr, "ERROR -> j2k_to_image: failed to decode image!\n");
      return 1;
    }
    
    /* Insert code here if you want to create actions on jp2_struct before deleting it */
    
    free(jp2_struct);
    break;
    
  case JPT_CFMT:
    if (!j2k_decode_jpt_stream(src, len, &img, &cp)) {
      fprintf(stderr, "ERROR -> j2k_to_image: failed to decode JPT-file!\n");
      return 1;
    }
    break;
    
  default:
    fprintf(stderr,
      "ERROR -> j2k_to_image : Unknown input image format\n");
    return 1;
    break;
  }
  
  //Free the memory containing the code-stream
  //------------------------------------------
  
  free(src);
  
  
  //Create output image
  //-------------------
  
  /* ---------------------------- / */
  /* /                            / */
  /* /  FORMAT : PNM, PGM or PPM  / */
  /* /                            / */
  /* ---------------------------- / */
  
  switch (cp.decod_format) {
  case PXM_DFMT:			/* PNM PGM PPM */
    
    tmp=outfile;
    while (*tmp) {
      tmp++;
    }
    tmp--;
    tmp--;
    S2 = *tmp;
    
    if (img.numcomps == 3 && img.comps[0].dx == img.comps[1].dx
      && img.comps[1].dx == img.comps[2].dx
      && img.comps[0].dy == img.comps[1].dy
      && img.comps[1].dy == img.comps[2].dy
      && img.comps[0].prec == img.comps[1].prec
      && img.comps[1].prec == img.comps[2].prec
      && S2 !='g' && S2 !='G') {
      
      fdest = fopen(outfile, "wb");
      if (!fdest) {
	fprintf(stderr, "ERROR -> failed to open %s for writing\n", outfile);
	return 1;
      }
      
      w = int_ceildiv(img.x1 - img.x0, img.comps[0].dx);
      // wr = int_ceildiv(int_ceildivpow2(img.x1 - img.x0,img.factor),img.comps[0].dx);
      wr = img.comps[0].w;
      wrr = int_ceildivpow2(img.comps[0].w, img.comps[0].factor);
      
      h = int_ceildiv(img.y1 - img.y0, img.comps[0].dy);
      // hr = int_ceildiv(int_ceildivpow2(img.y1 - img.y0,img.factor), img.comps[0].dy);
      hr = img.comps[0].h;
      hrr = int_ceildivpow2(img.comps[0].h, img.comps[0].factor);
      
      max = img.comps[0].prec > 8 ? 255 : (1 << img.comps[0].prec) - 1;
      
      img.comps[0].x0 =
	int_ceildivpow2(img.comps[0].x0 -
	int_ceildiv(img.x0, img.comps[0].dx),
	img.comps[0].factor);
      img.comps[0].y0 =
	int_ceildivpow2(img.comps[0].y0 -
	int_ceildiv(img.y0, img.comps[0].dy),
	img.comps[0].factor);
      
      
      fprintf(fdest, "P6\n# %d %d %d %d %d\n%d %d\n%d\n",
	cp.tcps[cp.tileno[0]].tccps[0].numresolutions, w, h,
	img.comps[0].x0, img.comps[0].y0, wrr, hrr, max);
      adjust = img.comps[0].prec > 8 ? img.comps[0].prec - 8 : 0;
      for (i = 0; i < wrr * hrr; i++) {
	char r, g, b;
	r = img.comps[0].data[i / wrr * wr + i % wrr];
	r += (img.comps[0].sgnd ? 1 << (img.comps[0].prec - 1) : 0);
	r = r >> adjust;
	
	g = img.comps[1].data[i / wrr * wr + i % wrr];
	g += (img.comps[1].sgnd ? 1 << (img.comps[1].prec - 1) : 0);
	g = g >> adjust;
	
	b = img.comps[2].data[i / wrr * wr + i % wrr];
	b += (img.comps[2].sgnd ? 1 << (img.comps[2].prec - 1) : 0);
	b = b >> adjust;
	
	fprintf(fdest, "%c%c%c", r, g, b);
      }
      free(img.comps[0].data);
      free(img.comps[1].data);
      free(img.comps[2].data);
      fclose(fdest);
      
    } else {
      int ncomp=(S2=='g' || S2=='G')?1:img.numcomps;
      if (img.numcomps>ncomp) {
	fprintf(stderr,"WARNING -> [PGM files] Only the first component\n");
	fprintf(stderr,"           is written to the file\n");
      }
      for (compno = 0; compno < ncomp; compno++) {
	char name[256];
	if (ncomp > 1) {
	  sprintf(name, "%d.%s", compno, outfile);
	} else {
	  sprintf(name, "%s", outfile);
	}
	
	fdest = fopen(name, "wb");
	if (!fdest) {
	  fprintf(stderr, "ERROR -> failed to open %s for writing\n", name);
	  return 1;
	}
	
	w = int_ceildiv(img.x1 - img.x0, img.comps[compno].dx);
	// wr = int_ceildiv(int_ceildivpow2(img.x1 - img.x0,img.factor),img.comps[compno].dx);
	wr = img.comps[compno].w;
	wrr =
	  int_ceildivpow2(img.comps[compno].w, img.comps[compno].factor);
	
	h = int_ceildiv(img.y1 - img.y0, img.comps[compno].dy);
	// hr = int_ceildiv(int_ceildivpow2(img.y1 - img.y0,img.factor), img.comps[compno].dy);
	hr = img.comps[compno].h;
	hrr =
	  int_ceildivpow2(img.comps[compno].h, img.comps[compno].factor);
	
	max =
	  img.comps[compno].prec >
	  8 ? 255 : (1 << img.comps[compno].prec) - 1;
	
	img.comps[compno].x0 =
	  int_ceildivpow2(img.comps[compno].x0 -
	  int_ceildiv(img.x0,
				      img.comps[compno].dx),
				      img.comps[compno].factor);
	img.comps[compno].y0 =
	  int_ceildivpow2(img.comps[compno].y0 -
	  int_ceildiv(img.y0,
				      img.comps[compno].dy),
				      img.comps[compno].factor);
	
	fprintf(fdest, "P5\n# %d %d %d %d %d\n%d %d\n%d\n",
	  cp.tcps[cp.tileno[0]].tccps[compno].
	  numresolutions, w, h, img.comps[compno].x0,
	  img.comps[compno].y0, wrr, hrr, max);
	adjust =
	  img.comps[compno].prec > 8 ? img.comps[compno].prec - 8 : 0;
	for (i = 0; i < wrr * hrr; i++) {
	  char l;
	  l = img.comps[compno].data[i / wrr * wr + i % wrr];
	  l += (img.comps[compno].
	    sgnd ? 1 << (img.comps[compno].prec - 1) : 0);
	  l = l >> adjust;
	  fprintf(fdest, "%c", l);
	}
	fclose(fdest);
	free(img.comps[compno].data);
      }
    }
    break;
    
    /* ------------------------ / */
    /* /                        / */
    /* /     FORMAT : PGX       / */
    /* /                        / */
    /* /----------------------- / */
  case PGX_DFMT:			/* PGX */
    for (compno = 0; compno < img.numcomps; compno++) {
      j2k_comp_t *comp = &img.comps[compno];
      char name[256];
      int nbytes = 0;
      tmp = outfile;
      while (*tmp) {
	tmp++;
      }
      while (*tmp!='.') {
	tmp--;
      }
      *tmp='\0';
      //if (img.numcomps > 1)
      sprintf(name, "%s-%d.pgx", outfile, compno);
      
      //else
      
      //sprintf(name, "%s.pgx", outfile);
      
      fdest = fopen(name, "wb");
      if (!fdest) {
	fprintf(stderr, "ERROR -> failed to open %s for writing\n", name);
	return 1;
      }
      
      // w = int_ceildiv(img.x1 - img.x0, comp->dx);
      // wr = int_ceildiv(int_ceildivpow2(img.x1 - img.x0,img.factor), comp->dx);
      w = img.comps[compno].w;
      wr = int_ceildivpow2(img.comps[compno].w, img.comps[compno].factor);
      
      // h = int_ceildiv(img.y1 - img.y0, comp->dy);
      // hr = int_ceildiv(int_ceildivpow2(img.y1 - img.y0,img.factor), comp->dy);
      h = img.comps[compno].h;
      hr = int_ceildivpow2(img.comps[compno].h, img.comps[compno].factor);
      
      fprintf(fdest, "PG ML %c %d %d %d\n", comp->sgnd ? '-' : '+',
	comp->prec, wr, hr);
      
      if (comp->prec <= 8)
	nbytes = 1;
      
      else if (comp->prec <= 16)
	nbytes = 2;
      
      else
	nbytes = 4;
      for (i = 0; i < wr * hr; i++) {
	int v = img.comps[compno].data[i / wr * w + i % wr];
	
	for (j = nbytes - 1; j >= 0; j--) {
	  
	  char byte = (char) (v >> (j * 8));
	  
	  fwrite(&byte, 1, 1, fdest);
	  
	}
      }
      free(img.comps[compno].data);
      fclose(fdest);
    }
    break;
    
    /* ------------------------ / */
    /* /                        / */
    /* /     FORMAT : BMP       / */
    /* /                        / */
    /* /----------------------- / */
    
  case BMP_DFMT:			/* BMP */
    if (img.numcomps == 3 && img.comps[0].dx == img.comps[1].dx
      && img.comps[1].dx == img.comps[2].dx
      && img.comps[0].dy == img.comps[1].dy
      && img.comps[1].dy == img.comps[2].dy
      && img.comps[0].prec == img.comps[1].prec
      && img.comps[1].prec == img.comps[2].prec) {
      /* -->> -->> -->> -->>
      
       24 bits color
       
      <<-- <<-- <<-- <<-- */
      
      fdest = fopen(outfile, "wb");
      if (!fdest) {
	fprintf(stderr, "ERROR -> failed to open %s for writing\n", outfile);
	return 1;
      }
      
      // w = int_ceildiv(img.x1 - img.x0, img.comps[0].dx);
      // wr = int_ceildiv(int_ceildivpow2(img.x1 - img.x0,img.factor), img.comps[0].dx);
      w = img.comps[0].w;
      wr = int_ceildivpow2(img.comps[0].w, img.comps[0].factor);
      
      // h = int_ceildiv(img.y1 - img.y0, img.comps[0].dy);
      // hr = int_ceildiv(int_ceildivpow2(img.y1 - img.y0,img.factor), img.comps[0].dy);
      h = img.comps[0].h;
      hr = int_ceildivpow2(img.comps[0].h, img.comps[0].factor);
      
      fprintf(fdest, "BM");
      
      /* FILE HEADER */
      /* ------------- */
      fprintf(fdest, "%c%c%c%c",
	(unsigned char) (hr * wr * 3 + 3 * hr * (wr % 2) +
	54) & 0xff,
	(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)
	>> 8) & 0xff,
	(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)
	>> 16) & 0xff,
	(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)
	>> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (54) & 0xff, ((54) >> 8) & 0xff,
	((54) >> 16) & 0xff, ((54) >> 24) & 0xff);
      
      /* INFO HEADER   */
      /* ------------- */
      fprintf(fdest, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,
	((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (unsigned char) ((wr) & 0xff),
	(unsigned char) ((wr) >> 8) & 0xff,
	(unsigned char) ((wr) >> 16) & 0xff,
	(unsigned char) ((wr) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (unsigned char) ((hr) & 0xff),
	(unsigned char) ((hr) >> 8) & 0xff,
	(unsigned char) ((hr) >> 16) & 0xff,
	(unsigned char) ((hr) >> 24) & 0xff);
      fprintf(fdest, "%c%c", (1) & 0xff, ((1) >> 8) & 0xff);
      fprintf(fdest, "%c%c", (24) & 0xff, ((24) >> 8) & 0xff);
      fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c",
	(unsigned char) (3 * hr * wr +
	3 * hr * (wr % 2)) & 0xff,
	(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >>
	8) & 0xff,
	(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >>
	16) & 0xff,
	(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >>
	24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
      
      for (i = 0; i < wr * hr; i++) {
	unsigned char R, G, B;
	/* a modifier */
	// R = img.comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
	R = img.comps[0].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
	// G = img.comps[1].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
	G = img.comps[1].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
	// B = img.comps[2].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
	B = img.comps[2].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
	fprintf(fdest, "%c%c%c", B, G, R);
	
	if ((i + 1) % wr == 0) {
	  for (pad = (3 * wr) % 4 ? 4 - (3 * wr) % 4 : 0; pad > 0; pad--)	/* ADD */
	    fprintf(fdest, "%c", 0);
	}
      }
      fclose(fdest);
      free(img.comps[1].data);
      free(img.comps[2].data);
    } else {			/* Gray-scale */
      
				/* -->> -->> -->> -->>
				
				 8 bits non code (Gray scale)
				 
	<<-- <<-- <<-- <<-- */
      fdest = fopen(outfile, "wb");
      // w = int_ceildiv(img.x1 - img.x0, img.comps[0].dx);
      // wr = int_ceildiv(int_ceildivpow2(img.x1 - img.x0,img.factor), img.comps[0].dx);
      w = img.comps[0].w;
      wr = int_ceildivpow2(img.comps[0].w, img.comps[0].factor);
      
      // h = int_ceildiv(img.y1 - img.y0, img.comps[0].dy);
      // hr = int_ceildiv(int_ceildivpow2(img.y1 - img.y0,img.factor), img.comps[0].dy);
      h = img.comps[0].h;
      hr = int_ceildivpow2(img.comps[0].h, img.comps[0].factor);
      
      fprintf(fdest, "BM");
      
      /* FILE HEADER */
      /* ------------- */
      fprintf(fdest, "%c%c%c%c",
	(unsigned char) (hr * wr + 54 + 1024 +
	hr * (wr % 2)) & 0xff,
	(unsigned char) ((hr * wr + 54 + 1024 + hr * (wr % 2))
	>> 8) & 0xff,
	(unsigned char) ((hr * wr + 54 + 1024 + hr * (wr % 2))
	>> 16) & 0xff,
	(unsigned char) ((hr * wr + 54 + 1024 + wr * (wr % 2))
	>> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (54 + 1024) & 0xff,
	((54 + 1024) >> 8) & 0xff, ((54 + 1024) >> 16) & 0xff,
	((54 + 1024) >> 24) & 0xff);
      
      /* INFO HEADER */
      /* ------------- */
      fprintf(fdest, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,
	((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (unsigned char) ((wr) & 0xff),
	(unsigned char) ((wr) >> 8) & 0xff,
	(unsigned char) ((wr) >> 16) & 0xff,
	(unsigned char) ((wr) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (unsigned char) ((hr) & 0xff),
	(unsigned char) ((hr) >> 8) & 0xff,
	(unsigned char) ((hr) >> 16) & 0xff,
	(unsigned char) ((hr) >> 24) & 0xff);
      fprintf(fdest, "%c%c", (1) & 0xff, ((1) >> 8) & 0xff);
      fprintf(fdest, "%c%c", (8) & 0xff, ((8) >> 8) & 0xff);
      fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c",
	(unsigned char) (hr * wr + hr * (wr % 2)) & 0xff,
	(unsigned char) ((hr * wr + hr * (wr % 2)) >> 8) &
	0xff,
	(unsigned char) ((hr * wr + hr * (wr % 2)) >> 16) &
	0xff,
	(unsigned char) ((hr * wr + hr * (wr % 2)) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff,
	((256) >> 16) & 0xff, ((256) >> 24) & 0xff);
      fprintf(fdest, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff,
	((256) >> 16) & 0xff, ((256) >> 24) & 0xff);
    }
    
    for (i = 0; i < 256; i++) {
      fprintf(fdest, "%c%c%c%c", i, i, i, 0);
    }
    
    for (i = 0; i < wr * hr; i++) {
      /* a modifier !! */
      // fprintf(fdest, "%c", img.comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)]);
      fprintf(fdest, "%c",
	img.comps[0].data[w * hr - ((i) / (wr) + 1) * w +
	(i) % (wr)]);
	/*if (((i + 1) % w == 0 && w % 2))
      fprintf(fdest, "%c", 0); */
      if ((i + 1) % wr == 0) {
	for (pad = wr % 4 ? 4 - wr % 4 : 0; pad > 0; pad--)	/* ADD */
	  fprintf(fdest, "%c", 0);
      }
    }
    fclose(fdest);
    free(img.comps[0].data);
    break;

  default:
    fprintf(stderr,
      "ERROR -> j2k_to_image : Unknown output image format\n");
    return 1;
    break;
  }
  
  
  // Free remaining structures
  //--------------------------
  j2k_dec_release();
  
  
  
  // Check memory leaks if debug mode
  //---------------------------------
  
#ifdef _DEBUG
  
  _CrtDumpMemoryLeaks();
  
#endif
  
  return 0;
}
