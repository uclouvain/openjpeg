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

#ifndef OPJ_HAVE_LIBTIFF
# error OPJ_HAVE_LIBTIFF_NOT_DEFINED
#endif /* OPJ_HAVE_LIBTIFF */

#include <tiffio.h>
#include "openjpeg.h"
#include "convert.h"

/* -->> -->> -->> -->>
 
 TIFF IMAGE FORMAT
 
 <<-- <<-- <<-- <<-- */

int imagetotif(opj_image_t * image, const char *outfile)
{
	int width, height, imgsize;
	int bps,index,adjust, sgnd;
	int ushift, dshift, has_alpha, force16;
	TIFF *tif;
	tdata_t buf;
	tstrip_t strip;
	tsize_t strip_size;
	
	ushift = dshift = force16 = has_alpha = 0;
	bps = (int)image->comps[0].prec;
	
	if(bps > 8 && bps < 16)
	{
		ushift = 16 - bps; dshift = bps - ushift;
		bps = 16; force16 = 1;
	}
	
	if(bps != 8 && bps != 16)
	{
		fprintf(stderr,"imagetotif: Bits=%d, Only 8 and 16 bits implemented\n",
						bps);
		fprintf(stderr,"\tAborting\n");
		return 1;
	}
	tif = TIFFOpen(outfile, "wb");
	
	if (!tif)
	{
		fprintf(stderr, "imagetotif:failed to open %s for writing\n", outfile);
		return 1;
	}
	sgnd = (int)image->comps[0].sgnd;
	adjust = sgnd ? 1 << (image->comps[0].prec - 1) : 0;
	
	if(image->numcomps >= 3
		 && image->comps[0].dx == image->comps[1].dx
		 && image->comps[1].dx == image->comps[2].dx
		 && image->comps[0].dy == image->comps[1].dy
		 && image->comps[1].dy == image->comps[2].dy
		 && image->comps[0].prec == image->comps[1].prec
		 && image->comps[1].prec == image->comps[2].prec)
	{
		has_alpha = (image->numcomps == 4);
		
		width   = (int)image->comps[0].w;
		height  = (int)image->comps[0].h;
		imgsize = width * height ;
		
		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3 + has_alpha);
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps);
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
		strip_size = TIFFStripSize(tif);
		buf = _TIFFmalloc(strip_size);
		index=0;
		
		for(strip = 0; strip < TIFFNumberOfStrips(tif); strip++)
		{
			unsigned char *dat8;
			tsize_t i, ssize, last_i = 0;
			int step, restx;
			ssize = TIFFStripSize(tif);
			dat8 = (unsigned char*)buf;
			
			if(bps == 8)
			{
				step = 3 + has_alpha;
				restx = step - 1;
				
				for(i=0; i < ssize - restx; i += step)
				{
					int r, g, b, a = 0;
					
					if(index < imgsize)
					{
						r = image->comps[0].data[index];
						g = image->comps[1].data[index];
						b = image->comps[2].data[index];
						if(has_alpha) a = image->comps[3].data[index];
						
						if(sgnd)
						{
							r += adjust;
							g += adjust;
							b += adjust;
							if(has_alpha) a += adjust;
						}
						if(r > 255) r = 255; else if(r < 0) r = 0;
						dat8[i+0] = (unsigned char)r ;
						if(g > 255) g = 255; else if(g < 0) g = 0;
						dat8[i+1] = (unsigned char)g ;
						if(b > 255) b = 255; else if(b < 0) b = 0;
						dat8[i+2] = (unsigned char)b ;
						if(has_alpha)
						{
							if(a > 255) a = 255; else if(a < 0) a = 0;
							dat8[i+3] = (unsigned char)a;
						}
						
						index++;
						last_i = i + step;
					}
					else
						break;
				}/*for(i = 0;)*/
				
				if(last_i < ssize)
				{
					for(i = last_i; i < ssize; i += step)
					{
						int r, g, b, a = 0;
						
						if(index < imgsize)
						{
							r = image->comps[0].data[index];
							g = image->comps[1].data[index];
							b = image->comps[2].data[index];
							if(has_alpha) a = image->comps[3].data[index];
							
							if(sgnd)
							{
								r += adjust;
								g += adjust;
								b += adjust;
								if(has_alpha) a += adjust;
							}
							if(r > 255) r = 255; else if(r < 0) r = 0;
							if(g > 255) g = 255; else if(g < 0) g = 0;
							if(b > 255) b = 255; else if(b < 0) b = 0;
							
							dat8[i+0] = (unsigned char)r ;
							if(i+1 < ssize) dat8[i+1] = (unsigned char)g ;  else break;
							if(i+2 < ssize) dat8[i+2] = (unsigned char)b ;  else break;
							if(has_alpha)
							{
								if(a > 255) a = 255; else if(a < 0) a = 0;
								
								if(i+3 < ssize) dat8[i+3] = (unsigned char)a ;  else break;
							}
							index++;
						}
						else
							break;
					}/*for(i)*/
				}/*if(last_i < ssize)*/
				
			}	/*if(bps == 8)*/
			else
				if(bps == 16)
				{
					step = 6 + has_alpha + has_alpha;
					restx = step - 1;
					
					for(i = 0; i < ssize - restx ; i += step)
					{
						int r, g, b, a = 0;
						
						if(index < imgsize)
						{
							r = image->comps[0].data[index];
							g = image->comps[1].data[index];
							b = image->comps[2].data[index];
							if(has_alpha) a = image->comps[3].data[index];
							
							if(sgnd)
							{
								r += adjust;
								g += adjust;
								b += adjust;
								if(has_alpha) a += adjust;
							}
							if(force16)
							{
								r = (r<<ushift) + (r>>dshift);
								g = (g<<ushift) + (g>>dshift);
								b = (b<<ushift) + (b>>dshift);
								if(has_alpha) a = (a<<ushift) + (a>>dshift);
							}
							if(r > 65535) r = 65535; else if(r < 0) r = 0;
							if(g > 65535) g = 65535; else if(g < 0) g = 0;
							if(b > 65535) b = 65535; else if(b < 0) b = 0;
							
							dat8[i+0] =  (unsigned char)r;/*LSB*/
							dat8[i+1] = (unsigned char)(r >> 8);/*MSB*/
							dat8[i+2] =  (unsigned char)g;
							dat8[i+3] = (unsigned char)(g >> 8);
							dat8[i+4] =  (unsigned char)b;
							dat8[i+5] = (unsigned char)(b >> 8);
							if(has_alpha)
							{
								if(a > 65535) a = 65535; else if(a < 0) a = 0;
								dat8[i+6] =  (unsigned char)a;
								dat8[i+7] = (unsigned char)(a >> 8);
							}
							index++;
							last_i = i + step;
						}
						else
							break;
					}/*for(i = 0;)*/
					
					if(last_i < ssize)
					{
						for(i = last_i ; i < ssize ; i += step)
						{
							int r, g, b, a = 0;
							
							if(index < imgsize)
							{
								r = image->comps[0].data[index];
								g = image->comps[1].data[index];
								b = image->comps[2].data[index];
								if(has_alpha) a = image->comps[3].data[index];
								
								if(sgnd)
								{
									r += adjust;
									g += adjust;
									b += adjust;
									if(has_alpha) a += adjust;
								}
								if(force16)
								{
									r = (r<<ushift) + (r>>dshift);
									g = (g<<ushift) + (g>>dshift);
									b = (b<<ushift) + (b>>dshift);
									if(has_alpha) a = (a<<ushift) + (a>>dshift);
								}
								if(r > 65535) r = 65535; else if(r < 0) r = 0;
								if(g > 65535) g = 65535; else if(g < 0) g = 0;
								if(b > 65535) b = 65535; else if(b < 0) b = 0;
								
								dat8[i+0] = (unsigned char) r;/*LSB*/
								if(i+1 < ssize) dat8[i+1] = (unsigned char)(r >> 8);else break;/*MSB*/
								if(i+2 < ssize) dat8[i+2] = (unsigned char) g;      else break;
								if(i+3 < ssize) dat8[i+3] = (unsigned char)(g >> 8);else break;
								if(i+4 < ssize) dat8[i+4] = (unsigned char) b;      else break;
								if(i+5 < ssize) dat8[i+5] = (unsigned char)(b >> 8);else break;
								
								if(has_alpha)
								{
									if(a > 65535) a = 65535; else if(a < 0) a = 0;
									if(i+6 < ssize) dat8[i+6] = (unsigned char)a; else break;
									if(i+7 < ssize) dat8[i+7] = (unsigned char)(a >> 8); else break;
								}
								index++;
							}
							else
								break;
						}/*for(i)*/
					}/*if(last_i < ssize)*/
					
				}/*if(bps == 16)*/
			(void)TIFFWriteEncodedStrip(tif, strip, (void*)buf, strip_size);
		}/*for(strip = 0; )*/
		
		_TIFFfree((void*)buf);
		TIFFClose(tif);
		
		return 0;
	}/*RGB(A)*/
	
	if(image->numcomps == 1 /* GRAY */
		 || (   image->numcomps == 2 /* GRAY_ALPHA */
				 && image->comps[0].dx == image->comps[1].dx
				 && image->comps[0].dy == image->comps[1].dy
				 && image->comps[0].prec == image->comps[1].prec))
	{
		int step;
		
		has_alpha = (image->numcomps == 2);
		
		width   = (int)image->comps[0].w;
		height  = (int)image->comps[0].h;
		imgsize = width * height;
		
		/* Set tags */
		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1 + has_alpha);
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps);
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
		
		/* Get a buffer for the data */
		strip_size = TIFFStripSize(tif);
		buf = _TIFFmalloc(strip_size);
		index = 0;
		
		for(strip = 0; strip < TIFFNumberOfStrips(tif); strip++)
		{
			unsigned char *dat8;
			tsize_t i, ssize = TIFFStripSize(tif);
			dat8 = (unsigned char*)buf;
			
			if(bps == 8)
			{
				step = 1 + has_alpha;
				
				for(i=0; i < ssize; i += step)
				{
					if(index < imgsize)
					{
						int r, a = 0;
						
						r = image->comps[0].data[index];
						if(has_alpha) a = image->comps[1].data[index];
						
						if(sgnd)
						{
							r += adjust;
							if(has_alpha) a += adjust;
						}
						if(r > 255) r = 255; else if(r < 0) r = 0;
						dat8[i+0] = (unsigned char)r;
						
						if(has_alpha)
						{
							if(a > 255) a = 255; else if(a < 0) a = 0;
							dat8[i+1] = (unsigned char)a;
						}
						index++;
					}
					else
						break;
	   }/*for(i )*/
			}/*if(bps == 8*/
			else
				if(bps == 16)
				{
					step = 2 + has_alpha + has_alpha;
					
					for(i=0; i < ssize; i += step)
					{
						if(index < imgsize)
						{
							int r, a = 0;
							
							r = image->comps[0].data[index];
							if(has_alpha) a = image->comps[1].data[index];
							
							if(sgnd)
							{
								r += adjust;
								if(has_alpha) a += adjust;
							}
							if(force16)
							{
								r = (r<<ushift) + (r>>dshift);
								if(has_alpha) a = (a<<ushift) + (a>>dshift);
							}
							if(r > 65535) r = 65535; else if(r < 0) r = 0;
							dat8[i+0] = (unsigned char)r;/*LSB*/
							dat8[i+1] = (unsigned char)(r >> 8);/*MSB*/
							if(has_alpha)
							{
								if(a > 65535) a = 65535; else if(a < 0) a = 0;
								dat8[i+2] = (unsigned char)a;
								dat8[i+3] = (unsigned char)(a >> 8);
							}
							index++;
						}/*if(index < imgsize)*/
						else
							break;
					}/*for(i )*/
				}
			(void)TIFFWriteEncodedStrip(tif, strip, (void*)buf, strip_size);
		}/*for(strip*/
		
		_TIFFfree(buf);
		TIFFClose(tif);
		
		return 0;
	}
	
	TIFFClose(tif);
	
	fprintf(stderr,"imagetotif: Bad color format.\n"
					"\tOnly RGB(A) and GRAY(A) has been implemented\n");
	fprintf(stderr,"\tFOUND: numcomps(%d)\n\tAborting\n",
					image->numcomps);
	
	return 1;
}/* imagetotif() */

typedef void (* tif_Xto32s)(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length);

static void tif_1uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & -(OPJ_SIZE_T)8U); i+=8U) {
		OPJ_UINT8 val = *pSrc++;
		pDst[i+0] =  val >> 7;
		pDst[i+1] = (val >> 6) & 0x1U;
		pDst[i+2] = (val >> 5) & 0x1U;
		pDst[i+3] = (val >> 4) & 0x1U;
		pDst[i+4] = (val >> 3) & 0x1U;
		pDst[i+5] = (val >> 2) & 0x1U;
		pDst[i+6] = (val >> 1) & 0x1U;
		pDst[i+7] = val & 0x1U;
	}
	if (length & 7U) {
		OPJ_UINT8 val = *pSrc++;
		length = length & 7U;
		pDst[i+0] =  val >> 7;
		
		if (length > 1U) {
			pDst[i+1] = (val >> 6) & 0x1U;
			if (length > 2U) {
				pDst[i+2] = (val >> 5) & 0x1U;
				if (length > 3U) {
					pDst[i+3] = (val >> 4) & 0x1U;
					if (length > 4U) {
						pDst[i+4] = (val >> 3) & 0x1U;
						if (length > 5U) {
							pDst[i+5] = (val >> 2) & 0x1U;
							if (length > 6U) {
								pDst[i+6] = (val >> 1) & 0x1U;
							}
						}
					}
				}
			}
		}
	}
}
static void tif_2uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & -(OPJ_SIZE_T)4U); i+=4U) {
		OPJ_UINT8 val = *pSrc++;
		pDst[i+0] =  val >> 6;
		pDst[i+1] = (val >> 4) & 0x3U;
		pDst[i+2] = (val >> 2) & 0x3U;
		pDst[i+3] = val & 0x3U;
	}
	if (length & 3U) {
		OPJ_UINT8 val = *pSrc++;
		length = length & 3U;
		pDst[i+0] =  val >> 6;
		
		if (length > 1U) {
			pDst[i+1] = (val >> 4) & 0x3U;
			if (length > 2U) {
				pDst[i+2] = (val >> 2) & 0x3U;
				
			}
		}
	}
}
static void tif_4uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & -(OPJ_SIZE_T)2U); i+=2U) {
		OPJ_UINT8 val = *pSrc++;
		pDst[i+0] = val >> 4;
		pDst[i+1] = val & 0xFU;
	}
	if (length & 1U) {
		OPJ_UINT8 val = *pSrc++;
		pDst[i+0] = val >> 4;
	}
}
static void tif_6uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & -(OPJ_SIZE_T)4U); i+=4U) {
		OPJ_UINT8 val0 = *pSrc++;
		OPJ_UINT8 val1 = *pSrc++;
		OPJ_UINT8 val2 = *pSrc++;
		pDst[i+0] = val0 >> 2;
		pDst[i+1] = ((val0 & 0x3U) << 4) | (val1 >> 4);
		pDst[i+2] = ((val1 & 0xFU) << 2) | (val2 >> 6);
		pDst[i+3] = val2 & 0x3FU;
		
	}
	if (length & 3U) {
		OPJ_UINT8 val0 = *pSrc++;
		length = length & 3U;
		pDst[i+0] =  val0 >> 2;
		
		if (length > 1U) {
			OPJ_UINT8 val1 = *pSrc++;
			pDst[i+1] = ((val0 & 0x3U) << 4) | (val1 >> 4);
			if (length > 2U) {
				OPJ_UINT8 val2 = *pSrc++;
				pDst[i+2] = ((val1 & 0xFU) << 2) | (val2 >> 6);
			}
		}
	}
}
static void tif_8uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < length; ++i) {
		pDst[i] = pSrc[i];
	}
}
static void tif_10uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & -(OPJ_SIZE_T)4U); i+=4U) {
		OPJ_INT32 val0 = *pSrc++;
		OPJ_INT32 val1 = *pSrc++;
		OPJ_INT32 val2 = *pSrc++;
		OPJ_INT32 val3 = *pSrc++;
		OPJ_INT32 val4 = *pSrc++;
		
		pDst[i+0] = (val0 << 2) | (val1 >> 6);
		pDst[i+1] = ((val1 & 0x3FU) << 4) | (val2 >> 4);
		pDst[i+2] = ((val2 & 0xFU) << 6) | (val3 >> 2);
		pDst[i+3] = ((val3 & 0x3U) << 8) | val4;
		
	}
	if (length & 3U) {
		OPJ_INT32 val0 = *pSrc++;
		OPJ_INT32 val1 = *pSrc++;
		length = length & 3U;
		pDst[i+0] = (val0 << 2) | (val1 >> 6);
		
		if (length > 1U) {
			OPJ_INT32 val2 = *pSrc++;
			pDst[i+1] = ((val1 & 0x3FU) << 4) | (val2 >> 4);
			if (length > 2U) {
				OPJ_INT32 val3 = *pSrc++;
				pDst[i+2] = ((val2 & 0xFU) << 6) | (val3 >> 2);
			}
		}
	}
}
static void tif_12uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & -(OPJ_SIZE_T)2U); i+=2U) {
		OPJ_INT32 val0 = *pSrc++;
		OPJ_INT32 val1 = *pSrc++;
		OPJ_INT32 val2 = *pSrc++;

		pDst[i+0] = (val0 << 4) | (val1 >> 4);
		pDst[i+1] = ((val1 & 0xFU) << 8) | val2;
	}
	if (length & 1U) {
		OPJ_INT32 val0 = *pSrc++;
		OPJ_INT32 val1 = *pSrc++;
		pDst[i+0] = (val0 << 4) | (val1 >> 4);
	}
}
static void tif_14uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & -(OPJ_SIZE_T)4U); i+=4U) {
		OPJ_INT32 val0 = *pSrc++;
		OPJ_INT32 val1 = *pSrc++;
		OPJ_INT32 val2 = *pSrc++;
		OPJ_INT32 val3 = *pSrc++;
		OPJ_INT32 val4 = *pSrc++;
		OPJ_INT32 val5 = *pSrc++;
		OPJ_INT32 val6 = *pSrc++;
		
		pDst[i+0] = (val0 << 6) | (val1 >> 2);
		pDst[i+1] = ((val1 & 0x3U) << 12) | (val2 << 4) | (val3 >> 4);
		pDst[i+2] = ((val3 & 0xFU) << 10) | (val4 << 2) | (val5 >> 6);
		pDst[i+3] = ((val5 & 0x3FU) << 8) | val6;
		
	}
	if (length & 3U) {
		OPJ_INT32 val0 = *pSrc++;
		OPJ_INT32 val1 = *pSrc++;
		length = length & 3U;
		pDst[i+0] = (val0 << 6) | (val1 >> 2);
		
		if (length > 1U) {
			OPJ_INT32 val2 = *pSrc++;
			OPJ_INT32 val3 = *pSrc++;
			pDst[i+1] = ((val1 & 0x3U) << 12) | (val2 << 4) | (val3 >> 4);
			if (length > 2U) {
				OPJ_INT32 val4 = *pSrc++;
				OPJ_INT32 val5 = *pSrc++;
				pDst[i+2] = ((val3 & 0xFU) << 10) | (val4 << 2) | (val5 >> 6);
			}
		}
	}
}
#if 0
static void tif_16uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < length; i++) {
		OPJ_INT32 val0 = *pSrc++;
		OPJ_INT32 val1 = *pSrc++;
#ifdef OPJ_BIG_ENDIAN
		pDst[i] = (val0 << 8) | val1;
#else
		pDst[i] = (val1 << 8) | val0;
#endif
	}
}
#else
/* seems that libtiff decodes this to machine endianness */
static void tif_16uto32s(const OPJ_UINT16* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < length; i++) {
		pDst[i] = pSrc[i];
	}
}
#endif

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


/*
 * libtiff/tif_getimage.c : 1,2,4,8,16 bitspersample accepted
 * CINEMA                 : 12 bit precision
 */
opj_image_t* tiftoimage(const char *filename, opj_cparameters_t *parameters)
{
	int subsampling_dx = parameters->subsampling_dx;
	int subsampling_dy = parameters->subsampling_dy;
	TIFF *tif;
	tdata_t buf;
	tstrip_t strip;
	tsize_t strip_size;
	int j, currentPlane, numcomps = 0, w, h;
	OPJ_COLOR_SPACE color_space;
	opj_image_cmptparm_t cmptparm[4]; /* RGBA */
	opj_image_t *image = NULL;
	int has_alpha = 0;
	unsigned short tiBps, tiPhoto, tiSf, tiSpp, tiPC;
	unsigned int tiWidth, tiHeight;
	OPJ_BOOL is_cinema = OPJ_IS_CINEMA(parameters->rsiz);
	tif_Xto32s cvtTifTo32s = NULL;
	convert_32s_CXPX cvtCxToPx = NULL;
	OPJ_INT32* buffer32s = NULL;
	OPJ_INT32* planes[4];
	OPJ_SIZE_T rowStride;
	
	tif = TIFFOpen(filename, "r");
	
	if(!tif)
	{
		fprintf(stderr, "tiftoimage:Failed to open %s for reading\n", filename);
		return 0;
	}
	tiBps = tiPhoto = tiSf = tiSpp = tiPC = 0;
	tiWidth = tiHeight = 0;
	
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &tiWidth);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &tiHeight);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &tiBps);
	TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &tiSf);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &tiSpp);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &tiPhoto);
	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &tiPC);
	w= (int)tiWidth;
	h= (int)tiHeight;
	
	if((tiBps > 16U) || ((tiBps != 1U) && (tiBps & 1U))) tiBps = 0U;
	if(tiPhoto != PHOTOMETRIC_MINISBLACK && tiPhoto != PHOTOMETRIC_RGB) tiPhoto = 0;
	
	if( !tiBps || !tiPhoto)
	{
		if( !tiBps)
			fprintf(stderr,"tiftoimage: Bits=%d, Only 1, 2, 4, 6, 8, 10, 12, 14 and 16 bits implemented\n",tiBps);
		else
			if( !tiPhoto)
				fprintf(stderr,"tiftoimage: Bad color format %d.\n\tOnly RGB(A)"
								" and GRAY(A) has been implemented\n",(int) tiPhoto);
		
		fprintf(stderr,"\tAborting\n");
		TIFFClose(tif);
		return NULL;
	}
	
	switch (tiBps) {
		case 1:
			cvtTifTo32s = tif_1uto32s;
			break;
		case 2:
			cvtTifTo32s = tif_2uto32s;
			break;
		case 4:
			cvtTifTo32s = tif_4uto32s;
			break;
		case 6:
			cvtTifTo32s = tif_6uto32s;
			break;
		case 8:
			cvtTifTo32s = tif_8uto32s;
			break;
		case 10:
			cvtTifTo32s = tif_10uto32s;
			break;
		case 12:
			cvtTifTo32s = tif_12uto32s;
			break;
		case 14:
			cvtTifTo32s = tif_14uto32s;
			break;
		case 16:
			cvtTifTo32s = (tif_Xto32s)tif_16uto32s;
			break;
		default:
			/* never here */
			break;
	}
	
	{/* From: tiff-4.0.x/libtiff/tif_getimage.c : */
		uint16* sampleinfo;
		uint16 extrasamples;
		
		TIFFGetFieldDefaulted(tif, TIFFTAG_EXTRASAMPLES,
													&extrasamples, &sampleinfo);
		
		if(extrasamples >= 1)
		{
			switch(sampleinfo[0])
			{
				case EXTRASAMPLE_UNSPECIFIED:
					/* Workaround for some images without correct info about alpha channel
					 */
					if(tiSpp > 3)
						has_alpha = 1;
					break;
					
				case EXTRASAMPLE_ASSOCALPHA: /* data pre-multiplied */
				case EXTRASAMPLE_UNASSALPHA: /* data not pre-multiplied */
					has_alpha = 1;
					break;
			}
		}
		else /* extrasamples == 0 */
			if(tiSpp == 4 || tiSpp == 2) has_alpha = 1;
	}
	
	/* initialize image components
	 */
	memset(&cmptparm[0], 0, 4 * sizeof(opj_image_cmptparm_t));
	
	if ((tiPhoto == PHOTOMETRIC_RGB) && (is_cinema) && (tiBps != 12U)) {
		fprintf(stdout,"WARNING:\n"
						"Input image bitdepth is %d bits\n"
						"TIF conversion has automatically rescaled to 12-bits\n"
						"to comply with cinema profiles.\n",
						tiBps);
	} else {
		is_cinema = 0U;
	}
	
	if(tiPhoto == PHOTOMETRIC_RGB) /* RGB(A) */
	{
		numcomps = 3 + has_alpha;
		color_space = OPJ_CLRSPC_SRGB;
	}
	else if (tiPhoto == PHOTOMETRIC_MINISBLACK) /* GRAY(A) */
	{
		numcomps = 1 + has_alpha;
		color_space = OPJ_CLRSPC_GRAY;
	}
	
	switch (numcomps) {
		case 1:
			cvtCxToPx = convert_32s_C1P1;
			break;
		case 2:
			cvtCxToPx = convert_32s_C2P2;
			break;
		case 3:
			cvtCxToPx = convert_32s_C3P3;
			break;
		case 4:
			cvtCxToPx = convert_32s_C4P4;
			break;
  default:
			/* never here */
			break;
	}
	if (tiPC == PLANARCONFIG_SEPARATE) {
		cvtCxToPx = convert_32s_C1P1; /* override */
		tiSpp = 1U; /* consider only one sample per plane */
	}

	for(j = 0; j < numcomps; j++)
	{
		cmptparm[j].prec = tiBps;
		cmptparm[j].bpp = tiBps;
		cmptparm[j].dx = (OPJ_UINT32)subsampling_dx;
		cmptparm[j].dy = (OPJ_UINT32)subsampling_dy;
		cmptparm[j].w = (OPJ_UINT32)w;
		cmptparm[j].h = (OPJ_UINT32)h;
	}
		
	image = opj_image_create((OPJ_UINT32)numcomps, &cmptparm[0], color_space);
	if(!image)
	{
		TIFFClose(tif);
		return NULL;
	}
	/* set image offset and reference grid */
	image->x0 = (OPJ_UINT32)parameters->image_offset_x0;
	image->y0 = (OPJ_UINT32)parameters->image_offset_y0;
	image->x1 =	!image->x0 ? (OPJ_UINT32)(w - 1) * (OPJ_UINT32)subsampling_dx + 1 :
	image->x0 + (OPJ_UINT32)(w - 1) * (OPJ_UINT32)subsampling_dx + 1;
	image->y1 =	!image->y0 ? (OPJ_UINT32)(h - 1) * (OPJ_UINT32)subsampling_dy + 1 :
	image->y0 + (OPJ_UINT32)(h - 1) * (OPJ_UINT32)subsampling_dy + 1;

	for(j = 0; j < numcomps; j++)
	{
		planes[j] = image->comps[j].data;
		if (has_alpha) {
			planes[j] = image->comps[j].data;
		}
	}
		
	strip_size = TIFFStripSize(tif);
	
	buf = _TIFFmalloc(strip_size);
	if (buf == NULL) {
		TIFFClose(tif);
		opj_image_destroy(image);
		return NULL;
	}
	rowStride = ((OPJ_SIZE_T)w * tiSpp * tiBps + 7U) / 8U;
	buffer32s = malloc((OPJ_SIZE_T)w * tiSpp * sizeof(OPJ_INT32));
	if (buffer32s == NULL) {
		_TIFFfree(buf);
		TIFFClose(tif);
		opj_image_destroy(image);
		return NULL;
	}
	
	strip = 0;
	currentPlane = 0;
	do
	{
		planes[0] = image->comps[currentPlane].data; /* to manage planar data */
		h= (int)tiHeight;
		/* Read the Image components */
		for(; (h > 0) && (strip < TIFFNumberOfStrips(tif)); strip++)
		{
				const OPJ_UINT8 *dat8;
				tsize_t ssize;
				
				ssize = TIFFReadEncodedStrip(tif, strip, buf, strip_size);
				dat8 = (const OPJ_UINT8*)buf;
				
				while (ssize >= rowStride) {
					cvtTifTo32s(dat8, buffer32s, w * tiSpp);
					cvtCxToPx(buffer32s, planes, w);
					planes[0] += w;
					planes[1] += w;
					planes[2] += w;
					planes[3] += w;
					dat8  += rowStride;
					ssize -= rowStride;
					h--;
				}
		}
		currentPlane++;
	} while ((tiPC == PLANARCONFIG_SEPARATE) && (currentPlane < numcomps));
	
	free(buffer32s);
	_TIFFfree(buf);
	TIFFClose(tif);
	
	if (is_cinema) {
		for (j=0; j < numcomps; ++j) {
			scale_component(&(image->comps[j]), 12);
		}
		
	}
	return image;

}/* tiftoimage() */

