/*
 * Copyright (c) 2001-2003, David Janssens
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

#include <openjpeg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

/* -->> -->> -->> -->>

  BMP IMAGE FORMAT

 <<-- <<-- <<-- <<-- */

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

typedef struct {
	UINT2 bfType;					/* 'BM' for Bitmap (19776) */
	UINT4 bfSize;					/* Size of the file        */
	UINT2 bfReserved1;				/* Reserved : 0            */
	UINT2 bfReserved2;				/* Reserved : 0            */
	UINT4 bfOffBits;				/* Offset                  */
} BITMAPFILEHEADER_t;

typedef struct {
	UINT4 biSize;					/* Size of the structure in bytes */
	UINT4 biWidth;					/* Width of the image in pixels */
	UINT4 biHeight;					/* Heigth of the image in pixels */
	UINT2 biPlanes;					/* 1 */
	UINT2 biBitCount;				/* Number of color bits by pixels */
	UINT4 biCompression;				/* Type of encoding 0: none 1: RLE8 2: RLE4 */
	UINT4 biSizeImage;				/* Size of the image in bytes */
	UINT4 biXpelsPerMeter;				/* Horizontal (X) resolution in pixels/meter */
	UINT4 biYpelsPerMeter;				/* Vertical (Y) resolution in pixels/meter */
	UINT4 biClrUsed;				/* Number of color used in the image (0: ALL) */
	UINT4 biClrImportant;				/* Number of important color (0: ALL) */
} BITMAPINFOHEADER_t;

int bmptoimage(char *filename, j2k_image_t * img, int subsampling_dx, int subsampling_dy, int Dim[2])
{
	FILE *IN;
	FILE *Compo0 = NULL, *Compo1 = NULL, *Compo2 = NULL;
	BITMAPFILEHEADER_t File_h;
	BITMAPINFOHEADER_t Info_h;
	unsigned char *RGB;
	unsigned char *table_R, *table_G, *table_B;
	int i, w, h, PAD, type = 0;
	int gray_scale = 1, not_end_file = 1, line = 0, col = 0;
	unsigned char v, v2;
	UINT4 W, H;

	IN = fopen(filename, "rb");
	if (!IN) {
		fprintf(stderr,	"\033[0;33mFailed to open %s for reading !!\033[0;39m\n", filename);
		return 0;
	}

	File_h.bfType = getc(IN);
	File_h.bfType = (getc(IN) << 8) + File_h.bfType;

	if (File_h.bfType != 19778) {
		printf("Error, not a BMP file!\n");
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
			img->x0 = Dim[0];
			img->y0 = Dim[1];
			img->x1 = !Dim[0] ? (w - 1) * subsampling_dx + 1 : Dim[0] + (w - 1) * subsampling_dx + 1;
			img->y1 = !Dim[1] ? (h - 1) * subsampling_dy + 1 : Dim[1] + (h - 1) * subsampling_dy + 1;
			img->numcomps = 3;
			img->comps = (j2k_comp_t *) malloc(img->numcomps * sizeof(j2k_comp_t));
			for (i = 0; i < img->numcomps; i++) {
				img->comps[i].prec = 8;
				img->comps[i].bpp = 8;
				img->comps[i].sgnd = 0;
				img->comps[i].dx = subsampling_dx;
				img->comps[i].dy = subsampling_dy;
			}
			Compo0 = fopen("Compo0", "wb");
			if (!Compo0) {
				fprintf(stderr,	"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
			}
			Compo1 = fopen("Compo1", "wb");
			if (!Compo1) {
				fprintf(stderr,	"\033[0;33mFailed to open Compo1 for writing !\033[0;39m\n");
			}
			Compo2 = fopen("Compo2", "wb");
			if (!Compo2) {
				fprintf(stderr,	"\033[0;33mFailed to open Compo2 for writing !\033[0;39m\n");
			}

			/* Place the cursor at the beginning of the image information */
			fseek(IN, 0, SEEK_SET);
			fseek(IN, File_h.bfOffBits, SEEK_SET);

			W = Info_h.biWidth;
			H = Info_h.biHeight;

			// PAD = 4 - (3 * W) % 4;
			// PAD = (PAD == 4) ? 0 : PAD;
			PAD = (3 * W) % 4 ? 4 - (3 * W) % 4 : 0;

			
			RGB = (unsigned char *) malloc((3 * W + PAD) * H * sizeof(unsigned char));

			fread(RGB, sizeof(unsigned char), (3 * W + PAD) * H, IN);
			
			for (i = 0; i < (3 * W + PAD) * H; i++)
			  {
			    unsigned char elmt;
			    int Wp = 3 * W + PAD;
			    
			    elmt = RGB[(H - (i/Wp + 1)) * Wp + i % Wp];
			    if ((i % Wp) < (3 * W))
			      {
				switch (type)
				  {
				  case 0:
				    fprintf(Compo2, "%c", elmt);
				    type = 1;
				    break;
				  case 1:
				    fprintf(Compo1, "%c", elmt);
				    type = 2;
				    break;
				  case 2:
				    fprintf(Compo0, "%c", elmt);
				    type = 0;
				    break;
				  }
			      }
			  }

			fclose(Compo0);
			fclose(Compo1);
			fclose(Compo2);
			free(RGB);
		} else if (Info_h.biBitCount == 8 && Info_h.biCompression == 0) {
			img->x0 = Dim[0];
			img->y0 = Dim[1];
			img->x1 = !Dim[0] ? (w - 1) * subsampling_dx + 1 : Dim[0] + (w - 1) * subsampling_dx + 1;
			img->y1 = !Dim[1] ? (h - 1) * subsampling_dy + 1 : Dim[1] + (h - 1) * subsampling_dy + 1;

			table_R = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_G = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_B = (unsigned char *) malloc(256 * sizeof(unsigned char));

			for (i = 0; i < Info_h.biClrUsed; i++) {
				table_B[i] = getc(IN);
				table_G[i] = getc(IN);
				table_R[i] = getc(IN);
				getc(IN);
				if (table_R[i] != table_G[i] && table_R[i] != table_B[i] && table_G[i] != table_B[i])
					gray_scale = 0;
			}

			/* Place the cursor at the beginning of the image information */
			fseek(IN, 0, SEEK_SET);
			fseek(IN, File_h.bfOffBits, SEEK_SET);

			W = Info_h.biWidth;
			H = Info_h.biHeight;
			if (Info_h.biWidth % 2)
				W++;

			RGB = (unsigned char *) malloc(W * H * sizeof(unsigned char));

			fread(RGB, sizeof(unsigned char), W * H, IN);
			if (gray_scale) {
				img->numcomps = 1;
				img->comps = (j2k_comp_t *) malloc(img->numcomps * sizeof(j2k_comp_t));
				img->comps[0].prec = 8;
				img->comps[0].bpp = 8;
				img->comps[0].sgnd = 0;
				img->comps[0].dx = subsampling_dx;
				img->comps[0].dy = subsampling_dy;
				Compo0 = fopen("Compo0", "wb");
				if (!Compo0) {
					fprintf(stderr,	"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
				}
				for (i = 0; i < W * H; i++) {
					if ((i % W < W - 1 && Info_h.biWidth % 2) || !(Info_h.biWidth % 2))
						fprintf(Compo0, "%c", table_R[RGB[W * H - ((i) / (W) + 1) * W + (i) % (W)]]);
				}
				fclose(Compo0);
			} else {
				img->numcomps = 3;
				img->comps = (j2k_comp_t *) malloc(img->numcomps * sizeof(j2k_comp_t));
				for (i = 0; i < img->numcomps; i++) {
					img->comps[i].prec = 8;
					img->comps[i].bpp = 8;
					img->comps[i].sgnd = 0;
					img->comps[i].dx = subsampling_dx;
					img->comps[i].dy = subsampling_dy;
				}

				Compo0 = fopen("Compo0", "wb");
				if (!Compo0) {
					fprintf(stderr,	"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
				}
				Compo1 = fopen("Compo1", "wb");
				if (!Compo1) {
					fprintf(stderr,	"\033[0;33mFailed to open Compo1 for writing !\033[0;39m\n");
				}
				Compo2 = fopen("Compo2", "wb");
				if (!Compo2) {
					fprintf(stderr,	"\033[0;33mFailed to open Compo2 for writing !\033[0;39m\n");
				}

				for (i = 0; i < W * H; i++) {
					if ((i % W < W - 1 && Info_h.biWidth % 2) || !(Info_h.biWidth % 2)) {
						fprintf(Compo0, "%c", table_R[RGB[W * H - ((i) / (W) + 1) * W + (i) % (W)]]);
						fprintf(Compo1, "%c", table_G[RGB[W * H - ((i) / (W) + 1) * W + (i) % (W)]]);
						fprintf(Compo2, "%c", table_B[RGB[W * H - ((i) / (W) + 1) * W + (i) % (W)]]);
					}

				}
				fclose(Compo0);
				fclose(Compo1);
				fclose(Compo2);
			}
			free(RGB);

		} else if (Info_h.biBitCount == 8 && Info_h.biCompression == 1) {
			img->x0 = Dim[0];
			img->y0 = Dim[1];
			img->x1 = !Dim[0] ? (w - 1) * subsampling_dx + 1 : Dim[0] + (w - 1) * subsampling_dx + 1;
			img->y1 = !Dim[1] ? (h - 1) * subsampling_dy + 1 : Dim[1] + (h - 1) * subsampling_dy + 1;

			table_R = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_G = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_B = (unsigned char *) malloc(256 * sizeof(unsigned char));

			for (i = 0; i < Info_h.biClrUsed; i++) {
				table_B[i] = getc(IN);
				table_G[i] = getc(IN);
				table_R[i] = getc(IN);
				getc(IN);
				if (table_R[i] != table_G[i] && table_R[i] != table_B[i] && table_G[i] != table_B[i])
					gray_scale = 0;
			}

			/* Place the cursor at the beginning of the image information */
			fseek(IN, 0, SEEK_SET);
			fseek(IN, File_h.bfOffBits, SEEK_SET);

			if (gray_scale) {
				img->numcomps = 1;
				img->comps = (j2k_comp_t *) malloc(sizeof(j2k_comp_t));
				img->comps[0].prec = 8;
				img->comps[0].bpp = 8;
				img->comps[0].sgnd = 0;
				img->comps[0].dx = subsampling_dx;
				img->comps[0].dy = subsampling_dy;
				Compo0 = fopen("Compo0", "wb");
				if (!Compo0) {
					fprintf(stderr,	"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
				}
			} else {
				img->numcomps = 3;
				img->comps = (j2k_comp_t *) malloc(img->numcomps * sizeof(j2k_comp_t));
				for (i = 0; i < img->numcomps; i++) {
					img->comps[i].prec = 8;
					img->comps[i].bpp = 8;
					img->comps[i].sgnd = 0;
					img->comps[i].dx = subsampling_dx;
					img->comps[i].dy = subsampling_dy;
				}
				Compo0 = fopen("Compo0", "wb");
				if (!Compo0) {
					fprintf(stderr,	"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
				}
				Compo1 = fopen("Compo1", "wb");
				if (!Compo1) {
					fprintf(stderr,	"\033[0;33mFailed to open Compo1 for writing !\033[0;39m\n");
				}
				Compo2 = fopen("Compo2", "wb");
				if (!Compo2) {
					fprintf(stderr,	"\033[0;33mFailed to open Compo2 for writing !\033[0;39m\n");
				}
			}

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
						printf("No Delta supported\n");
						return 1;
						break;
					default:
						for (i = 0; i < v; i++) {
							v2 = getc(IN);
							RGB[line * Info_h.biWidth + col] = v2;
							col++;
						}
						if (v % 2)
							v2 = getc(IN);
					}
				}
			}
			if (gray_scale) {
				for (line = 0; line < Info_h.biHeight; line++)
					for (col = 0; col < Info_h.biWidth; col++)
						fprintf(Compo0, "%c", table_R[(int)RGB[(Info_h.biHeight - line - 1) * Info_h.biWidth + col]]);
				fclose(Compo0);
			} else {
				for (line = 0; line < Info_h.biHeight; line++)
					for (col = 0; col < Info_h.biWidth; col++) {
						fprintf(Compo0, "%c", table_R[(int)RGB[(Info_h.biHeight - line - 1) * Info_h.biWidth + col]]);
						fprintf(Compo1, "%c", table_G[(int)RGB[(Info_h.biHeight - line - 1) * Info_h.biWidth + col]]);
						fprintf(Compo2, "%c", table_B[(int)RGB[(Info_h.biHeight - line - 1) * Info_h.biWidth + col]]);
					}
				fclose(Compo0);
				fclose(Compo1);
				fclose(Compo2);
			}
			free(RGB);
		} else
			fprintf(stderr,"Other system than 24 bits/pixels or 8 bits (no RLE coding) is not yet implemented [%d]\n",
				Info_h.biBitCount);

		fclose(IN);
		return 1;
	}
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

int pgxtoimage(char *filename, j2k_image_t * img, int tdy,
							 int subsampling_dx, int subsampling_dy, int Dim[2],
							 j2k_cp_t cp)
{
	FILE *f;
	int w, h, prec;
	int i, compno, bandno;
	char str[256], endian[16];
	char sign;
	int bigendian;
	j2k_comp_t *comp;

	img->numcomps = 1;
	img->comps = (j2k_comp_t *) malloc(img->numcomps * sizeof(j2k_comp_t));
	for (compno = 0; compno < img->numcomps; compno++) {
		FILE *src;
		char tmp[16];
		int max = 0;
		int Y1;
		comp = &img->comps[compno];
		sprintf(str, "%s", filename);
		f = fopen(str, "rb");
		if (!f) {
			fprintf(stderr, "Failed to open %s for reading !\n", str);
			return 0;
		}
		if (fscanf(f, "PG %s %c %d %d %d", endian, &sign, &prec, &w, &h) == 5)
		{
			fgetc(f);
			if (!strcmp(endian, "ML"))
				bigendian = 1;
			else
				bigendian = 0;
			if (compno == 0) {
				img->x0 = Dim[0];
				img->y0 = Dim[1];
				img->x1 =
					!Dim[0] ? (w - 1) * subsampling_dx + 1 : Dim[0] + (w -
																														 1) *
					subsampling_dx + 1;
				img->y1 =
					!Dim[1] ? (h - 1) * subsampling_dy + 1 : Dim[1] + (h -
																														 1) *
					subsampling_dy + 1;
			} else {
				if (w != img->x1 || h != img->y1)
					return 0;
			}

			if (sign == '-') {
				comp->sgnd = 1;
			} else {
				comp->sgnd = 0;
			}
			comp->prec = prec;
			comp->dx = subsampling_dx;
			comp->dy = subsampling_dy;
			bandno = 1;

			Y1 =
				cp.ty0 + bandno * cp.tdy <
				img->y1 ? cp.ty0 + bandno * cp.tdy : img->y1;
			Y1 -= img->y0;

			sprintf(tmp, "bandtile%d", bandno);	/* bandtile file */
			src = fopen(tmp, "wb");
			if (!src) {
				fprintf(stderr, "failed to open %s for writing !\n", tmp);
			}
			for (i = 0; i < w * h; i++) {
				int v;
				if (i == Y1 * w / subsampling_dy && tdy != -1) {	/* bandtile is full */
					fclose(src);
					bandno++;
					sprintf(tmp, "bandtile%d", bandno);
					src = fopen(tmp, "wb");
					if (!src) {
						fprintf(stderr, "failed to open %s for writing !\n", tmp);
					}
					Y1 =
						cp.ty0 + bandno * cp.tdy <
						img->y1 ? cp.ty0 + bandno * cp.tdy : img->y1;
					Y1 -= img->y0;
				}
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
				fprintf(src, "%d ", v);
			}
		} else {
			return 0;
		}
		fclose(f);
		fclose(src);
		comp->bpp = int_floorlog2(max) + 1;
	}
	return 1;
}

/* -->> -->> -->> -->>

  PNM IMAGE FORMAT

 <<-- <<-- <<-- <<-- */

int pnmtoimage(char *filename, j2k_image_t * img, int subsampling_dx,
							 int subsampling_dy, int Dim[2])
{
	FILE *f;
	FILE *Compo0, *Compo1, *Compo2;
	int w, h;
	int i;
	char value;
	char comment[256];

	f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr,
						"\033[0;33mFailed to open %s for reading !!\033[0;39m\n",
						filename);
		return 0;
	}

	if (fgetc(f) != 'P')
		return 0;
	value = fgetc(f);

	if (value == '2') {
		fgetc(f);
		if (fgetc(f) == '#') {
			fseek(f, 0, SEEK_SET);
			fscanf(f, "P2\n");
			fgets(comment, 256, f);
			fscanf(f, "%d %d\n255", &w, &h);
		} else {
			fseek(f, 0, SEEK_SET);
			fscanf(f, "P2\n%d %d\n255", &w, &h);
		}

		fgetc(f);
		img->x0 = Dim[0];
		img->y0 = Dim[1];
		img->x1 =
			!Dim[0] ? (w - 1) * subsampling_dx + 1 : Dim[0] + (w -
																												 1) *
			subsampling_dx + 1;
		img->y1 =
			!Dim[1] ? (h - 1) * subsampling_dy + 1 : Dim[1] + (h -
																												 1) *
			subsampling_dy + 1;

		img->numcomps = 1;
		img->comps = (j2k_comp_t *) malloc(sizeof(j2k_comp_t));
		img->comps[0].prec = 8;
		img->comps[0].bpp = 8;
		img->comps[0].sgnd = 0;
		img->comps[0].dx = subsampling_dx;
		img->comps[0].dy = subsampling_dy;

		Compo0 = fopen("Compo0", "wb");
		if (!Compo0) {
			fprintf(stderr,
							"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
		}
		for (i = 0; i < w * h; i++) {
			unsigned int l;
			fscanf(f, "%d", &l);
			fprintf(Compo0, "%c", l);
		}
		fclose(Compo0);
	} else if (value == '5') {
		fgetc(f);
		if (fgetc(f) == '#') {
			fseek(f, 0, SEEK_SET);
			fscanf(f, "P5\n");
			fgets(comment, 256, f);
			fscanf(f, "%d %d\n255", &w, &h);
		} else {
			fseek(f, 0, SEEK_SET);
			fscanf(f, "P5\n%d %d\n255", &w, &h);
		}

		fgetc(f);
		img->x0 = Dim[0];
		img->y0 = Dim[1];
		img->x1 =
			!Dim[0] ? (w - 1) * subsampling_dx + 1 : Dim[0] + (w -
																												 1) *
			subsampling_dx + 1;
		img->y1 =
			!Dim[1] ? (h - 1) * subsampling_dy + 1 : Dim[1] + (h -
																												 1) *
			subsampling_dy + 1;

		img->numcomps = 1;
		img->comps = (j2k_comp_t *) malloc(sizeof(j2k_comp_t));
		img->comps[0].prec = 8;
		img->comps[0].bpp = 8;
		img->comps[0].sgnd = 0;
		img->comps[0].dx = subsampling_dx;
		img->comps[0].dy = subsampling_dy;
		Compo0 = fopen("Compo0", "wb");
		if (!Compo0) {
			fprintf(stderr,
							"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
		}
		for (i = 0; i < w * h; i++) {
			unsigned char l;
			fread(&l, 1, 1, f);
			fwrite(&l, 1, 1, Compo0);
		}
		fclose(Compo0);
	} else if (value == '3') {
		fgetc(f);
		if (fgetc(f) == '#') {
			fseek(f, 0, SEEK_SET);
			fscanf(f, "P3\n");
			fgets(comment, 256, f);
			fscanf(f, "%d %d\n255", &w, &h);
		} else {
			fseek(f, 0, SEEK_SET);
			fscanf(f, "P3\n%d %d\n255", &w, &h);
		}

		fgetc(f);
		img->x0 = Dim[0];
		img->y0 = Dim[1];
		img->x1 =
			!Dim[0] ? (w - 1) * subsampling_dx + 1 : Dim[0] + (w -
																												 1) *
			subsampling_dx + 1;
		img->y1 =
			!Dim[1] ? (h - 1) * subsampling_dy + 1 : Dim[1] + (h -
																												 1) *
			subsampling_dy + 1;
		img->numcomps = 3;
		img->comps = (j2k_comp_t *) malloc(img->numcomps * sizeof(j2k_comp_t));
		for (i = 0; i < img->numcomps; i++) {
			img->comps[i].prec = 8;
			img->comps[i].bpp = 8;
			img->comps[i].sgnd = 0;
			img->comps[i].dx = subsampling_dx;
			img->comps[i].dy = subsampling_dy;
		}
		Compo0 = fopen("Compo0", "wb");
		if (!Compo0) {
			fprintf(stderr,
							"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
		}

		Compo1 = fopen("Compo1", "wb");
		if (!Compo1) {
			fprintf(stderr,
							"\033[0;33mFailed to open Compo1 for writing !\033[0;39m\n");
		}

		Compo2 = fopen("Compo2", "wb");
		if (!Compo2) {
			fprintf(stderr,
							"\033[0;33mFailed to open Compo2 for writing !\033[0;39m\n");
		}

		for (i = 0; i < w * h; i++) {
			unsigned int r, g, b;
			fscanf(f, "%d", &r);
			fscanf(f, "%d", &g);
			fscanf(f, "%d", &b);
			fprintf(Compo0, "%c", r);
			fprintf(Compo1, "%c", g);
			fprintf(Compo2, "%c", b);
		}
		fclose(Compo0);
		fclose(Compo1);
		fclose(Compo2);
	} else if (value == '6') {
		fgetc(f);
		if (fgetc(f) == '#') {
			fseek(f, 0, SEEK_SET);
			fscanf(f, "P6\n");
			fgets(comment, 256, f);
			fscanf(f, "%d %d\n255", &w, &h);
		} else {
			fseek(f, 0, SEEK_SET);
			fscanf(f, "P6\n%d %d\n255", &w, &h);
		}

		fgetc(f);
		img->x0 = Dim[0];
		img->y0 = Dim[1];
		img->x1 =
			!Dim[0] ? (w - 1) * subsampling_dx + 1 : Dim[0] + (w -
																												 1) *
			subsampling_dx + 1;
		img->y1 =
			!Dim[1] ? (h - 1) * subsampling_dy + 1 : Dim[1] + (h -
																												 1) *
			subsampling_dy + 1;
		img->numcomps = 3;
		img->comps = (j2k_comp_t *) malloc(img->numcomps * sizeof(j2k_comp_t));
		for (i = 0; i < img->numcomps; i++) {
			img->comps[i].prec = 8;
			img->comps[i].bpp = 8;
			img->comps[i].sgnd = 0;
			img->comps[i].dx = subsampling_dx;
			img->comps[i].dy = subsampling_dy;
		}
		Compo0 = fopen("Compo0", "wb");
		if (!Compo0) {
			fprintf(stderr,
							"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
		}

		Compo1 = fopen("Compo1", "wb");
		if (!Compo1) {
			fprintf(stderr,
							"\033[0;33mFailed to open Compo1 for writing !\033[0;39m\n");
		}

		Compo2 = fopen("Compo2", "wb");
		if (!Compo2) {
			fprintf(stderr,
							"\033[0;33mFailed to open Compo2 for writing !\033[0;39m\n");
		}

		for (i = 0; i < w * h; i++) {
			unsigned char r, g, b;
			fread(&r, 1, 1, f);
			fread(&g, 1, 1, f);
			fread(&b, 1, 1, f);
			fwrite(&r, 1, 1, Compo0);
			fwrite(&g, 1, 1, Compo1);
			fwrite(&b, 1, 1, Compo2);
		}
		fclose(Compo0);
		fclose(Compo1);
		fclose(Compo2);
	} else {
		return 0;
	}
	fclose(f);
	return 1;
}
