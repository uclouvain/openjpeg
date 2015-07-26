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
 * Copyright (c) 2015, Matthieu Darbois
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

#include <zlib.h>
#include <png.h>

#include "openjpeg.h"
#include "convert.h"

#define PNG_MAGIC "\x89PNG\x0d\x0a\x1a\x0a"
#define MAGIC_SIZE 8
/* PNG allows bits per sample: 1, 2, 4, 8, 16 */

typedef void (* convert_32s_CXPX)(const OPJ_INT32* pSrc, OPJ_INT32* const* pDst, OPJ_SIZE_T length);
static void convert_32s_C1P1(const OPJ_INT32* pSrc, OPJ_INT32* const* pDst, OPJ_SIZE_T length)
{
	memcpy(pDst[0], pSrc, length * sizeof(OPJ_INT32));
}
static void convert_32s_C2P2(const OPJ_INT32* pSrc, OPJ_INT32* const* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	OPJ_INT32* pDst0 = pDst[0];
	OPJ_INT32* pDst1 = pDst[1];
	
	for (i = 0; i < length; i++) {
		pDst0[i] = pSrc[2*i+0];
		pDst1[i] = pSrc[2*i+1];
	}
}
static void convert_32s_C3P3(const OPJ_INT32* pSrc, OPJ_INT32* const* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	OPJ_INT32* pDst0 = pDst[0];
	OPJ_INT32* pDst1 = pDst[1];
	OPJ_INT32* pDst2 = pDst[2];
	
	for (i = 0; i < length; i++) {
		pDst0[i] = pSrc[3*i+0];
		pDst1[i] = pSrc[3*i+1];
		pDst2[i] = pSrc[3*i+2];
	}
}
static void convert_32s_C4P4(const OPJ_INT32* pSrc, OPJ_INT32* const* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	OPJ_INT32* pDst0 = pDst[0];
	OPJ_INT32* pDst1 = pDst[1];
	OPJ_INT32* pDst2 = pDst[2];
	OPJ_INT32* pDst3 = pDst[3];
	
	for (i = 0; i < length; i++) {
		pDst0[i] = pSrc[4*i+0];
		pDst1[i] = pSrc[4*i+1];
		pDst2[i] = pSrc[4*i+2];
		pDst3[i] = pSrc[4*i+3];
	}
}

typedef void (* convert_XXx32s_C1R)(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length);
static void convert_1u32s_C1R(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & ~(OPJ_SIZE_T)7U); i+=8U) {
		OPJ_UINT32 val = *pSrc++;
		pDst[i+0] = (OPJ_INT32)( val >> 7);
		pDst[i+1] = (OPJ_INT32)((val >> 6) & 0x1U);
		pDst[i+2] = (OPJ_INT32)((val >> 5) & 0x1U);
		pDst[i+3] = (OPJ_INT32)((val >> 4) & 0x1U);
		pDst[i+4] = (OPJ_INT32)((val >> 3) & 0x1U);
		pDst[i+5] = (OPJ_INT32)((val >> 2) & 0x1U);
		pDst[i+6] = (OPJ_INT32)((val >> 1) & 0x1U);
		pDst[i+7] = (OPJ_INT32)(val & 0x1U);
	}
	if (length & 7U) {
		OPJ_UINT32 val = *pSrc++;
		length = length & 7U;
		pDst[i+0] = (OPJ_INT32)(val >> 7);
		
		if (length > 1U) {
			pDst[i+1] = (OPJ_INT32)((val >> 6) & 0x1U);
			if (length > 2U) {
				pDst[i+2] = (OPJ_INT32)((val >> 5) & 0x1U);
				if (length > 3U) {
					pDst[i+3] = (OPJ_INT32)((val >> 4) & 0x1U);
					if (length > 4U) {
						pDst[i+4] = (OPJ_INT32)((val >> 3) & 0x1U);
						if (length > 5U) {
							pDst[i+5] = (OPJ_INT32)((val >> 2) & 0x1U);
							if (length > 6U) {
								pDst[i+6] = (OPJ_INT32)((val >> 1) & 0x1U);
							}
						}
					}
				}
			}
		}
	}
}
static void convert_2u32s_C1R(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & ~(OPJ_SIZE_T)3U); i+=4U) {
		OPJ_UINT32 val = *pSrc++;
		pDst[i+0] = (OPJ_INT32)( val >> 6);
		pDst[i+1] = (OPJ_INT32)((val >> 4) & 0x3U);
		pDst[i+2] = (OPJ_INT32)((val >> 2) & 0x3U);
		pDst[i+3] = (OPJ_INT32)(val & 0x3U);
	}
	if (length & 3U) {
		OPJ_UINT32 val = *pSrc++;
		length = length & 3U;
		pDst[i+0] =  (OPJ_INT32)(val >> 6);
		
		if (length > 1U) {
			pDst[i+1] = (OPJ_INT32)((val >> 4) & 0x3U);
			if (length > 2U) {
				pDst[i+2] = (OPJ_INT32)((val >> 2) & 0x3U);
				
			}
		}
	}
}
static void convert_4u32s_C1R(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & ~(OPJ_SIZE_T)1U); i+=2U) {
		OPJ_UINT32 val = *pSrc++;
		pDst[i+0] = (OPJ_INT32)(val >> 4);
		pDst[i+1] = (OPJ_INT32)(val & 0xFU);
	}
	if (length & 1U) {
		OPJ_UINT8 val = *pSrc++;
		pDst[i+0] = (OPJ_INT32)(val >> 4);
	}
}
static void convert_8u32s_C1R(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < length; i++) {
		pDst[i] = pSrc[i];
	}
}
static void convert_16u32s_C1R(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < length; i++) {
		OPJ_INT32 val0 = *pSrc++;
		OPJ_INT32 val1 = *pSrc++;
		pDst[i] = val0 << 8 | val1;
	}
}

opj_image_t *pngtoimage(const char *read_idf, opj_cparameters_t * params)
{
	png_structp  png;
	png_infop    info;
	double gamma;
	int bit_depth, interlace_type,compression_type, filter_type;
	OPJ_UINT32 i;
	png_uint_32  width, height;
	int color_type;
	FILE *reader = NULL;
	OPJ_BYTE** rows = NULL;
	OPJ_INT32* row32s = NULL;
	/* j2k: */
	opj_image_t *image = NULL;
	opj_image_cmptparm_t cmptparm[4];
	OPJ_UINT32 nr_comp;
	OPJ_BYTE sigbuf[8];
	convert_XXx32s_C1R cvtXXTo32s = NULL;
	convert_32s_CXPX cvtCxToPx = NULL;
	OPJ_INT32* planes[4];
	
	if((reader = fopen(read_idf, "rb")) == NULL)
	{
		fprintf(stderr,"pngtoimage: can not open %s\n",read_idf);
		return NULL;
	}
	
	if(fread(sigbuf, 1, MAGIC_SIZE, reader) != MAGIC_SIZE
		 || memcmp(sigbuf, PNG_MAGIC, MAGIC_SIZE) != 0)
	{
		fprintf(stderr,"pngtoimage: %s is no valid PNG file\n",read_idf);
		goto fin;
	}
	
	if((png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
																	 NULL, NULL, NULL)) == NULL)
		goto fin;
	if((info = png_create_info_struct(png)) == NULL)
		goto fin;
	
	if(setjmp(png_jmpbuf(png)))
		goto fin;
	
	png_init_io(png, reader);
	png_set_sig_bytes(png, MAGIC_SIZE);
	
	png_read_info(png, info);
	
	if(png_get_IHDR(png, info, &width, &height,
									&bit_depth, &color_type, &interlace_type,
									&compression_type, &filter_type) == 0)
		goto fin;
	
	/* png_set_expand():
	 * expand paletted images to RGB, expand grayscale images of
	 * less than 8-bit depth to 8-bit depth, and expand tRNS chunks
	 * to alpha channels.
	 */
	if(color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_expand(png);
	}
	
	if(png_get_valid(png, info, PNG_INFO_tRNS)) {
		png_set_expand(png);
	}
	/* We might wan't to expand background */
	/*
	if(png_get_valid(png, info, PNG_INFO_bKGD)) {
		png_color_16p bgnd;
		png_get_bKGD(png, info, &bgnd);
		png_set_background(png, bgnd, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	}
	*/
	
	if( !png_get_gAMA(png, info, &gamma))
		gamma = 1.0;
	
	/* we're not displaying but converting, screen gamma == 1.0 */
	png_set_gamma(png, 1.0, gamma);
	
	png_read_update_info(png, info);
	
	color_type = png_get_color_type(png, info);
	
	switch (color_type) {
		case PNG_COLOR_TYPE_GRAY:
			nr_comp = 1;
			cvtCxToPx = convert_32s_C1P1;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			nr_comp = 2;
			cvtCxToPx = convert_32s_C2P2;
			break;
		case PNG_COLOR_TYPE_RGB:
			nr_comp = 3;
			cvtCxToPx = convert_32s_C3P3;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			nr_comp = 4;
			cvtCxToPx = convert_32s_C4P4;
			break;
		default:
			fprintf(stderr,"pngtoimage: colortype %d is not supported\n", color_type);
			goto fin;
	}
	bit_depth = png_get_bit_depth(png, info);
	
	switch (bit_depth) {
		case 1:
			cvtXXTo32s = convert_1u32s_C1R;
			break;
		case 2:
			cvtXXTo32s = convert_2u32s_C1R;
			break;
		case 4:
			cvtXXTo32s = convert_4u32s_C1R;
			break;
		case 8:
			cvtXXTo32s = convert_8u32s_C1R;
			break;
		case 16:
			cvtXXTo32s = convert_16u32s_C1R;
			break;
		default:
			fprintf(stderr,"pngtoimage: bit depth %d is not supported\n", bit_depth);
			goto fin;
	}

	
	rows = (OPJ_BYTE**)calloc(height+1, sizeof(OPJ_BYTE*));
	for(i = 0; i < height; ++i)
		rows[i] = (OPJ_BYTE*)malloc(png_get_rowbytes(png,info));
	
	png_read_image(png, rows);
	
	/* Create image */
	memset(cmptparm, 0, sizeof(cmptparm));
	for(i = 0; i < nr_comp; ++i)
	{
		cmptparm[i].prec = (OPJ_UINT32)bit_depth;
		/* bits_per_pixel: 8 or 16 */
		cmptparm[i].bpp = (OPJ_UINT32)bit_depth;
		cmptparm[i].sgnd = 0;
		cmptparm[i].dx = (OPJ_UINT32)params->subsampling_dx;
		cmptparm[i].dy = (OPJ_UINT32)params->subsampling_dy;
		cmptparm[i].w = (OPJ_UINT32)width;
		cmptparm[i].h = (OPJ_UINT32)height;
	}
	
	image = opj_image_create(nr_comp, &cmptparm[0], (nr_comp > 2U) ? OPJ_CLRSPC_SRGB : OPJ_CLRSPC_GRAY);
	if(image == NULL) goto fin;
	image->x0 = (OPJ_UINT32)params->image_offset_x0;
	image->y0 = (OPJ_UINT32)params->image_offset_y0;
	image->x1 = (OPJ_UINT32)(image->x0 + (width  - 1) * (OPJ_UINT32)params->subsampling_dx + 1 + image->x0);
	image->y1 = (OPJ_UINT32)(image->y0 + (height - 1) * (OPJ_UINT32)params->subsampling_dy + 1 + image->y0);
	
	row32s = malloc((size_t)width * nr_comp * sizeof(OPJ_INT32));
	if(row32s == NULL) goto fin;
	
	/* Set alpha channel */
	image->comps[nr_comp-1U].alpha = 1U - (nr_comp & 1U);
	
	for(i = 0; i < nr_comp; i++)
	{
		planes[i] = image->comps[i].data;
	}
	
	for(i = 0; i < height; ++i)
	{
		cvtXXTo32s(rows[i], row32s, (OPJ_SIZE_T)width * nr_comp);
		cvtCxToPx(row32s, planes, width);
		planes[0] += width;
		planes[1] += width;
		planes[2] += width;
		planes[3] += width;
	}
fin:
	if(rows)
	{
		for(i = 0; i < height; ++i)
			free(rows[i]);
		free(rows);
	}
	if (row32s) {
		free(row32s);
	}
	if(png)
		png_destroy_read_struct(&png, &info, NULL);
	
	fclose(reader);
	
	return image;
	
}/* pngtoimage() */

int imagetopng(opj_image_t * image, const char *write_idf)
{
	FILE *writer;
	png_structp png;
	png_infop info;
	int *red, *green, *blue, *alpha;
	unsigned char *row_buf, *d;
	int has_alpha, width, height, nr_comp, color_type;
	int adjustR, adjustG, adjustB, adjustA, x, y, fails;
	int prec, ushift, dshift, is16, force16, force8;
	unsigned short mask = 0xffff;
	png_color_8 sig_bit;
	
	is16 = force16 = force8 = ushift = dshift = 0; fails = 1;
	prec = (int)image->comps[0].prec;
	nr_comp = (int)image->numcomps;
	
	if(prec > 8 && prec < 16)
	{
		ushift = 16 - prec; dshift = prec - ushift;
		prec = 16; force16 = 1;
	}
	else
		if(prec < 8 && nr_comp > 1)/* GRAY_ALPHA, RGB, RGB_ALPHA */
		{
			ushift = 8 - prec; dshift = 8 - ushift;
			prec = 8; force8 = 1;
		}
	
	if(prec != 1 && prec != 2 && prec != 4 && prec != 8 && prec != 16)
	{
		fprintf(stderr,"imagetopng: can not create %s"
						"\n\twrong bit_depth %d\n", write_idf, prec);
		return fails;
	}
	writer = fopen(write_idf, "wb");
	
	if(writer == NULL) return fails;
	
	info = NULL; has_alpha = 0;
	
	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also check that
	 * the library version is compatible with the one used at compile time,
	 * in case we are using dynamically linked libraries.  REQUIRED.
	 */
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
																NULL, NULL, NULL);
	/*png_voidp user_error_ptr, user_error_fn, user_warning_fn); */
	
	if(png == NULL) goto fin;
	
	/* Allocate/initialize the image information data.  REQUIRED
	 */
	info = png_create_info_struct(png);
	
	if(info == NULL) goto fin;
	
	/* Set error handling.  REQUIRED if you are not supplying your own
	 * error handling functions in the png_create_write_struct() call.
	 */
	if(setjmp(png_jmpbuf(png))) goto fin;
	
	/* I/O initialization functions is REQUIRED
	 */
	png_init_io(png, writer);
	
	/* Set the image information here.  Width and height are up to 2^31,
	 * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
	 * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
	 * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
	 * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
	 * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
	 * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE.
	 * REQUIRED
	 *
	 * ERRORS:
	 *
	 * color_type == PNG_COLOR_TYPE_PALETTE && bit_depth > 8
	 * color_type == PNG_COLOR_TYPE_RGB && bit_depth < 8
	 * color_type == PNG_COLOR_TYPE_GRAY_ALPHA && bit_depth < 8
	 * color_type == PNG_COLOR_TYPE_RGB_ALPHA) && bit_depth < 8
	 *
	 */
	png_set_compression_level(png, Z_BEST_COMPRESSION);
	
	if(prec == 16) mask = 0xffff;
	else
		if(prec == 8) mask = 0x00ff;
		else
			if(prec == 4) mask = 0x000f;
			else
				if(prec == 2) mask = 0x0003;
				else
					if(prec == 1) mask = 0x0001;
	
	if(nr_comp >= 3
		 && image->comps[0].dx == image->comps[1].dx
		 && image->comps[1].dx == image->comps[2].dx
		 && image->comps[0].dy == image->comps[1].dy
		 && image->comps[1].dy == image->comps[2].dy
		 && image->comps[0].prec == image->comps[1].prec
		 && image->comps[1].prec == image->comps[2].prec)
	{
		int v;
		
		has_alpha = (nr_comp > 3);
		
		is16 = (prec == 16);
		
		width = (int)image->comps[0].w;
		height = (int)image->comps[0].h;
		
		red = image->comps[0].data;
		green = image->comps[1].data;
		blue = image->comps[2].data;
		
		sig_bit.red = sig_bit.green = sig_bit.blue = (png_byte)prec;
		
		if(has_alpha)
		{
			sig_bit.alpha = (png_byte)prec;
			alpha = image->comps[3].data;
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
			adjustA = (image->comps[3].sgnd ? 1 << (image->comps[3].prec - 1) : 0);
		}
		else
		{
			sig_bit.alpha = 0; alpha = NULL;
			color_type = PNG_COLOR_TYPE_RGB;
			adjustA = 0;
		}
		png_set_sBIT(png, info, &sig_bit);
		
		png_set_IHDR(png, info, (png_uint_32)width, (png_uint_32)height, prec,
								 color_type,
								 PNG_INTERLACE_NONE,
								 PNG_COMPRESSION_TYPE_BASE,  PNG_FILTER_TYPE_BASE);
		
		png_set_gamma(png, 2.2, 1./2.2);
		png_set_sRGB(png, info, PNG_sRGB_INTENT_PERCEPTUAL);
		/*=============================*/
		png_write_info(png, info);
		/*=============================*/
		if(prec < 8)
		{
			png_set_packing(png);
		}
		/*
		printf("%s:%d:sgnd(%d,%d,%d) w(%d) h(%d) alpha(%d)\n",__FILE__,__LINE__,
		image->comps[0].sgnd,
		image->comps[1].sgnd,image->comps[2].sgnd,width,height,has_alpha);
		*/
		
		adjustR = (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
		adjustG = (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
		adjustB = (image->comps[2].sgnd ? 1 << (image->comps[2].prec - 1) : 0);
		
		row_buf = (unsigned char*)malloc((size_t)width * (size_t)nr_comp * 2);
		
		for(y = 0; y < height; ++y)
		{
			d = row_buf;
			
			for(x = 0; x < width; ++x)
			{
				if(is16)
				{
					v = *red + adjustR; ++red;
					if(v > 65535) v = 65535; else if(v < 0) v = 0;
					
					if(force16) { v = (v<<ushift) + (v>>dshift); }
					
					*d++ = (unsigned char)(v>>8); *d++ = (unsigned char)v;
					
					v = *green + adjustG; ++green;
					if(v > 65535) v = 65535; else if(v < 0) v = 0;
					
					if(force16) { v = (v<<ushift) + (v>>dshift); }
					
					*d++ = (unsigned char)(v>>8); *d++ = (unsigned char)v;
					
					v =  *blue + adjustB; ++blue;
					if(v > 65535) v = 65535; else if(v < 0) v = 0;
					
					if(force16) { v = (v<<ushift) + (v>>dshift); }
					
					*d++ = (unsigned char)(v>>8); *d++ = (unsigned char)v;
					
					if(has_alpha)
					{
						v = *alpha + adjustA; ++alpha;
						if(v > 65535) v = 65535; else if(v < 0) v = 0;
						
						if(force16) { v = (v<<ushift) + (v>>dshift); }
						
						*d++ = (unsigned char)(v>>8); *d++ = (unsigned char)v;
					}
					continue;
				}/* if(is16) */
				
				v = *red + adjustR; ++red;
				if(v > 255) v = 255; else if(v < 0) v = 0;
				
				if(force8) { v = (v<<ushift) + (v>>dshift); }
				
				*d++ = (unsigned char)(v & mask);
				
				v = *green + adjustG; ++green;
				if(v > 255) v = 255; else if(v < 0) v = 0;
				
				if(force8) { v = (v<<ushift) + (v>>dshift); }
				
				*d++ = (unsigned char)(v & mask);
				
				v = *blue + adjustB; ++blue;
				if(v > 255) v = 255; else if(v < 0) v = 0;
				
				if(force8) { v = (v<<ushift) + (v>>dshift); }
				
				*d++ = (unsigned char)(v & mask);
				
				if(has_alpha)
				{
					v = *alpha + adjustA; ++alpha;
					if(v > 255) v = 255; else if(v < 0) v = 0;
					
					if(force8) { v = (v<<ushift) + (v>>dshift); }
					
					*d++ = (unsigned char)(v & mask);
				}
			}	/* for(x) */
			
			png_write_row(png, row_buf);
			
		}	/* for(y) */
		free(row_buf);
		
	}/* nr_comp >= 3 */
	else
		if(nr_comp == 1 /* GRAY */
			 || (   nr_comp == 2 /* GRAY_ALPHA */
					 && image->comps[0].dx == image->comps[1].dx
					 && image->comps[0].dy == image->comps[1].dy
					 && image->comps[0].prec == image->comps[1].prec))
		{
			int v;
			
			red = image->comps[0].data;
			
			sig_bit.gray = (png_byte)prec;
			sig_bit.red = sig_bit.green = sig_bit.blue = sig_bit.alpha = 0;
			alpha = NULL; adjustA = 0;
			color_type = PNG_COLOR_TYPE_GRAY;
			
			if(nr_comp == 2)
			{
				has_alpha = 1; sig_bit.alpha = (png_byte)prec;
				alpha = image->comps[1].data;
				color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
				adjustA = (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
			}
			width = (int)image->comps[0].w;
			height = (int)image->comps[0].h;
			
			png_set_IHDR(png, info, (png_uint_32)width, (png_uint_32)height, sig_bit.gray,
									 color_type,
									 PNG_INTERLACE_NONE,
									 PNG_COMPRESSION_TYPE_BASE,  PNG_FILTER_TYPE_BASE);
			
			png_set_sBIT(png, info, &sig_bit);
			
			png_set_gamma(png, 2.2, 1./2.2);
			png_set_sRGB(png, info, PNG_sRGB_INTENT_PERCEPTUAL);
			/*=============================*/
			png_write_info(png, info);
			/*=============================*/
			adjustR = (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
			
			if(prec < 8)
			{
				png_set_packing(png);
			}
			
			if(prec > 8)
			{
				row_buf = (unsigned char*)
				malloc((size_t)width * (size_t)nr_comp * sizeof(unsigned short));
				
				for(y = 0; y < height; ++y)
				{
					d = row_buf;
					
					for(x = 0; x < width; ++x)
					{
						v = *red + adjustR; ++red;
						if(v > 65535) v = 65535; else if(v < 0) v = 0;
						
						if(force16) { v = (v<<ushift) + (v>>dshift); }
						
						*d++ = (unsigned char)(v>>8); *d++ = (unsigned char)v;
						
						if(has_alpha)
						{
							v = *alpha++;
							if(v > 65535) v = 65535; else if(v < 0) v = 0;
							
							if(force16) { v = (v<<ushift) + (v>>dshift); }
							
							*d++ = (unsigned char)(v>>8); *d++ = (unsigned char)v;
						}
					}/* for(x) */
					png_write_row(png, row_buf);
					
				}	/* for(y) */
				free(row_buf);
			}
			else /* prec <= 8 */
			{
				row_buf = (unsigned char*)calloc((size_t)width, (size_t)nr_comp * 2);
				
				for(y = 0; y < height; ++y)
				{
					d = row_buf;
					
					for(x = 0; x < width; ++x)
					{
						v = *red + adjustR; ++red;
						if(v > 255) v = 255; else if(v < 0) v = 0;
						
						if(force8) { v = (v<<ushift) + (v>>dshift); }
						
						*d++ = (unsigned char)(v & mask);
						
						if(has_alpha)
						{
							v = *alpha + adjustA; ++alpha;
							if(v > 255) v = 255; else if(v < 0) v = 0;
							
							if(force8) { v = (v<<ushift) + (v>>dshift); }
							
							*d++ = (unsigned char)(v & mask);
						}
					}/* for(x) */
					
					png_write_row(png, row_buf);
					
				}	/* for(y) */
				free(row_buf);
			}
		}
		else
		{
			fprintf(stderr,"imagetopng: can not create %s\n",write_idf);
			goto fin;
		}
	png_write_end(png, info);
	
	fails = 0;
	
fin:
	
	if(png)
	{
		png_destroy_write_struct(&png, &info);
	}
	fclose(writer);
	
	if(fails) remove(write_idf);
	
	return fails;
}/* imagetopng() */

