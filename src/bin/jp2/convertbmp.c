/*
 * The copyright in this software is being made available under the 2-clauses 
 * BSD License, included below. This software may be subject to other third 
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2002-2014, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2014, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux 
 * Copyright (c) 2003-2014, Antonin Descampe
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
#include "opj_apps_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "openjpeg.h"
#include "convert.h"

typedef struct {
	OPJ_UINT16 bfType;      /* 'BM' for Bitmap (19776) */
	OPJ_UINT32 bfSize;      /* Size of the file        */
	OPJ_UINT16 bfReserved1; /* Reserved : 0            */
	OPJ_UINT16 bfReserved2; /* Reserved : 0            */
	OPJ_UINT32 bfOffBits;   /* Offset                  */
} OPJ_BITMAPFILEHEADER;

typedef struct {
	OPJ_UINT32 biSize;          /* Size of the structure in bytes */
	OPJ_UINT32 biWidth;         /* Width of the image in pixels */
	OPJ_UINT32 biHeight;        /* Heigth of the image in pixels */
	OPJ_UINT16 biPlanes;        /* 1 */
	OPJ_UINT16 biBitCount;      /* Number of color bits by pixels */
	OPJ_UINT32 biCompression;   /* Type of encoding 0: none 1: RLE8 2: RLE4 */
	OPJ_UINT32 biSizeImage;     /* Size of the image in bytes */
	OPJ_UINT32 biXpelsPerMeter; /* Horizontal (X) resolution in pixels/meter */
	OPJ_UINT32 biYpelsPerMeter; /* Vertical (Y) resolution in pixels/meter */
	OPJ_UINT32 biClrUsed;       /* Number of color used in the image (0: ALL) */
	OPJ_UINT32 biClrImportant;  /* Number of important color (0: ALL) */
} OPJ_BITMAPINFOHEADER;

static void opj_applyLUT8u_8u32s_C1R(
	OPJ_UINT8 const* pSrc, OPJ_INT32 srcStride,
	OPJ_INT32* pDst, OPJ_INT32 dstStride,
	OPJ_UINT8 const* pLUT,
	OPJ_UINT32 width, OPJ_UINT32 height)
{
	OPJ_UINT32 y;
	
	for (y = height; y != 0U; --y) {
		OPJ_UINT32 x;
		
		for(x = 0; x < width; x++)
		{
			pDst[x] = pLUT[pSrc[x]];
		}
		pSrc += srcStride;
		pDst += dstStride;
	}
}

static void opj_applyLUT8u_8u32s_C1P3R(
	OPJ_UINT8 const* pSrc, OPJ_INT32 srcStride,
	OPJ_INT32* const* pDst, OPJ_INT32 const* pDstStride,
	OPJ_UINT8 const* const* pLUT,
	OPJ_UINT32 width, OPJ_UINT32 height)
{
	OPJ_UINT32 y;
	OPJ_INT32* pR = pDst[0];
	OPJ_INT32* pG = pDst[1];
	OPJ_INT32* pB = pDst[2];
	OPJ_UINT8 const* pLUT_R = pLUT[0];
	OPJ_UINT8 const* pLUT_G = pLUT[1];
	OPJ_UINT8 const* pLUT_B = pLUT[2];
	
	for (y = height; y != 0U; --y) {
		OPJ_UINT32 x;
		
		for(x = 0; x < width; x++)
		{
			OPJ_UINT8 idx = pSrc[x];
			pR[x] = pLUT_R[idx];
			pG[x] = pLUT_G[idx];
			pB[x] = pLUT_B[idx];
		}
		pSrc += srcStride;
		pR += pDstStride[0];
		pG += pDstStride[1];
		pB += pDstStride[2];
	}
}

static opj_image_t* bmp24toimage(FILE *IN, const OPJ_BITMAPFILEHEADER* File_h, const OPJ_BITMAPINFOHEADER* Info_h, const opj_cparameters_t *parameters)
{
	opj_image_cmptparm_t cmptparm[3];	/* maximum of 3 components */
	opj_image_t * image = NULL;
	int i, index;
	OPJ_UINT32 width, height, stride;
	OPJ_UINT32 x, y;
	OPJ_UINT8 *pData = NULL;
	const OPJ_UINT8 *pSrc = NULL;

	width  = Info_h->biWidth;
	height = Info_h->biHeight;
	
	/* initialize image components */
	memset(&cmptparm[0], 0, 3 * sizeof(opj_image_cmptparm_t));
	for(i = 0; i < 3; i++)
	{
		cmptparm[i].prec = 8;
		cmptparm[i].bpp  = 8;
		cmptparm[i].sgnd = 0;
		cmptparm[i].dx   = (OPJ_UINT32)parameters->subsampling_dx;
		cmptparm[i].dy   = (OPJ_UINT32)parameters->subsampling_dy;
		cmptparm[i].w    = (OPJ_UINT32)width;
		cmptparm[i].h    = (OPJ_UINT32)height;
	}
	/* create the image */
	image = opj_image_create(3U, &cmptparm[0], OPJ_CLRSPC_SRGB);
	if(!image)
	{
		return NULL;
	}
	
	/* set image offset and reference grid */
	image->x0 = (OPJ_UINT32)parameters->image_offset_x0;
	image->y0 = (OPJ_UINT32)parameters->image_offset_y0;
	image->x1 =	image->x0 + (OPJ_UINT32)(width  - 1U) * (OPJ_UINT32)parameters->subsampling_dx + 1U;
	image->y1 = image->y0 + (OPJ_UINT32)(height - 1U) * (OPJ_UINT32)parameters->subsampling_dy + 1U;
	
	/* set image data */
	
	/* Place the cursor at the beginning of the image information */
	fseek(IN, 0, SEEK_SET);
	fseek(IN, (long)File_h->bfOffBits, SEEK_SET);
	
	stride = width * 3U;
	/* line is 32 bits aligned */
	if (stride & 3U) {
		stride += 4U - (stride & 3U);
	}
	
	pData = (OPJ_UINT8 *)malloc(stride * height * sizeof(OPJ_UINT8));
	
	if ( fread(pData, sizeof(OPJ_UINT8), stride * height, IN) != (stride * height) )
	{
		free(pData);
		opj_image_destroy(image);
		fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
		return NULL;
	}
	
	index = 0;
	pSrc = pData + (height - 1U) * stride;
	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{
			image->comps[0].data[index] = pSrc[3*x+2];	/* R */
			image->comps[1].data[index] = pSrc[3*x+1];	/* G */
			image->comps[2].data[index] = pSrc[3*x+0];	/* B */
			index++;
		}
		pSrc -= stride;
	}
	free(pData);
	return image;
}

static opj_image_t* bmp8toimage(FILE *IN, const OPJ_BITMAPFILEHEADER* File_h, const OPJ_BITMAPINFOHEADER* Info_h, const opj_cparameters_t *parameters)
{
	OPJ_UINT8 lut_R[256], lut_G[256], lut_B[256];
	opj_image_cmptparm_t cmptparm[3];	/* maximum of 3 components */
	opj_image_t * image = NULL;
	OPJ_SIZE_T index;
	OPJ_UINT32 i, palette_len;
	OPJ_UINT32 width, height, stride;
	OPJ_UINT8 *pData = NULL;
	const OPJ_UINT8 *pSrc = NULL;
	OPJ_UINT32 numcmpts = 1U; /* grayscale by default */
	
	width  = Info_h->biWidth;
	height = Info_h->biHeight;
	
	/* initialize */
	memset(&cmptparm[0], 0, sizeof(cmptparm));
	memset(&lut_R[0], 0, sizeof(lut_R));
	memset(&lut_G[0], 0, sizeof(lut_G));
	memset(&lut_B[0], 0, sizeof(lut_B));
	
	palette_len = Info_h->biClrUsed;
	if ((palette_len == 0U) || (palette_len > 256U)) {
		palette_len = 256U;
	}
	
	/* Load palette */
	{
		OPJ_UINT8 has_color = 0U;
		for (i = 0; i < palette_len; i++) {
			lut_B[i] = (OPJ_UINT8)getc(IN);
			lut_G[i] = (OPJ_UINT8)getc(IN);
			lut_R[i] = (OPJ_UINT8)getc(IN);
			getc(IN); /* padding */
			has_color |= lut_B[i] ^ (lut_G[i] | lut_R[i]);
		}
		if(has_color) {
			numcmpts = 3;
		}
	}
	
	
	for(i = 0; i < numcmpts; i++)
	{
		cmptparm[i].prec = 8;
		cmptparm[i].bpp  = 8;
		cmptparm[i].sgnd = 0;
		cmptparm[i].dx   = (OPJ_UINT32)parameters->subsampling_dx;
		cmptparm[i].dy   = (OPJ_UINT32)parameters->subsampling_dy;
		cmptparm[i].w    = width;
		cmptparm[i].h    = height;
	}
	/* create the image */
	image = opj_image_create(numcmpts, &cmptparm[0], (numcmpts == 1U) ? OPJ_CLRSPC_GRAY : OPJ_CLRSPC_SRGB);
	if(!image)
	{
		return NULL;
	}
	
	/* set image offset and reference grid */
	image->x0 = (OPJ_UINT32)parameters->image_offset_x0;
	image->y0 = (OPJ_UINT32)parameters->image_offset_y0;
	image->x1 =	image->x0 + (width  - 1U) * (OPJ_UINT32)parameters->subsampling_dx + 1U;
	image->y1 = image->y0 + (height - 1U) * (OPJ_UINT32)parameters->subsampling_dy + 1U;
	
	/* set image data */
	
	/* Place the cursor at the beginning of the image information */
	fseek(IN, 0, SEEK_SET);
	fseek(IN, (long)File_h->bfOffBits, SEEK_SET);
	
	stride = width;
	/* line is 32 bits aligned */
	if (stride & 3U) {
		stride += 4U - (stride & 3U);
	}
	
	pData = (OPJ_UINT8 *)malloc(stride * height * sizeof(OPJ_UINT8));
	
	if ( fread(pData, sizeof(OPJ_UINT8), stride * height, IN) != (stride * height) )
	{
		free(pData);
		opj_image_destroy(image);
		fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
		return NULL;
	}
	
	index = 0;
	pSrc = pData + (height - 1U) * stride;
	if (numcmpts == 1U) {
		opj_applyLUT8u_8u32s_C1R(pSrc, -(OPJ_INT32)stride, image->comps[0].data, (OPJ_INT32)width, lut_R, width, height);
	}
	else {
		OPJ_INT32* pDst[] = { image->comps[0].data, image->comps[1].data, image->comps[2].data };
		OPJ_INT32  pDstStride[] = { (OPJ_INT32)width, (OPJ_INT32)width, (OPJ_INT32)width };
		OPJ_UINT8 const* pLUT[] = { lut_R, lut_G, lut_B };
		
		opj_applyLUT8u_8u32s_C1P3R(pSrc, -(OPJ_INT32)stride, pDst, pDstStride, pLUT, width, height);
	}
	
	free(pData);
	return image;
}

static opj_image_t* bmprle8toimage(FILE *IN, const OPJ_BITMAPFILEHEADER* File_h, const OPJ_BITMAPINFOHEADER* Info_h, const opj_cparameters_t *parameters)
{
	OPJ_UINT8 lut_R[256], lut_G[256], lut_B[256];
	opj_image_cmptparm_t cmptparm[3];	/* maximum of 3 components */
	opj_image_t * image = NULL;
	OPJ_SIZE_T index;
	OPJ_UINT32 i, palette_len;
	OPJ_UINT32 x, y, width, height;
	OPJ_UINT8 *pData = NULL, *pix;
	const OPJ_UINT8 *beyond;
	OPJ_UINT32 numcmpts = 1U; /* grayscale by default */
	
	width  = Info_h->biWidth;
	height = Info_h->biHeight;
	
	/* initialize */
	memset(&cmptparm[0], 0, sizeof(cmptparm));
	memset(&lut_R[0], 0, sizeof(lut_R));
	memset(&lut_G[0], 0, sizeof(lut_G));
	memset(&lut_B[0], 0, sizeof(lut_B));
	
	palette_len = Info_h->biClrUsed;
	if ((palette_len == 0U) || (palette_len > 256U)) {
		palette_len = 256U;
	}
	
	/* Load palette */
	{
		OPJ_UINT8 has_color = 0U;
		for (i = 0; i < palette_len; i++) {
			lut_B[i] = (OPJ_UINT8)getc(IN);
			lut_G[i] = (OPJ_UINT8)getc(IN);
			lut_R[i] = (OPJ_UINT8)getc(IN);
			getc(IN); /* padding */
			has_color |= lut_B[i] ^ (lut_G[i] | lut_R[i]);
		}
		if(has_color) {
			numcmpts = 3;
		}
	}
	
	
	for(i = 0; i < numcmpts; i++)
	{
		cmptparm[i].prec = 8;
		cmptparm[i].bpp  = 8;
		cmptparm[i].sgnd = 0;
		cmptparm[i].dx   = (OPJ_UINT32)parameters->subsampling_dx;
		cmptparm[i].dy   = (OPJ_UINT32)parameters->subsampling_dy;
		cmptparm[i].w    = width;
		cmptparm[i].h    = height;
	}
	/* create the image */
	image = opj_image_create(numcmpts, &cmptparm[0], (numcmpts == 1U) ? OPJ_CLRSPC_GRAY : OPJ_CLRSPC_SRGB);
	if(!image)
	{
		return NULL;
	}
	
	/* set image offset and reference grid */
	image->x0 = (OPJ_UINT32)parameters->image_offset_x0;
	image->y0 = (OPJ_UINT32)parameters->image_offset_y0;
	image->x1 =	image->x0 + (width  - 1U) * (OPJ_UINT32)parameters->subsampling_dx + 1U;
	image->y1 = image->y0 + (height - 1U) * (OPJ_UINT32)parameters->subsampling_dy + 1U;
	
	/* set image data */
	
	/* Place the cursor at the beginning of the image information */
	fseek(IN, 0, SEEK_SET);
	fseek(IN, (long)File_h->bfOffBits, SEEK_SET);
	
	pData = (OPJ_UINT8 *) calloc(1, width * height * sizeof(OPJ_UINT8));
	beyond = pData + width * height;
	pix = (OPJ_UINT8 *)(beyond - width);
	x = y = 0U;
	while (y < height)
	{
		int c = getc(IN);
		
		if (c) {
			int j;
			OPJ_UINT8 c1 = (OPJ_UINT8)getc(IN);
			
			for (j = 0; (j < c) && (x < width) && ((OPJ_SIZE_T)pix < (OPJ_SIZE_T)beyond); j++, x++, pix++) {
				*pix = c1;
			}
		}
		else {
			c = getc(IN);
			if (c == 0x00) { /* EOL */
				x = 0;
				++y;
				pix = pData + (height - y - 1U) * width + x;
			}
			else if (c == 0x01) { /* EOP */
				break;
			}
			else if (c == 0x02) { /* MOVE by dxdy */
				c = getc(IN);
				x += (OPJ_UINT32)c;
				c = getc(IN);
				y += (OPJ_UINT32)c;
				pix = pData + (height - y - 1U) * width + x;
			}
			else /* 03 .. 255 */
			{
				int j;
				for (j = 0; (j < c) && (x < width) && ((OPJ_SIZE_T)pix < (OPJ_SIZE_T)beyond); j++, x++, pix++)
				{
					OPJ_UINT8 c1 = (OPJ_UINT8)getc(IN);
					*pix = c1;
				}
				if ((OPJ_UINT32)c & 1U) { /* skip padding byte */
					getc(IN);
				}
			}
		}
	}/* while() */
	
	index = 0;
	if (numcmpts == 1) {
		opj_applyLUT8u_8u32s_C1R(pData, (OPJ_INT32)width, image->comps[0].data, (OPJ_INT32)width, lut_R, width, height);
	}
	else {
		OPJ_INT32* pDst[] = { image->comps[0].data, image->comps[1].data, image->comps[2].data };
		OPJ_INT32  pDstStride[] = { (OPJ_INT32)width, (OPJ_INT32)width, (OPJ_INT32)width };
		OPJ_UINT8 const* pLUT[] = { lut_R, lut_G, lut_B };
		
		opj_applyLUT8u_8u32s_C1P3R(pData, (OPJ_INT32)width, pDst, pDstStride, pLUT, width, height);
	}
	
	free(pData);
	return image;
}

opj_image_t* bmptoimage(const char *filename, opj_cparameters_t *parameters)
{
	opj_image_t * image = NULL;

	FILE *IN;
	OPJ_BITMAPFILEHEADER File_h;
	OPJ_BITMAPINFOHEADER Info_h;

	IN = fopen(filename, "rb");
	if (!IN)
	{
		fprintf(stderr, "Failed to open %s for reading !!\n", filename);
		return NULL;
	}

	File_h.bfType = (OPJ_UINT16)getc(IN);
	File_h.bfType = (OPJ_UINT16)((getc(IN) << 8) + File_h.bfType);

	if (File_h.bfType != 19778) {
		fprintf(stderr,"Error, not a BMP file!\n");
		fclose(IN);
		return NULL;
	}
	
	/* FILE HEADER */
	/* ------------- */
	File_h.bfSize = (OPJ_UINT32)getc(IN);
	File_h.bfSize = (OPJ_UINT32)(getc(IN) << 8) + File_h.bfSize;
	File_h.bfSize = (OPJ_UINT32)(getc(IN) << 16) + File_h.bfSize;
	File_h.bfSize = (OPJ_UINT32)(getc(IN) << 24) + File_h.bfSize;
	
	File_h.bfReserved1 = (OPJ_UINT16)getc(IN);
	File_h.bfReserved1 = (OPJ_UINT16)((getc(IN) << 8) + File_h.bfReserved1);

	File_h.bfReserved2 = (OPJ_UINT16)getc(IN);
	File_h.bfReserved2 = (OPJ_UINT16)((getc(IN) << 8) + File_h.bfReserved2);

	File_h.bfOffBits = (OPJ_UINT32)getc(IN);
	File_h.bfOffBits = (OPJ_UINT32)(getc(IN) << 8) + File_h.bfOffBits;
	File_h.bfOffBits = (OPJ_UINT32)(getc(IN) << 16) + File_h.bfOffBits;
	File_h.bfOffBits = (OPJ_UINT32)(getc(IN) << 24) + File_h.bfOffBits;

	/* INFO HEADER */
	/* ------------- */

	Info_h.biSize = (OPJ_UINT32)getc(IN);
	Info_h.biSize = (OPJ_UINT32)(getc(IN) << 8) + Info_h.biSize;
	Info_h.biSize = (OPJ_UINT32)(getc(IN) << 16) + Info_h.biSize;
	Info_h.biSize = (OPJ_UINT32)(getc(IN) << 24) + Info_h.biSize;

	if(Info_h.biSize != 40) {
		fprintf(stderr,"Error, unknown BMP header size %d\n", Info_h.biSize);
		fclose(IN);
		return NULL;
	}
	
	Info_h.biWidth = (OPJ_UINT32)getc(IN);
	Info_h.biWidth = (OPJ_UINT32)(getc(IN) << 8) + Info_h.biWidth;
	Info_h.biWidth = (OPJ_UINT32)(getc(IN) << 16) + Info_h.biWidth;
	Info_h.biWidth = (OPJ_UINT32)(getc(IN) << 24) + Info_h.biWidth;

	Info_h.biHeight = (OPJ_UINT32)getc(IN);
	Info_h.biHeight = (OPJ_UINT32)(getc(IN) << 8) + Info_h.biHeight;
	Info_h.biHeight = (OPJ_UINT32)(getc(IN) << 16) + Info_h.biHeight;
	Info_h.biHeight = (OPJ_UINT32)(getc(IN) << 24) + Info_h.biHeight;

	Info_h.biPlanes = (OPJ_UINT16)getc(IN);
	Info_h.biPlanes = (OPJ_UINT16)((getc(IN) << 8) + Info_h.biPlanes);

	Info_h.biBitCount = (OPJ_UINT16)getc(IN);
	Info_h.biBitCount = (OPJ_UINT16)((getc(IN) << 8) + Info_h.biBitCount);

	Info_h.biCompression = (OPJ_UINT32)getc(IN);
	Info_h.biCompression = (OPJ_UINT32)(getc(IN) << 8) + Info_h.biCompression;
	Info_h.biCompression = (OPJ_UINT32)(getc(IN) << 16) + Info_h.biCompression;
	Info_h.biCompression = (OPJ_UINT32)(getc(IN) << 24) + Info_h.biCompression;

	Info_h.biSizeImage = (OPJ_UINT32)getc(IN);
	Info_h.biSizeImage = (OPJ_UINT32)(getc(IN) << 8) + Info_h.biSizeImage;
	Info_h.biSizeImage = (OPJ_UINT32)(getc(IN) << 16) + Info_h.biSizeImage;
	Info_h.biSizeImage = (OPJ_UINT32)(getc(IN) << 24) + Info_h.biSizeImage;

	Info_h.biXpelsPerMeter = (OPJ_UINT32)getc(IN);
	Info_h.biXpelsPerMeter = (OPJ_UINT32)(getc(IN) << 8) + Info_h.biXpelsPerMeter;
	Info_h.biXpelsPerMeter = (OPJ_UINT32)(getc(IN) << 16) + Info_h.biXpelsPerMeter;
	Info_h.biXpelsPerMeter = (OPJ_UINT32)(getc(IN) << 24) + Info_h.biXpelsPerMeter;

	Info_h.biYpelsPerMeter = (OPJ_UINT32)getc(IN);
	Info_h.biYpelsPerMeter = (OPJ_UINT32)(getc(IN) << 8) + Info_h.biYpelsPerMeter;
	Info_h.biYpelsPerMeter = (OPJ_UINT32)(getc(IN) << 16) + Info_h.biYpelsPerMeter;
	Info_h.biYpelsPerMeter = (OPJ_UINT32)(getc(IN) << 24) + Info_h.biYpelsPerMeter;

	Info_h.biClrUsed = (OPJ_UINT32)getc(IN);
	Info_h.biClrUsed = (OPJ_UINT32)(getc(IN) << 8) + Info_h.biClrUsed;
	Info_h.biClrUsed = (OPJ_UINT32)(getc(IN) << 16) + Info_h.biClrUsed;
	Info_h.biClrUsed = (OPJ_UINT32)(getc(IN) << 24) + Info_h.biClrUsed;

	Info_h.biClrImportant = (OPJ_UINT32)getc(IN);
	Info_h.biClrImportant = (OPJ_UINT32)(getc(IN) << 8) + Info_h.biClrImportant;
	Info_h.biClrImportant = (OPJ_UINT32)(getc(IN) << 16) + Info_h.biClrImportant;
	Info_h.biClrImportant = (OPJ_UINT32)(getc(IN) << 24) + Info_h.biClrImportant;
	
	/* Read the data */
	if (Info_h.biBitCount == 24) { /*RGB */
		image = bmp24toimage(IN, &File_h, &Info_h, parameters);
	}
	else if (Info_h.biBitCount == 8 && Info_h.biCompression == 0) { /* RGB 8bpp Indexed */
		image = bmp8toimage(IN, &File_h, &Info_h, parameters);
	}
	else if (Info_h.biBitCount == 8 && Info_h.biCompression == 1) { /*RLE8*/
		image = bmprle8toimage(IN, &File_h, &Info_h, parameters);
	}
	else {
		fprintf(stderr, "Other system than 24 bits/pixels or 8 bits (no RLE coding) is not yet implemented [%d]\n", Info_h.biBitCount);
	}
	fclose(IN);
	return image;
}

int imagetobmp(opj_image_t * image, const char *outfile) {
    int w, h;
    int i, pad;
    FILE *fdest = NULL;
    int adjustR, adjustG, adjustB;

    if (image->comps[0].prec < 8) {
        fprintf(stderr, "Unsupported number of components: %d\n", image->comps[0].prec);
        return 1;
    }
    if (image->numcomps >= 3 && image->comps[0].dx == image->comps[1].dx
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

        w = (int)image->comps[0].w;
        h = (int)image->comps[0].h;

        fprintf(fdest, "BM");

        /* FILE HEADER */
        /* ------------- */
        fprintf(fdest, "%c%c%c%c",
                (OPJ_UINT8) (h * w * 3 + 3 * h * (w % 2) + 54) & 0xff,
                (OPJ_UINT8) ((h * w * 3 + 3 * h * (w % 2) + 54)	>> 8) & 0xff,
                (OPJ_UINT8) ((h * w * 3 + 3 * h * (w % 2) + 54)	>> 16) & 0xff,
                (OPJ_UINT8) ((h * w * 3 + 3 * h * (w % 2) + 54)	>> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (54) & 0xff, ((54) >> 8) & 0xff,((54) >> 16) & 0xff, ((54) >> 24) & 0xff);

        /* INFO HEADER   */
        /* ------------- */
        fprintf(fdest, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,	((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (OPJ_UINT8) ((w) & 0xff),
                (OPJ_UINT8) ((w) >> 8) & 0xff,
                (OPJ_UINT8) ((w) >> 16) & 0xff,
                (OPJ_UINT8) ((w) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (OPJ_UINT8) ((h) & 0xff),
                (OPJ_UINT8) ((h) >> 8) & 0xff,
                (OPJ_UINT8) ((h) >> 16) & 0xff,
                (OPJ_UINT8) ((h) >> 24) & 0xff);
        fprintf(fdest, "%c%c", (1) & 0xff, ((1) >> 8) & 0xff);
        fprintf(fdest, "%c%c", (24) & 0xff, ((24) >> 8) & 0xff);
        fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (OPJ_UINT8) (3 * h * w + 3 * h * (w % 2)) & 0xff,
                (OPJ_UINT8) ((h * w * 3 + 3 * h * (w % 2)) >> 8) & 0xff,
                (OPJ_UINT8) ((h * w * 3 + 3 * h * (w % 2)) >> 16) & 0xff,
                (OPJ_UINT8) ((h * w * 3 + 3 * h * (w % 2)) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff, ((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);

        if (image->comps[0].prec > 8) {
            adjustR = (int)image->comps[0].prec - 8;
            printf("BMP CONVERSION: Truncating component 0 from %d bits to 8 bits\n", image->comps[0].prec);
        }
        else
            adjustR = 0;
        if (image->comps[1].prec > 8) {
            adjustG = (int)image->comps[1].prec - 8;
            printf("BMP CONVERSION: Truncating component 1 from %d bits to 8 bits\n", image->comps[1].prec);
        }
        else
            adjustG = 0;
        if (image->comps[2].prec > 8) {
            adjustB = (int)image->comps[2].prec - 8;
            printf("BMP CONVERSION: Truncating component 2 from %d bits to 8 bits\n", image->comps[2].prec);
        }
        else
            adjustB = 0;

        for (i = 0; i < w * h; i++) {
            OPJ_UINT8 rc, gc, bc;
            int r, g, b;

            r = image->comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
            r += (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
            r = ((r >> adjustR)+((r >> (adjustR-1))%2));
            if(r > 255) r = 255; else if(r < 0) r = 0;
            rc = (OPJ_UINT8)r;

            g = image->comps[1].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
            g += (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
            g = ((g >> adjustG)+((g >> (adjustG-1))%2));
            if(g > 255) g = 255; else if(g < 0) g = 0;
            gc = (OPJ_UINT8)g;

            b = image->comps[2].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
            b += (image->comps[2].sgnd ? 1 << (image->comps[2].prec - 1) : 0);
            b = ((b >> adjustB)+((b >> (adjustB-1))%2));
            if(b > 255) b = 255; else if(b < 0) b = 0;
            bc = (OPJ_UINT8)b;

            fprintf(fdest, "%c%c%c", bc, gc, rc);

            if ((i + 1) % w == 0) {
                for (pad = (3 * w) % 4 ? 4 - (3 * w) % 4 : 0; pad > 0; pad--)	/* ADD */
                    fprintf(fdest, "%c", 0);
            }
        }
        fclose(fdest);
    } else {			/* Gray-scale */

        /* -->> -->> -->> -->>
        8 bits non code (Gray scale)
        <<-- <<-- <<-- <<-- */

        fdest = fopen(outfile, "wb");
        w = (int)image->comps[0].w;
        h = (int)image->comps[0].h;

        fprintf(fdest, "BM");

        /* FILE HEADER */
        /* ------------- */
        fprintf(fdest, "%c%c%c%c", (OPJ_UINT8) (h * w + 54 + 1024 + h * (w % 2)) & 0xff,
                (OPJ_UINT8) ((h * w + 54 + 1024 + h * (w % 2)) >> 8) & 0xff,
                (OPJ_UINT8) ((h * w + 54 + 1024 + h * (w % 2)) >> 16) & 0xff,
                (OPJ_UINT8) ((h * w + 54 + 1024 + w * (w % 2)) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (54 + 1024) & 0xff, ((54 + 1024) >> 8) & 0xff,
                ((54 + 1024) >> 16) & 0xff,
                ((54 + 1024) >> 24) & 0xff);

        /* INFO HEADER */
        /* ------------- */
        fprintf(fdest, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,	((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (OPJ_UINT8) ((w) & 0xff),
                (OPJ_UINT8) ((w) >> 8) & 0xff,
                (OPJ_UINT8) ((w) >> 16) & 0xff,
                (OPJ_UINT8) ((w) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (OPJ_UINT8) ((h) & 0xff),
                (OPJ_UINT8) ((h) >> 8) & 0xff,
                (OPJ_UINT8) ((h) >> 16) & 0xff,
                (OPJ_UINT8) ((h) >> 24) & 0xff);
        fprintf(fdest, "%c%c", (1) & 0xff, ((1) >> 8) & 0xff);
        fprintf(fdest, "%c%c", (8) & 0xff, ((8) >> 8) & 0xff);
        fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (OPJ_UINT8) (h * w + h * (w % 2)) & 0xff,
                (OPJ_UINT8) ((h * w + h * (w % 2)) >> 8) &	0xff,
                (OPJ_UINT8) ((h * w + h * (w % 2)) >> 16) &	0xff,
                (OPJ_UINT8) ((h * w + h * (w % 2)) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff, ((256) >> 16) & 0xff, ((256) >> 24) & 0xff);
        fprintf(fdest, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff, ((256) >> 16) & 0xff, ((256) >> 24) & 0xff);

        if (image->comps[0].prec > 8) {
            adjustR = (int)image->comps[0].prec - 8;
            printf("BMP CONVERSION: Truncating component 0 from %d bits to 8 bits\n", image->comps[0].prec);
        }else
            adjustR = 0;

        for (i = 0; i < 256; i++) {
            fprintf(fdest, "%c%c%c%c", i, i, i, 0);
        }

        for (i = 0; i < w * h; i++) {
            int r;

            r = image->comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
            r += (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
            r = ((r >> adjustR)+((r >> (adjustR-1))%2));
            if(r > 255) r = 255; else if(r < 0) r = 0;

            fprintf(fdest, "%c", (OPJ_UINT8)r);

            if ((i + 1) % w == 0) {
                for (pad = w % 4 ? 4 - w % 4 : 0; pad > 0; pad--)	/* ADD */
                    fprintf(fdest, "%c", 0);
            }
        }
        fclose(fdest);
    }

    return 0;
}
