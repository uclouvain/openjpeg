/*
 * Copyright (c) 2002-2007, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2007, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux and Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * Copyright (c) 2006-2007, Parvatha Elangovan
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openjpeg.h"
#include "../libs/libtiff/tiffio.h"

/*
 * Get logarithm of an integer and round downwards.
 *
 * log2(a)
 */
static int int_floorlog2(int a) {
	int l;
	for (l = 0; a > 1; l++) {
		a >>= 1;
	}
	return l;
}

/*
 * Divide an integer by a power of 2 and round upwards.
 *
 * a divided by 2^b
 */
static int int_ceildivpow2(int a, int b) {
	return (a + (1 << b) - 1) >> b;
}

/*
 * Divide an integer and round upwards.
 *
 * a divided by b
 */
static int int_ceildiv(int a, int b) {
	return (a + b - 1) / b;
}

/* -->> -->> -->> -->>

  BMP IMAGE FORMAT

 <<-- <<-- <<-- <<-- */

/* WORD defines a two byte word */
typedef unsigned short int WORD;

/* DWORD defines a four byte word */
typedef unsigned long int DWORD;

typedef struct {
  WORD bfType;			/* 'BM' for Bitmap (19776) */
  DWORD bfSize;			/* Size of the file        */
  WORD bfReserved1;		/* Reserved : 0            */
  WORD bfReserved2;		/* Reserved : 0            */
  DWORD bfOffBits;		/* Offset                  */
} BITMAPFILEHEADER_t;

typedef struct {
  DWORD biSize;			/* Size of the structure in bytes */
  DWORD biWidth;		/* Width of the image in pixels */
  DWORD biHeight;		/* Heigth of the image in pixels */
  WORD biPlanes;		/* 1 */
  WORD biBitCount;		/* Number of color bits by pixels */
  DWORD biCompression;		/* Type of encoding 0: none 1: RLE8 2: RLE4 */
  DWORD biSizeImage;		/* Size of the image in bytes */
  DWORD biXpelsPerMeter;	/* Horizontal (X) resolution in pixels/meter */
  DWORD biYpelsPerMeter;	/* Vertical (Y) resolution in pixels/meter */
  DWORD biClrUsed;		/* Number of color used in the image (0: ALL) */
  DWORD biClrImportant;		/* Number of important color (0: ALL) */
} BITMAPINFOHEADER_t;

opj_image_t* bmptoimage(const char *filename, opj_cparameters_t *parameters) {
	int subsampling_dx = parameters->subsampling_dx;
	int subsampling_dy = parameters->subsampling_dy;

	int i, numcomps, w, h;
	OPJ_COLOR_SPACE color_space;
	opj_image_cmptparm_t cmptparm[3];	/* maximum of 3 components */
	opj_image_t * image = NULL;

	FILE *IN;
	BITMAPFILEHEADER_t File_h;
	BITMAPINFOHEADER_t Info_h;
	unsigned char *RGB;
	unsigned char *table_R, *table_G, *table_B;
	unsigned int j, PAD = 0;

	int x, y, index;
	int gray_scale = 1, not_end_file = 1; 

	unsigned int line = 0, col = 0;
	unsigned char v, v2;
	DWORD W, H;
  
	IN = fopen(filename, "rb");
	if (!IN) {
		fprintf(stderr, "Failed to open %s for reading !!\n", filename);
		return 0;
	}
	
	File_h.bfType = getc(IN);
	File_h.bfType = (getc(IN) << 8) + File_h.bfType;
	
	if (File_h.bfType != 19778) {
		fprintf(stderr,"Error, not a BMP file!\n");
		return 0;
	} else {
		/* FILE HEADER */
		/* ------------- */
		File_h.bfSize = getc(IN);
		File_h.bfSize = (getc(IN) << 8) + File_h.bfSize;
		File_h.bfSize = (getc(IN) << 16) + File_h.bfSize;
		File_h.bfSize = (getc(IN) << 24) + File_h.bfSize;

		File_h.bfReserved1 = getc(IN);
		File_h.bfReserved1 = (getc(IN) << 8) + File_h.bfReserved1;

		File_h.bfReserved2 = getc(IN);
		File_h.bfReserved2 = (getc(IN) << 8) + File_h.bfReserved2;

		File_h.bfOffBits = getc(IN);
		File_h.bfOffBits = (getc(IN) << 8) + File_h.bfOffBits;
		File_h.bfOffBits = (getc(IN) << 16) + File_h.bfOffBits;
		File_h.bfOffBits = (getc(IN) << 24) + File_h.bfOffBits;

		/* INFO HEADER */
		/* ------------- */

		Info_h.biSize = getc(IN);
		Info_h.biSize = (getc(IN) << 8) + Info_h.biSize;
		Info_h.biSize = (getc(IN) << 16) + Info_h.biSize;
		Info_h.biSize = (getc(IN) << 24) + Info_h.biSize;

		Info_h.biWidth = getc(IN);
		Info_h.biWidth = (getc(IN) << 8) + Info_h.biWidth;
		Info_h.biWidth = (getc(IN) << 16) + Info_h.biWidth;
		Info_h.biWidth = (getc(IN) << 24) + Info_h.biWidth;
		w = Info_h.biWidth;

		Info_h.biHeight = getc(IN);
		Info_h.biHeight = (getc(IN) << 8) + Info_h.biHeight;
		Info_h.biHeight = (getc(IN) << 16) + Info_h.biHeight;
		Info_h.biHeight = (getc(IN) << 24) + Info_h.biHeight;
		h = Info_h.biHeight;

		Info_h.biPlanes = getc(IN);
		Info_h.biPlanes = (getc(IN) << 8) + Info_h.biPlanes;

		Info_h.biBitCount = getc(IN);
		Info_h.biBitCount = (getc(IN) << 8) + Info_h.biBitCount;

		Info_h.biCompression = getc(IN);
		Info_h.biCompression = (getc(IN) << 8) + Info_h.biCompression;
		Info_h.biCompression = (getc(IN) << 16) + Info_h.biCompression;
		Info_h.biCompression = (getc(IN) << 24) + Info_h.biCompression;

		Info_h.biSizeImage = getc(IN);
		Info_h.biSizeImage = (getc(IN) << 8) + Info_h.biSizeImage;
		Info_h.biSizeImage = (getc(IN) << 16) + Info_h.biSizeImage;
		Info_h.biSizeImage = (getc(IN) << 24) + Info_h.biSizeImage;

		Info_h.biXpelsPerMeter = getc(IN);
		Info_h.biXpelsPerMeter = (getc(IN) << 8) + Info_h.biXpelsPerMeter;
		Info_h.biXpelsPerMeter = (getc(IN) << 16) + Info_h.biXpelsPerMeter;
		Info_h.biXpelsPerMeter = (getc(IN) << 24) + Info_h.biXpelsPerMeter;

		Info_h.biYpelsPerMeter = getc(IN);
		Info_h.biYpelsPerMeter = (getc(IN) << 8) + Info_h.biYpelsPerMeter;
		Info_h.biYpelsPerMeter = (getc(IN) << 16) + Info_h.biYpelsPerMeter;
		Info_h.biYpelsPerMeter = (getc(IN) << 24) + Info_h.biYpelsPerMeter;

		Info_h.biClrUsed = getc(IN);
		Info_h.biClrUsed = (getc(IN) << 8) + Info_h.biClrUsed;
		Info_h.biClrUsed = (getc(IN) << 16) + Info_h.biClrUsed;
		Info_h.biClrUsed = (getc(IN) << 24) + Info_h.biClrUsed;

		Info_h.biClrImportant = getc(IN);
		Info_h.biClrImportant = (getc(IN) << 8) + Info_h.biClrImportant;
		Info_h.biClrImportant = (getc(IN) << 16) + Info_h.biClrImportant;
		Info_h.biClrImportant = (getc(IN) << 24) + Info_h.biClrImportant;

		/* Read the data and store them in the OUT file */
    
		if (Info_h.biBitCount == 24) {
			numcomps = 3;
			color_space = CLRSPC_SRGB;
			/* initialize image components */
			memset(&cmptparm[0], 0, 3 * sizeof(opj_image_cmptparm_t));
			for(i = 0; i < numcomps; i++) {
				cmptparm[i].prec = 8;
				cmptparm[i].bpp = 8;
				cmptparm[i].sgnd = 0;
				cmptparm[i].dx = subsampling_dx;
				cmptparm[i].dy = subsampling_dy;
				cmptparm[i].w = w;
				cmptparm[i].h = h;
			}
			/* create the image */
			image = opj_image_create(numcomps, &cmptparm[0], color_space);
			if(!image) {
				fclose(IN);
				return NULL;
			}

			/* set image offset and reference grid */
			image->x0 = parameters->image_offset_x0;
			image->y0 = parameters->image_offset_y0;
			image->x1 =	!image->x0 ? (w - 1) * subsampling_dx + 1 : image->x0 + (w - 1) * subsampling_dx + 1;
			image->y1 =	!image->y0 ? (h - 1) * subsampling_dy + 1 : image->y0 + (h - 1) * subsampling_dy + 1;

			/* set image data */

			/* Place the cursor at the beginning of the image information */
			fseek(IN, 0, SEEK_SET);
			fseek(IN, File_h.bfOffBits, SEEK_SET);
			
			W = Info_h.biWidth;
			H = Info_h.biHeight;

			/* PAD = 4 - (3 * W) % 4; */
			/* PAD = (PAD == 4) ? 0 : PAD; */
			PAD = (3 * W) % 4 ? 4 - (3 * W) % 4 : 0;
			
			RGB = (unsigned char *) malloc((3 * W + PAD) * H * sizeof(unsigned char));
			
			fread(RGB, sizeof(unsigned char), (3 * W + PAD) * H, IN);
			
			index = 0;

			for(y = 0; y < (int)H; y++) {
				unsigned char *scanline = RGB + (3 * W + PAD) * (H - 1 - y);
				for(x = 0; x < (int)W; x++) {
					unsigned char *pixel = &scanline[3 * x];
					image->comps[0].data[index] = pixel[2];	/* R */
					image->comps[1].data[index] = pixel[1];	/* G */
					image->comps[2].data[index] = pixel[0];	/* B */
					index++;
				}
			}

			free(RGB);

		} else if (Info_h.biBitCount == 8 && Info_h.biCompression == 0) {
			table_R = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_G = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_B = (unsigned char *) malloc(256 * sizeof(unsigned char));
			
			for (j = 0; j < Info_h.biClrUsed; j++) {
				table_B[j] = getc(IN);
				table_G[j] = getc(IN);
				table_R[j] = getc(IN);
				getc(IN);
				if (table_R[j] != table_G[j] && table_R[j] != table_B[j] && table_G[j] != table_B[j])
					gray_scale = 0;
			}
			
			/* Place the cursor at the beginning of the image information */
			fseek(IN, 0, SEEK_SET);
			fseek(IN, File_h.bfOffBits, SEEK_SET);
			
			W = Info_h.biWidth;
			H = Info_h.biHeight;
			if (Info_h.biWidth % 2)
				W++;
			
			numcomps = gray_scale ? 1 : 3;
			color_space = gray_scale ? CLRSPC_GRAY : CLRSPC_SRGB;
			/* initialize image components */
			memset(&cmptparm[0], 0, 3 * sizeof(opj_image_cmptparm_t));
			for(i = 0; i < numcomps; i++) {
				cmptparm[i].prec = 8;
				cmptparm[i].bpp = 8;
				cmptparm[i].sgnd = 0;
				cmptparm[i].dx = subsampling_dx;
				cmptparm[i].dy = subsampling_dy;
				cmptparm[i].w = w;
				cmptparm[i].h = h;
			}
			/* create the image */
			image = opj_image_create(numcomps, &cmptparm[0], color_space);
			if(!image) {
				fclose(IN);
				return NULL;
			}

			/* set image offset and reference grid */
			image->x0 = parameters->image_offset_x0;
			image->y0 = parameters->image_offset_y0;
			image->x1 =	!image->x0 ? (w - 1) * subsampling_dx + 1 : image->x0 + (w - 1) * subsampling_dx + 1;
			image->y1 =	!image->y0 ? (h - 1) * subsampling_dy + 1 : image->y0 + (h - 1) * subsampling_dy + 1;

			/* set image data */

			RGB = (unsigned char *) malloc(W * H * sizeof(unsigned char));
			
			fread(RGB, sizeof(unsigned char), W * H, IN);
			if (gray_scale) {
				index = 0;
				for (j = 0; j < W * H; j++) {
					if ((j % W < W - 1 && Info_h.biWidth % 2) || !(Info_h.biWidth % 2)) {
						image->comps[0].data[index] = table_R[RGB[W * H - ((j) / (W) + 1) * W + (j) % (W)]];
						index++;
					}
				}

			} else {		
				index = 0;
				for (j = 0; j < W * H; j++) {
					if ((j % W < W - 1 && Info_h.biWidth % 2) || !(Info_h.biWidth % 2)) {
						unsigned char pixel_index = RGB[W * H - ((j) / (W) + 1) * W + (j) % (W)];
						image->comps[0].data[index] = table_R[pixel_index];
						image->comps[1].data[index] = table_G[pixel_index];
						image->comps[2].data[index] = table_B[pixel_index];
						index++;
					}
				}
			}
			free(RGB);
      free(table_R);
      free(table_G);
      free(table_B);
		} else if (Info_h.biBitCount == 8 && Info_h.biCompression == 1) {				
			table_R = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_G = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_B = (unsigned char *) malloc(256 * sizeof(unsigned char));
			
			for (j = 0; j < Info_h.biClrUsed; j++) {
				table_B[j] = getc(IN);
				table_G[j] = getc(IN);
				table_R[j] = getc(IN);
				getc(IN);
				if (table_R[j] != table_G[j] && table_R[j] != table_B[j] && table_G[j] != table_B[j])
					gray_scale = 0;
			}

			numcomps = gray_scale ? 1 : 3;
			color_space = gray_scale ? CLRSPC_GRAY : CLRSPC_SRGB;
			/* initialize image components */
			memset(&cmptparm[0], 0, 3 * sizeof(opj_image_cmptparm_t));
			for(i = 0; i < numcomps; i++) {
				cmptparm[i].prec = 8;
				cmptparm[i].bpp = 8;
				cmptparm[i].sgnd = 0;
				cmptparm[i].dx = subsampling_dx;
				cmptparm[i].dy = subsampling_dy;
				cmptparm[i].w = w;
				cmptparm[i].h = h;
			}
			/* create the image */
			image = opj_image_create(numcomps, &cmptparm[0], color_space);
			if(!image) {
				fclose(IN);
				return NULL;
			}

			/* set image offset and reference grid */
			image->x0 = parameters->image_offset_x0;
			image->y0 = parameters->image_offset_y0;
			image->x1 =	!image->x0 ? (w - 1) * subsampling_dx + 1 : image->x0 + (w - 1) * subsampling_dx + 1;
			image->y1 =	!image->y0 ? (h - 1) * subsampling_dy + 1 : image->y0 + (h - 1) * subsampling_dy + 1;

			/* set image data */
			
			/* Place the cursor at the beginning of the image information */
			fseek(IN, 0, SEEK_SET);
			fseek(IN, File_h.bfOffBits, SEEK_SET);
			
			RGB = (unsigned char *) malloc(Info_h.biWidth * Info_h.biHeight * sizeof(unsigned char));
            
			while (not_end_file) {
				v = getc(IN);
				if (v) {
					v2 = getc(IN);
					for (i = 0; i < (int) v; i++) {
						RGB[line * Info_h.biWidth + col] = v2;
						col++;
					}
				} else {
					v = getc(IN);
					switch (v) {
						case 0:
							col = 0;
							line++;
							break;
						case 1:
							line++;
							not_end_file = 0;
							break;
						case 2:
							fprintf(stderr,"No Delta supported\n");
							opj_image_destroy(image);
							fclose(IN);
							return NULL;
						default:
							for (i = 0; i < v; i++) {
								v2 = getc(IN);
								RGB[line * Info_h.biWidth + col] = v2;
								col++;
							}
							if (v % 2)
								v2 = getc(IN);
							break;
					}
				}
			}
			if (gray_scale) {
				index = 0;
				for (line = 0; line < Info_h.biHeight; line++) {
					for (col = 0; col < Info_h.biWidth; col++) {
						image->comps[0].data[index] = table_R[(int)RGB[(Info_h.biHeight - line - 1) * Info_h.biWidth + col]];
						index++;
					}
				}
			} else {
				index = 0;
				for (line = 0; line < Info_h.biHeight; line++) {
					for (col = 0; col < Info_h.biWidth; col++) {
						unsigned char pixel_index = (int)RGB[(Info_h.biHeight - line - 1) * Info_h.biWidth + col];
						image->comps[0].data[index] = table_R[pixel_index];
						image->comps[1].data[index] = table_G[pixel_index];
						image->comps[2].data[index] = table_B[pixel_index];
						index++;
					}
				}
			}
			free(RGB);
      free(table_R);
      free(table_G);
      free(table_B);
	} else {
		fprintf(stderr, 
			"Other system than 24 bits/pixels or 8 bits (no RLE coding) is not yet implemented [%d]\n", Info_h.biBitCount);
	}
	fclose(IN);
 }
 
 return image;
}

int imagetobmp(opj_image_t * image, const char *outfile) {
	int w, wr, h, hr;
	int i, pad;
	FILE *fdest = NULL;

	if (image->numcomps == 3 && image->comps[0].dx == image->comps[1].dx
		&& image->comps[1].dx == image->comps[2].dx
		&& image->comps[0].dy == image->comps[1].dy
		&& image->comps[1].dy == image->comps[2].dy
		&& image->comps[0].prec == image->comps[1].prec
		&& image->comps[1].prec == image->comps[2].prec) {
		
		/* -->> -->> -->> -->>    
		24 bits color	    
		<<-- <<-- <<-- <<-- */
	    
		fdest = fopen(outfile, "wb");
		if (!fdest) {
			fprintf(stderr, "ERROR -> failed to open %s for writing\n", outfile);
			return 1;
		}
	    
		/* w = int_ceildiv(image->x1 - image->x0, image->comps[0].dx); */
		/* wr = int_ceildiv(int_ceildivpow2(image->x1 - image->x0,image->factor), image->comps[0].dx); */
		w = image->comps[0].w;
		wr = int_ceildivpow2(image->comps[0].w, image->comps[0].factor);
	    
		/* h = int_ceildiv(image->y1 - image->y0, image->comps[0].dy); */
		/* hr = int_ceildiv(int_ceildivpow2(image->y1 - image->y0,image->factor), image->comps[0].dy); */
		h = image->comps[0].h;
		hr = int_ceildivpow2(image->comps[0].h, image->comps[0].factor);
	    
		fprintf(fdest, "BM");
	    
		/* FILE HEADER */
		/* ------------- */
		fprintf(fdest, "%c%c%c%c",
			(unsigned char) (hr * wr * 3 + 3 * hr * (wr % 2) + 54) & 0xff,
			(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)	>> 8) & 0xff,
			(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)	>> 16) & 0xff,
			(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)	>> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (54) & 0xff, ((54) >> 8) & 0xff,((54) >> 16) & 0xff, ((54) >> 24) & 0xff);
	    
		/* INFO HEADER   */
		/* ------------- */
		fprintf(fdest, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,	((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
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
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (unsigned char) (3 * hr * wr + 3 * hr * (wr % 2)) & 0xff,
			(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >> 8) & 0xff,
			(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >> 16) & 0xff,
			(unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff, ((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
	    
		for (i = 0; i < wr * hr; i++) {
			unsigned char R, G, B;
			/* a modifier */
			/* R = image->comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)]; */
			R = image->comps[0].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
			/* G = image->comps[1].data[w * h - ((i) / (w) + 1) * w + (i) % (w)]; */
			G = image->comps[1].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
			/* B = image->comps[2].data[w * h - ((i) / (w) + 1) * w + (i) % (w)]; */
			B = image->comps[2].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
			fprintf(fdest, "%c%c%c", B, G, R);
			
			if ((i + 1) % wr == 0) {
				for (pad = (3 * wr) % 4 ? 4 - (3 * wr) % 4 : 0; pad > 0; pad--)	/* ADD */
					fprintf(fdest, "%c", 0);
			}
		}
		fclose(fdest);
	} else {			/* Gray-scale */

		/* -->> -->> -->> -->>
		8 bits non code (Gray scale)
		<<-- <<-- <<-- <<-- */

		fdest = fopen(outfile, "wb");
		/* w = int_ceildiv(image->x1 - image->x0, image->comps[0].dx); */
		/* wr = int_ceildiv(int_ceildivpow2(image->x1 - image->x0,image->factor), image->comps[0].dx); */
		w = image->comps[0].w;
		wr = int_ceildivpow2(image->comps[0].w, image->comps[0].factor);
	    
		/* h = int_ceildiv(image->y1 - image->y0, image->comps[0].dy); */
		/* hr = int_ceildiv(int_ceildivpow2(image->y1 - image->y0,image->factor), image->comps[0].dy); */
		h = image->comps[0].h;
		hr = int_ceildivpow2(image->comps[0].h, image->comps[0].factor);
	    
		fprintf(fdest, "BM");
	    
		/* FILE HEADER */
		/* ------------- */
		fprintf(fdest, "%c%c%c%c", (unsigned char) (hr * wr + 54 + 1024 + hr * (wr % 2)) & 0xff,
			(unsigned char) ((hr * wr + 54 + 1024 + hr * (wr % 2)) >> 8) & 0xff,
			(unsigned char) ((hr * wr + 54 + 1024 + hr * (wr % 2)) >> 16) & 0xff,
			(unsigned char) ((hr * wr + 54 + 1024 + wr * (wr % 2)) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (54 + 1024) & 0xff, ((54 + 1024) >> 8) & 0xff, 
			((54 + 1024) >> 16) & 0xff,
			((54 + 1024) >> 24) & 0xff);
	    
		/* INFO HEADER */
		/* ------------- */
		fprintf(fdest, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,	((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
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
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (unsigned char) (hr * wr + hr * (wr % 2)) & 0xff,
			(unsigned char) ((hr * wr + hr * (wr % 2)) >> 8) &	0xff,
			(unsigned char) ((hr * wr + hr * (wr % 2)) >> 16) &	0xff,
			(unsigned char) ((hr * wr + hr * (wr % 2)) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff, ((256) >> 16) & 0xff, ((256) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff, ((256) >> 16) & 0xff, ((256) >> 24) & 0xff);

		for (i = 0; i < 256; i++) {
			fprintf(fdest, "%c%c%c%c", i, i, i, 0);
		}

		for (i = 0; i < wr * hr; i++) {
			/* a modifier !! */
			/* fprintf(fdest, "%c", image->comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)]); */
			fprintf(fdest, "%c", image->comps[0].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)]);
			/*if (((i + 1) % w == 0 && w % 2))
			fprintf(fdest, "%c", 0); */
			if ((i + 1) % wr == 0) {
				for (pad = wr % 4 ? 4 - wr % 4 : 0; pad > 0; pad--)	/* ADD */
					fprintf(fdest, "%c", 0);
			}
		}
		fclose(fdest);
	}

	return 0;
}

/* -->> -->> -->> -->>

PGX IMAGE FORMAT

<<-- <<-- <<-- <<-- */


unsigned char readuchar(FILE * f)
{
  unsigned char c1;
  fread(&c1, 1, 1, f);
  return c1;
}

unsigned short readushort(FILE * f, int bigendian)
{
  unsigned char c1, c2;
  fread(&c1, 1, 1, f);
  fread(&c2, 1, 1, f);
  if (bigendian)
    return (c1 << 8) + c2;
  else
    return (c2 << 8) + c1;
}

unsigned int readuint(FILE * f, int bigendian)
{
  unsigned char c1, c2, c3, c4;
  fread(&c1, 1, 1, f);
  fread(&c2, 1, 1, f);
  fread(&c3, 1, 1, f);
  fread(&c4, 1, 1, f);
  if (bigendian)
    return (c1 << 24) + (c2 << 16) + (c3 << 8) + c4;
  else
    return (c4 << 24) + (c3 << 16) + (c2 << 8) + c1;
}

opj_image_t* pgxtoimage(const char *filename, opj_cparameters_t *parameters) {
	FILE *f = NULL;
	int w, h, prec;
	int i, numcomps, max;
	OPJ_COLOR_SPACE color_space;
	opj_image_cmptparm_t cmptparm;	/* maximum of 1 component  */
	opj_image_t * image = NULL;

	char endian1,endian2,sign;
	char signtmp[32];

	char temp[32];
	int bigendian;
	opj_image_comp_t *comp = NULL;

	numcomps = 1;
	color_space = CLRSPC_GRAY;

	memset(&cmptparm, 0, sizeof(opj_image_cmptparm_t));

	max = 0;

	f = fopen(filename, "rb");
	if (!f) {
	  fprintf(stderr, "Failed to open %s for reading !\n", filename);
	  return NULL;
	}

	fseek(f, 0, SEEK_SET);
	fscanf(f, "PG%[ \t]%c%c%[ \t+-]%d%[ \t]%d%[ \t]%d",temp,&endian1,&endian2,signtmp,&prec,temp,&w,temp,&h);
	
	i=0;
	sign='+';		
	while (signtmp[i]!='\0') {
		if (signtmp[i]=='-') sign='-';
		i++;
	}
	
	fgetc(f);
	if (endian1=='M' && endian2=='L') {
		bigendian = 1;
	} else if (endian2=='M' && endian1=='L') {
		bigendian = 0;
	} else {
		fprintf(stderr, "Bad pgx header, please check input file\n");
		return NULL;
	}

	/* initialize image component */

	cmptparm.x0 = parameters->image_offset_x0;
	cmptparm.y0 = parameters->image_offset_y0;
	cmptparm.w = !cmptparm.x0 ? (w - 1) * parameters->subsampling_dx + 1 : cmptparm.x0 + (w - 1) * parameters->subsampling_dx + 1;
	cmptparm.h = !cmptparm.y0 ? (h - 1) * parameters->subsampling_dy + 1 : cmptparm.y0 + (h - 1) * parameters->subsampling_dy + 1;
	
	if (sign == '-') {
		cmptparm.sgnd = 1;
	} else {
		cmptparm.sgnd = 0;
	}
	cmptparm.prec = prec;
	cmptparm.bpp = prec;
	cmptparm.dx = parameters->subsampling_dx;
	cmptparm.dy = parameters->subsampling_dy;
	
	/* create the image */
	image = opj_image_create(numcomps, &cmptparm, color_space);
	if(!image) {
		fclose(f);
		return NULL;
	}
	/* set image offset and reference grid */
	image->x0 = cmptparm.x0;
	image->y0 = cmptparm.x0;
	image->x1 = cmptparm.w;
	image->y1 = cmptparm.h;

	/* set image data */

	comp = &image->comps[0];

	for (i = 0; i < w * h; i++) {
		int v;
		if (comp->prec <= 8) {
			if (!comp->sgnd) {
				v = readuchar(f);
			} else {
				v = (char) readuchar(f);
			}
		} else if (comp->prec <= 16) {
			if (!comp->sgnd) {
				v = readushort(f, bigendian);
			} else {
				v = (short) readushort(f, bigendian);
			}
		} else {
			if (!comp->sgnd) {
				v = readuint(f, bigendian);
			} else {
				v = (int) readuint(f, bigendian);
			}
		}
		if (v > max)
			max = v;
		comp->data[i] = v;
	}
	fclose(f);
	comp->bpp = int_floorlog2(max) + 1;

	return image;
}

int imagetopgx(opj_image_t * image, const char *outfile) {
	int w, wr, h, hr;
	int i, j, compno;
	FILE *fdest = NULL;

	for (compno = 0; compno < image->numcomps; compno++) {
		opj_image_comp_t *comp = &image->comps[compno];
		char bname[256]; /* buffer for name */
    char *name = bname; /* pointer */
    int nbytes = 0;
    const size_t olen = strlen(outfile);
    const size_t dotpos = olen - 4;
    const size_t total = dotpos + 1 + 1 + 4; /* '-' + '[1-3]' + '.pgx' */
    if( outfile[dotpos] != '.' ) {
      /* `pgx` was recognized but there is no dot at expected position */
      fprintf(stderr, "ERROR -> Impossible happen." );
      return 1;
      }
    if( total > 256 ) {
      name = (char*)malloc(total+1);
      }
    strncpy(name, outfile, dotpos);
		if (image->numcomps > 1) {
			sprintf(name+dotpos, "-%d.pgx", compno);
		} else {
			strcpy(name+dotpos, ".pgx");
		}
		fdest = fopen(name, "wb");
		if (!fdest) {
			fprintf(stderr, "ERROR -> failed to open %s for writing\n", name);
			return 1;
		}
    /* dont need name anymore */
    if( total > 256 ) {
      free(name);
      }
		/* w = int_ceildiv(image->x1 - image->x0, comp->dx); */
		/* wr = int_ceildiv(int_ceildivpow2(image->x1 - image->x0,image->factor), comp->dx); */
		w = image->comps[compno].w;
		wr = int_ceildivpow2(image->comps[compno].w, image->comps[compno].factor);
	    
		/* h = int_ceildiv(image->y1 - image->y0, comp->dy); */
		/* hr = int_ceildiv(int_ceildivpow2(image->y1 - image->y0,image->factor), comp->dy); */
		h = image->comps[compno].h;
		hr = int_ceildivpow2(image->comps[compno].h, image->comps[compno].factor);
	    
		fprintf(fdest, "PG ML %c %d %d %d\n", comp->sgnd ? '-' : '+', comp->prec, wr, hr);
		if (comp->prec <= 8) {
			nbytes = 1;
		} else if (comp->prec <= 16) {
			nbytes = 2;
		} else {
			nbytes = 4;
		}
		for (i = 0; i < wr * hr; i++) {
			int v = image->comps[compno].data[i / wr * w + i % wr];
			for (j = nbytes - 1; j >= 0; j--) {
				char byte = (char) (v >> (j * 8));
				fwrite(&byte, 1, 1, fdest);
			}
		}
		fclose(fdest);
	}

	return 0;
}

/* -->> -->> -->> -->>

PNM IMAGE FORMAT

<<-- <<-- <<-- <<-- */

opj_image_t* pnmtoimage(const char *filename, opj_cparameters_t *parameters) {
	int subsampling_dx = parameters->subsampling_dx;
	int subsampling_dy = parameters->subsampling_dy;

	FILE *f = NULL;
	int i, compno, numcomps, w, h;
	OPJ_COLOR_SPACE color_space;
	opj_image_cmptparm_t cmptparm[3];	/* maximum of 3 components */
	opj_image_t * image = NULL;
	char value;
	
	f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "Failed to open %s for reading !!\n", filename);
		return 0;
	}

	if (fgetc(f) != 'P')
		return 0;
	value = fgetc(f);

		switch(value) {
			case '2':	/* greyscale image type */
			case '5':
				numcomps = 1;
				color_space = CLRSPC_GRAY;
				break;
				
			case '3':	/* RGB image type */
			case '6':
				numcomps = 3;
				color_space = CLRSPC_SRGB;
				break;
				
			default:
				fclose(f);
				return NULL;
		}
		
		fgetc(f);
		
		/* skip comments */
		while(fgetc(f) == '#') while(fgetc(f) != '\n');
		
		fseek(f, -1, SEEK_CUR);
		fscanf(f, "%d %d\n255", &w, &h);			
		fgetc(f);	/* <cr><lf> */
		
	/* initialize image components */
	memset(&cmptparm[0], 0, 3 * sizeof(opj_image_cmptparm_t));
	for(i = 0; i < numcomps; i++) {
		cmptparm[i].prec = 8;
		cmptparm[i].bpp = 8;
		cmptparm[i].sgnd = 0;
		cmptparm[i].dx = subsampling_dx;
		cmptparm[i].dy = subsampling_dy;
		cmptparm[i].w = w;
		cmptparm[i].h = h;
	}
	/* create the image */
	image = opj_image_create(numcomps, &cmptparm[0], color_space);
	if(!image) {
		fclose(f);
		return NULL;
	}

	/* set image offset and reference grid */
	image->x0 = parameters->image_offset_x0;
	image->y0 = parameters->image_offset_y0;
	image->x1 = parameters->image_offset_x0 + (w - 1) *	subsampling_dx + 1;
	image->y1 = parameters->image_offset_y0 + (h - 1) *	subsampling_dy + 1;

	/* set image data */

	if ((value == '2') || (value == '3')) {	/* ASCII */
		for (i = 0; i < w * h; i++) {
			for(compno = 0; compno < numcomps; compno++) {
				unsigned int index = 0;
				fscanf(f, "%u", &index);
				/* compno : 0 = GREY, (0, 1, 2) = (R, G, B) */
				image->comps[compno].data[i] = index;
			}
		}
	} else if ((value == '5') || (value == '6')) {	/* BINARY */
		for (i = 0; i < w * h; i++) {
			for(compno = 0; compno < numcomps; compno++) {
				unsigned char index = 0;
				fread(&index, 1, 1, f);
				/* compno : 0 = GREY, (0, 1, 2) = (R, G, B) */
				image->comps[compno].data[i] = index;
			}
		}
	}

	fclose(f);

	return image;
}

int imagetopnm(opj_image_t * image, const char *outfile) {
	int w, wr, wrr, h, hr, hrr, max;
	int i, compno;
	int adjust;
	FILE *fdest = NULL;
	char S2;
	const char *tmp = outfile;

	while (*tmp) {
		tmp++;
	}
	tmp--;
	tmp--;
	S2 = *tmp;

	if (image->numcomps == 3 && image->comps[0].dx == image->comps[1].dx
		&& image->comps[1].dx == image->comps[2].dx
		&& image->comps[0].dy == image->comps[1].dy
		&& image->comps[1].dy == image->comps[2].dy
		&& image->comps[0].prec == image->comps[1].prec
		&& image->comps[1].prec == image->comps[2].prec
		&& S2 !='g' && S2 !='G') {

		fdest = fopen(outfile, "wb");
		if (!fdest) {
			fprintf(stderr, "ERROR -> failed to open %s for writing\n", outfile);
			return 1;
		}

		w = int_ceildiv(image->x1 - image->x0, image->comps[0].dx);
		/* wr = int_ceildiv(int_ceildivpow2(image->x1 - image->x0,image->factor),image->comps[0].dx); */
		wr = image->comps[0].w;
		wrr = int_ceildivpow2(image->comps[0].w, image->comps[0].factor);
        
		h = int_ceildiv(image->y1 - image->y0, image->comps[0].dy);
		/* hr = int_ceildiv(int_ceildivpow2(image->y1 - image->y0,image->factor), image->comps[0].dy); */
		hr = image->comps[0].h;
		hrr = int_ceildivpow2(image->comps[0].h, image->comps[0].factor);
	    
		max = image->comps[0].prec > 8 ? 255 : (1 << image->comps[0].prec) - 1;
	    
		image->comps[0].x0 = int_ceildivpow2(image->comps[0].x0 - int_ceildiv(image->x0, image->comps[0].dx), image->comps[0].factor);
		image->comps[0].y0 = int_ceildivpow2(image->comps[0].y0 -	int_ceildiv(image->y0, image->comps[0].dy), image->comps[0].factor);

		fprintf(fdest, "P6\n%d %d\n%d\n", wrr, hrr, max);
		adjust = image->comps[0].prec > 8 ? image->comps[0].prec - 8 : 0;
		for (i = 0; i < wrr * hrr; i++) {
			int r, g, b;
			unsigned char rc,gc,bc;
			r = image->comps[0].data[i / wrr * wr + i % wrr];
			r += (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
			rc = (unsigned char) ((r >> adjust)+((r >> (adjust-1))%2));

			g = image->comps[1].data[i / wrr * wr + i % wrr];
			g += (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
			gc = (unsigned char) ((g >> adjust)+((g >> (adjust-1))%2));
			
			b = image->comps[2].data[i / wrr * wr + i % wrr];
			b += (image->comps[2].sgnd ? 1 << (image->comps[2].prec - 1) : 0);
			bc = (unsigned char) ((b >> adjust)+((b >> (adjust-1))%2));
			
			fprintf(fdest, "%c%c%c", rc, gc, bc);
		}
		fclose(fdest);
	} else {
		int ncomp=(S2=='g' || S2=='G')?1:image->numcomps;
		if (image->numcomps>ncomp) {
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
            
			w = int_ceildiv(image->x1 - image->x0, image->comps[compno].dx);
			/* wr = int_ceildiv(int_ceildivpow2(image->x1 - image->x0,image->factor),image->comps[compno].dx); */
			wr = image->comps[compno].w;
			wrr = int_ceildivpow2(image->comps[compno].w, image->comps[compno].factor);
			
			h = int_ceildiv(image->y1 - image->y0, image->comps[compno].dy);
			/* hr = int_ceildiv(int_ceildivpow2(image->y1 - image->y0,image->factor), image->comps[compno].dy); */
			hr = image->comps[compno].h;
			hrr = int_ceildivpow2(image->comps[compno].h, image->comps[compno].factor);
			
			max = image->comps[compno].prec > 8 ? 255 : (1 << image->comps[compno].prec) - 1;
			
			image->comps[compno].x0 = int_ceildivpow2(image->comps[compno].x0 - int_ceildiv(image->x0, image->comps[compno].dx), image->comps[compno].factor);
			image->comps[compno].y0 = int_ceildivpow2(image->comps[compno].y0 - int_ceildiv(image->y0, image->comps[compno].dy), image->comps[compno].factor);
			
			fprintf(fdest, "P5\n%d %d\n%d\n", wrr, hrr, max);
			adjust = image->comps[compno].prec > 8 ? image->comps[compno].prec - 8 : 0;

			for (i = 0; i < wrr * hrr; i++) {
				int l;
				unsigned char lc;
				l = image->comps[compno].data[i / wrr * wr + i % wrr];
				l += (image->comps[compno].sgnd ? 1 << (image->comps[compno].prec - 1) : 0);
				lc = (unsigned char) ((l >> adjust)+((l >> (adjust-1))%2));
				fprintf(fdest, "%c", lc);
			}
			fclose(fdest);
		}
	}

	return 0;
}

/* -->> -->> -->> -->>

	TIFF IMAGE FORMAT

 <<-- <<-- <<-- <<-- */

typedef struct tiff_infoheader{
	DWORD tiWidth;  // Width of Image in pixel
	DWORD tiHeight; // Height of Image in pixel
	DWORD tiPhoto;	// Photometric
	WORD  tiBps;	// Bits per sample
	WORD  tiSf;		// Sample Format
	WORD  tiSpp;	// Sample per pixel 1-bilevel,gray scale , 2- RGB
	WORD  tiPC;	// Planar config (1-Interleaved, 2-Planarcomp)
}tiff_infoheader_t;

int imagetotif(opj_image_t * image, const char *outfile) {
	int width, height;
	int bps,index;
	TIFF *tif;
	tdata_t buf;
	tstrip_t strip;
	tsize_t strip_size;

	if (image->numcomps == 3 && image->comps[0].dx == image->comps[1].dx
		&& image->comps[1].dx == image->comps[2].dx
		&& image->comps[0].dy == image->comps[1].dy
		&& image->comps[1].dy == image->comps[2].dy
		&& image->comps[0].prec == image->comps[1].prec
		&& image->comps[1].prec == image->comps[2].prec) {

			/* -->> -->> -->>    
			RGB color	    
			<<-- <<-- <<-- */

			tif = TIFFOpen(outfile, "wb"); 
			if (!tif) {
				fprintf(stderr, "ERROR -> failed to open %s for writing\n", outfile);
				return 1;
			}

			width	= image->comps[0].w;
			height= image->comps[0].h;
			bps		= image->comps[0].prec;
			/* Set tags */
			TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
			TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
			TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps);
			TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
			TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
			TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);

			/* Get a buffer for the data */
			buf = _TIFFmalloc(TIFFStripSize(tif));
			index=0;
			strip_size=0;
			strip_size=TIFFStripSize(tif);
			for (strip = 0; strip < TIFFNumberOfStrips(tif); strip++) {
				unsigned char *dat8;
				int i;
				dat8 = buf;
				if (image->comps[0].prec == 8){
					for (i=0; i<TIFFStripSize(tif); i+=3) {	// 8 bits per pixel 
						dat8[i+0] = image->comps[0].data[index] ;	// R 
						dat8[i+1] = image->comps[1].data[index] ;	// G 
						dat8[i+2] = image->comps[2].data[index] ;	// B 
						index++;
					}
				}else if (image->comps[0].prec == 12){
					for (i=0; i<TIFFStripSize(tif); i+=9) {	// 12 bits per pixel 
						dat8[i+0] = (image->comps[0].data[index]>>8)<<4 | (image->comps[0].data[index]>>4);
						dat8[i+1] = (image->comps[0].data[index]<<4)|((image->comps[1].data[index]>>8)& 0x0f);
						dat8[i+2] = (image->comps[1].data[index]);
						dat8[i+3] = (image->comps[2].data[index]>>8)<<4 | (image->comps[2].data[index]>>4);
						dat8[i+4] = (image->comps[2].data[index]<<4)|((image->comps[1].data[index+1]>>8)& 0x0f);
						dat8[i+5] = (image->comps[0].data[index+1]);
						dat8[i+6] = (image->comps[1].data[index+1]>>8)<<4 | (image->comps[1].data[index+1]>>4);
						dat8[i+7] = (image->comps[1].data[index+1]<<4)|((image->comps[2].data[index+1]>>8)& 0x0f);
						dat8[i+8] = (image->comps[2].data[index+1]);
						index+=2;
					}
				}else if (image->comps[0].prec == 16){
					for (i=0; i<TIFFStripSize(tif); i+=6) {	// 16 bits per pixel 
						dat8[i+0] =  image->comps[0].data[index];//LSB
						dat8[i+1] = (image->comps[0].data[index]>> 8);//MSB	 
						dat8[i+2] =  image->comps[1].data[index]; 
						dat8[i+3] = (image->comps[1].data[index]>> 8);	
						dat8[i+4] =  image->comps[2].data[index];	 
						dat8[i+5] = (image->comps[2].data[index]>> 8); 
						index++;
					}
				}else{
					fprintf(stderr,"Bits=%d, Only 8,12,16 bits implemented\n",image->comps[0].prec);
					fprintf(stderr,"Aborting\n");
					return 1;
				}
				TIFFWriteEncodedStrip(tif, strip, buf, strip_size);
			}
			_TIFFfree(buf);
			TIFFClose(tif);
		}else if (image->numcomps == 1){
			/* -->> -->> -->>    
			Black and White	    
			<<-- <<-- <<-- */

			tif = TIFFOpen(outfile, "wb"); 
			if (!tif) {
				fprintf(stderr, "ERROR -> failed to open %s for writing\n", outfile);
				return 1;
			}

			width	= image->comps[0].w;
			height= image->comps[0].h;
			bps		= image->comps[0].prec;

			/* Set tags */
			TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
			TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
			TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps);
			TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
			TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
			TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);

			/* Get a buffer for the data */
			buf = _TIFFmalloc(TIFFStripSize(tif));
			index = 0;
			strip_size = 0;
			strip_size = TIFFStripSize(tif);
			for (strip = 0; strip < TIFFNumberOfStrips(tif); strip++) {
				unsigned char *dat8;
				int i;
				dat8 = buf;
				if (image->comps[0].prec == 8){
					for (i=0; i<TIFFStripSize(tif); i+=1) {	// 8 bits per pixel 
						dat8[i+0] = image->comps[0].data[index] ;
						index++;
					}
				}else if (image->comps[0].prec == 12){
					for (i = 0; i<TIFFStripSize(tif); i+=3) {	// 12 bits per pixel 
						dat8[i+0] = (image->comps[0].data[index]>>8)<<4 | (image->comps[0].data[index]>>4);
						dat8[i+1] = (image->comps[0].data[index]<<4)|((image->comps[0].data[index+1]>>8)& 0x0f);
						dat8[i+2] = (image->comps[0].data[index+1]);
						index+=2;
					}
				}else if (image->comps[0].prec == 16){
					for (i=0; i<TIFFStripSize(tif); i+=2) {	// 16 bits per pixel 
						dat8[i+0] =  image->comps[0].data[index];
						dat8[i+1] = (image->comps[0].data[index]>> 8);
						index++;
					}
				}else{
					fprintf(stderr,"Bits=%d, Only 8,12,16 bits implemented\n",image->comps[0].prec);
					fprintf(stderr,"Aborting\n");
					return 1;
				}
				TIFFWriteEncodedStrip(tif, strip, buf, strip_size);
			}
			_TIFFfree(buf);
			TIFFClose(tif);
		}else{
			fprintf(stderr,"False color format. Only RGB & Grayscale has been implemented\n");
			fprintf(stderr,"Aborting\n");
			return 1;
		}
		return 0;
}

opj_image_t* tiftoimage(char *filename, opj_cparameters_t *parameters)
{
	int subsampling_dx = parameters->subsampling_dx;
	int subsampling_dy = parameters->subsampling_dy;
	TIFF *tif;
	tiff_infoheader_t Info;
	tdata_t buf;
	tstrip_t strip;
	tsize_t strip_size;
	int j, numcomps, w, h,index;
	OPJ_COLOR_SPACE color_space;
	opj_image_cmptparm_t cmptparm[3];
	opj_image_t * image = NULL;

	tif = TIFFOpen(filename, "r");

	if (!tif) {
		fprintf(stderr, "Failed to open %s for reading\n", filename);
		return 0;
	}

	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &Info.tiWidth);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &Info.tiHeight);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &Info.tiBps);
	TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &Info.tiSf);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &Info.tiSpp);
	Info.tiPhoto = 0;
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &Info.tiPhoto);
	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &Info.tiPC);
	w= Info.tiWidth;
	h= Info.tiHeight;
	
	if (Info.tiPhoto == 2) { 
		/* -->> -->> -->>    
		RGB color	    
		<<-- <<-- <<-- */

		numcomps = 3;
		color_space = CLRSPC_SRGB;
		/* initialize image components*/ 
		memset(&cmptparm[0], 0, 3 * sizeof(opj_image_cmptparm_t));
		for(j = 0; j < numcomps; j++) {
			if (parameters->cp_cinema) {
				cmptparm[j].prec = 12;
				cmptparm[j].bpp = 12;
			}else{
				cmptparm[j].prec = Info.tiBps;
				cmptparm[j].bpp = Info.tiBps;
			}
			cmptparm[j].sgnd = 0;
			cmptparm[j].dx = subsampling_dx;
			cmptparm[j].dy = subsampling_dy;
			cmptparm[j].w = w;
			cmptparm[j].h = h;
		}
		/* create the image*/ 
		image = opj_image_create(numcomps, &cmptparm[0], color_space);
		if(!image) {
			TIFFClose(tif);
			return NULL;
		}

		/* set image offset and reference grid */
		image->x0 = parameters->image_offset_x0;
		image->y0 = parameters->image_offset_y0;
		image->x1 =	!image->x0 ? (w - 1) * subsampling_dx + 1 : image->x0 + (w - 1) * subsampling_dx + 1;
		image->y1 =	!image->y0 ? (h - 1) * subsampling_dy + 1 : image->y0 + (h - 1) * subsampling_dy + 1;

		buf = _TIFFmalloc(TIFFStripSize(tif));
		strip_size=0;
		strip_size=TIFFStripSize(tif);
		index = 0;
		/* Read the Image components*/
		for (strip = 0; strip < TIFFNumberOfStrips(tif); strip++) {
			unsigned char *dat8;
			int i, ssize;
			ssize = TIFFReadEncodedStrip(tif, strip, buf, strip_size);
			dat8 = buf;

			if (Info.tiBps==12){
				for (i=0; i<ssize; i+=9) {	/*12 bits per pixel*/
					image->comps[0].data[index]   = ( dat8[i+0]<<4 )		|(dat8[i+1]>>4);
					image->comps[1].data[index]   = ((dat8[i+1]& 0x0f)<< 8)	| dat8[i+2];
					image->comps[2].data[index]   = ( dat8[i+3]<<4)			|(dat8[i+4]>>4);
					image->comps[0].data[index+1] = ((dat8[i+4]& 0x0f)<< 8)	| dat8[i+5];
					image->comps[1].data[index+1] = ( dat8[i+6] <<4)		|(dat8[i+7]>>4);
					image->comps[2].data[index+1] = ((dat8[i+7]& 0x0f)<< 8)	| dat8[i+8];
					index+=2;
				}
			}
			else if( Info.tiBps==16){
				for (i=0; i<ssize; i+=6) {	/* 16 bits per pixel */
					image->comps[0].data[index] = ( dat8[i+1] << 8 ) | dat8[i+0];	// R 
					image->comps[1].data[index] = ( dat8[i+3] << 8 ) | dat8[i+2];	// G 
					image->comps[2].data[index] = ( dat8[i+5] << 8 ) | dat8[i+4];	// B 
					if(parameters->cp_cinema){/* Rounding to 12 bits*/
						image->comps[0].data[index] = (image->comps[0].data[index] + 0x08) >> 4 ;
						image->comps[1].data[index] = (image->comps[1].data[index] + 0x08) >> 4 ;
						image->comps[2].data[index] = (image->comps[2].data[index] + 0x08) >> 4 ;
					}
					index++;
				}
			}
			else if ( Info.tiBps==8){
				for (i=0; i<ssize; i+=3) {	/* 8 bits per pixel */
					image->comps[0].data[index] = dat8[i+0];	// R 
					image->comps[1].data[index] = dat8[i+1];	// G 
					image->comps[2].data[index] = dat8[i+2];	// B 
					if(parameters->cp_cinema){/* Rounding to 12 bits*/
						image->comps[0].data[index] = image->comps[0].data[index] << 4 ;
						image->comps[1].data[index] = image->comps[1].data[index] << 4 ;
						image->comps[2].data[index] = image->comps[2].data[index] << 4 ;
					}
					index++;
				}
			}
			else{
				fprintf(stderr,"Bits=%d, Only 8,12,16 bits implemented\n",Info.tiBps);
				fprintf(stderr,"Aborting\n");
				return NULL;
			}
		}

		_TIFFfree(buf);
		TIFFClose(tif);
	}else if(Info.tiPhoto == 1) { 
		/* -->> -->> -->>    
		Black and White
		<<-- <<-- <<-- */

		numcomps = 1;
		color_space = CLRSPC_GRAY;
		/* initialize image components*/ 
		memset(&cmptparm[0], 0, sizeof(opj_image_cmptparm_t));
		cmptparm[0].prec = Info.tiBps;
		cmptparm[0].bpp = Info.tiBps;
		cmptparm[0].sgnd = 0;
		cmptparm[0].dx = subsampling_dx;
		cmptparm[0].dy = subsampling_dy;
		cmptparm[0].w = w;
		cmptparm[0].h = h;

		/* create the image*/ 
		image = opj_image_create(numcomps, &cmptparm[0], color_space);
		if(!image) {
			TIFFClose(tif);
			return NULL;
		}
		/* set image offset and reference grid */
		image->x0 = parameters->image_offset_x0;
		image->y0 = parameters->image_offset_y0;
		image->x1 =	!image->x0 ? (w - 1) * subsampling_dx + 1 : image->x0 + (w - 1) * subsampling_dx + 1;
		image->y1 =	!image->y0 ? (h - 1) * subsampling_dy + 1 : image->y0 + (h - 1) * subsampling_dy + 1;

		buf = _TIFFmalloc(TIFFStripSize(tif));
		strip_size = 0;
		strip_size = TIFFStripSize(tif);
		index = 0;
		/* Read the Image components*/
		for (strip = 0; strip < TIFFNumberOfStrips(tif); strip++) {
			unsigned char *dat8;
			int i, ssize;
			ssize = TIFFReadEncodedStrip(tif, strip, buf, strip_size);
			dat8 = buf;

			if (Info.tiBps==12){
				for (i=0; i<ssize; i+=3) {	/* 12 bits per pixel*/
					image->comps[0].data[index] = ( dat8[i+0]<<4 )				|(dat8[i+1]>>4) ;
					image->comps[0].data[index] = ((dat8[i+1]& 0x0f)<< 8)	| dat8[i+2];
					index+=2;
				}
			}
			else if( Info.tiBps==16){
				for (i=0; i<ssize; i+=2) {	/* 16 bits per pixel */
					image->comps[0].data[index] = ( dat8[i+1] << 8 ) | dat8[i+0];
					index++;
				}
			}
			else if ( Info.tiBps==8){
				for (i=0; i<ssize; i+=1) {	/* 8 bits per pixel */
					image->comps[0].data[index] = dat8[i+0];
					index++;
				}
			}
			else{
				fprintf(stderr,"Bits=%d, Only 8,12,16 bits implemented\n",Info.tiBps);
				fprintf(stderr,"Aborting\n");
				return NULL;
			}
		}

		_TIFFfree(buf);
		TIFFClose(tif);
	}else{
		fprintf(stderr,"False color format. Only RGB & Grayscale has been implemented\n");
		fprintf(stderr,"Aborting\n");
		return NULL;
	}
	return image;
}
