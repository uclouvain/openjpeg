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

#ifndef OPJ_HAVE_LIBTIFF
# error OPJ_HAVE_LIBTIFF_NOT_DEFINED
#endif /* OPJ_HAVE_LIBTIFF */

#include <tiffio.h>
#include "openjpeg.h"
#include "convert.h"

/* -->> -->> -->> -->>
 
 TIFF IMAGE FORMAT
 
 <<-- <<-- <<-- <<-- */

static void tif_32sto10u(const OPJ_INT32* pSrc, OPJ_BYTE* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & ~(OPJ_SIZE_T)3U); i+=4U) {
		OPJ_UINT32 src0 = (OPJ_UINT32)pSrc[i+0];
		OPJ_UINT32 src1 = (OPJ_UINT32)pSrc[i+1];
		OPJ_UINT32 src2 = (OPJ_UINT32)pSrc[i+2];
		OPJ_UINT32 src3 = (OPJ_UINT32)pSrc[i+3];
		
		*pDst++ = (OPJ_BYTE)(src0 >> 2);
		*pDst++ = (OPJ_BYTE)(((src0 & 0x3U) << 6) | (src1 >> 4));
		*pDst++ = (OPJ_BYTE)(((src1 & 0xFU) << 4) | (src2 >> 6));
		*pDst++ = (OPJ_BYTE)(((src2 & 0x3FU) << 2) | (src3 >> 8));
		*pDst++ = (OPJ_BYTE)(src3);
	}
	
	if (length & 3U) {
		OPJ_UINT32 src0 = (OPJ_UINT32)pSrc[i+0];
		OPJ_UINT32 src1 = 0U;
		OPJ_UINT32 src2 = 0U;
		length = length & 3U;
		
		if (length > 1U) {
			src1 = (OPJ_UINT32)pSrc[i+1];
			if (length > 2U) {
				src2 = (OPJ_UINT32)pSrc[i+2];
			}
		}
		*pDst++ = (OPJ_BYTE)(src0 >> 2);
		*pDst++ = (OPJ_BYTE)(((src0 & 0x3U) << 6) | (src1 >> 4));
		if (length > 1U) {
			*pDst++ = (OPJ_BYTE)(((src1 & 0xFU) << 4) | (src2 >> 6));
			if (length > 2U) {
				*pDst++ = (OPJ_BYTE)(((src2 & 0x3FU) << 2));
			}
		}
	}
}
static void tif_32sto12u(const OPJ_INT32* pSrc, OPJ_BYTE* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & ~(OPJ_SIZE_T)1U); i+=2U) {
		OPJ_UINT32 src0 = (OPJ_UINT32)pSrc[i+0];
		OPJ_UINT32 src1 = (OPJ_UINT32)pSrc[i+1];
		
		*pDst++ = (OPJ_BYTE)(src0 >> 4);
		*pDst++ = (OPJ_BYTE)(((src0 & 0xFU) << 4) | (src1 >> 8));
		*pDst++ = (OPJ_BYTE)(src1);
	}
	
	if (length & 1U) {
		OPJ_UINT32 src0 = (OPJ_UINT32)pSrc[i+0];
		*pDst++ = (OPJ_BYTE)(src0 >> 4);
		*pDst++ = (OPJ_BYTE)(((src0 & 0xFU) << 4));
	}
}
static void tif_32sto14u(const OPJ_INT32* pSrc, OPJ_BYTE* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & ~(OPJ_SIZE_T)3U); i+=4U) {
		OPJ_UINT32 src0 = (OPJ_UINT32)pSrc[i+0];
		OPJ_UINT32 src1 = (OPJ_UINT32)pSrc[i+1];
		OPJ_UINT32 src2 = (OPJ_UINT32)pSrc[i+2];
		OPJ_UINT32 src3 = (OPJ_UINT32)pSrc[i+3];
		
		*pDst++ = (OPJ_BYTE)(src0 >> 6);
		*pDst++ = (OPJ_BYTE)(((src0 & 0x3FU) << 2) | (src1 >> 12));
		*pDst++ = (OPJ_BYTE)(src1 >> 4);
		*pDst++ = (OPJ_BYTE)(((src1 & 0xFU) << 4) | (src2 >> 10));
		*pDst++ = (OPJ_BYTE)(src2 >> 2);
		*pDst++ = (OPJ_BYTE)(((src2 & 0x3U) << 6) | (src3 >> 8));
		*pDst++ = (OPJ_BYTE)(src3);
	}
	
	if (length & 3U) {
		OPJ_UINT32 src0 = (OPJ_UINT32)pSrc[i+0];
		OPJ_UINT32 src1 = 0U;
		OPJ_UINT32 src2 = 0U;
		length = length & 3U;
		
		if (length > 1U) {
			src1 = (OPJ_UINT32)pSrc[i+1];
			if (length > 2U) {
				src2 = (OPJ_UINT32)pSrc[i+2];
			}
		}
		*pDst++ = (OPJ_BYTE)(src0 >> 6);
		*pDst++ = (OPJ_BYTE)(((src0 & 0x3FU) << 2) | (src1 >> 12));
		if (length > 1U) {
			*pDst++ = (OPJ_BYTE)(src1 >> 4);
			*pDst++ = (OPJ_BYTE)(((src1 & 0xFU) << 4) | (src2 >> 10));
			if (length > 2U) {
				*pDst++ = (OPJ_BYTE)(src2 >> 2);
				*pDst++ = (OPJ_BYTE)(((src2 & 0x3U) << 6));
			}
		}
	}
}
static void tif_32sto16u(const OPJ_INT32* pSrc, OPJ_UINT16* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < length; ++i) {
		pDst[i] = (OPJ_UINT16)pSrc[i];
	}
}

int imagetotif(opj_image_t * image, const char *outfile)
{
	int width, height;
	int bps,adjust, sgnd;
	int tiPhoto;
	TIFF *tif;
	tdata_t buf;
	tsize_t strip_size;
	OPJ_UINT32 i, numcomps;
	OPJ_SIZE_T rowStride;
	OPJ_INT32* buffer32s = NULL;
	OPJ_INT32 const* planes[4];
	convert_32s_PXCX cvtPxToCx = NULL;
	convert_32sXXx_C1R cvt32sToTif = NULL;

	bps = (int)image->comps[0].prec;
	planes[0] = image->comps[0].data;
	
	numcomps = image->numcomps;
	
	if (image->color_space == OPJ_CLRSPC_CMYK) {
		if (numcomps < 4U) {
			fprintf(stderr,"imagetotif: CMYK images shall be composed of at least 4 planes.\n");
			fprintf(stderr,"\tAborting\n");
			return 1;
		}
		tiPhoto = PHOTOMETRIC_SEPARATED;
		if (numcomps > 4U) {
			numcomps = 4U; /* Alpha not supported */
		}
	}
	else if (numcomps > 2U) {
		tiPhoto = PHOTOMETRIC_RGB;
		if (numcomps > 4U) {
			numcomps = 4U;
		}
	} else {
		tiPhoto = PHOTOMETRIC_MINISBLACK;
	}
	for (i = 1U; i < numcomps; ++i) {
		if (image->comps[0].dx != image->comps[i].dx) {
			break;
		}
		if (image->comps[0].dy != image->comps[i].dy) {
			break;
		}
		if (image->comps[0].prec != image->comps[i].prec) {
			break;
		}
		if (image->comps[0].sgnd != image->comps[i].sgnd) {
			break;
		}
		planes[i] = image->comps[i].data;
	}
	if (i != numcomps) {
		fprintf(stderr,"imagetotif: All components shall have the same subsampling, same bit depth.\n");
		fprintf(stderr,"\tAborting\n");
		return 1;
	}
	
	if((bps > 16) || ((bps != 1) && (bps & 1))) bps = 0;
	if(bps == 0)
	{
		fprintf(stderr,"imagetotif: Bits=%d, Only 1, 2, 4, 6, 8, 10, 12, 14 and 16 bits implemented\n",bps);
		fprintf(stderr,"\tAborting\n");
		return 1;
	}
	tif = TIFFOpen(outfile, "wb");
	if (!tif)
	{
		fprintf(stderr, "imagetotif:failed to open %s for writing\n", outfile);
		return 1;
	}
	for (i = 0U; i < numcomps; ++i) {
		clip_component(&(image->comps[i]), image->comps[0].prec);
	}
	cvtPxToCx = convert_32s_PXCX_LUT[numcomps];
	switch (bps) {
		case 1:
		case 2:
		case 4:
		case 6:
		case 8:
			cvt32sToTif = convert_32sXXu_C1R_LUT[bps];
			break;
		case 10:
			cvt32sToTif = tif_32sto10u;
			break;
		case 12:
			cvt32sToTif = tif_32sto12u;
			break;
		case 14:
			cvt32sToTif = tif_32sto14u;
			break;
		case 16:
			cvt32sToTif = (convert_32sXXx_C1R)tif_32sto16u;
			break;
		default:
			/* never here */
			break;
	}
	sgnd = (int)image->comps[0].sgnd;
	adjust = sgnd ? 1 << (image->comps[0].prec - 1) : 0;
	width   = (int)image->comps[0].w;
	height  = (int)image->comps[0].h;
	
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, numcomps);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps);
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, tiPhoto);
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
	
	strip_size = TIFFStripSize(tif);
	rowStride = ((OPJ_SIZE_T)width * numcomps * (OPJ_SIZE_T)bps + 7U) / 8U;
	if (rowStride != (OPJ_SIZE_T)strip_size) {
		fprintf(stderr, "Invalid TIFF strip size\n");
		TIFFClose(tif);
		return 1;
	}
	buf = _TIFFmalloc(strip_size);
	if (buf == NULL) {
		TIFFClose(tif);
		return 1;
	}
	buffer32s = (OPJ_INT32 *)malloc((OPJ_SIZE_T)width * numcomps * sizeof(OPJ_INT32));
	if (buffer32s == NULL) {
		_TIFFfree(buf);
		TIFFClose(tif);
		return 1;
	}
	
	for (i = 0; i < image->comps[0].h; ++i) {
		cvtPxToCx(planes, buffer32s, (OPJ_SIZE_T)width, adjust);
		cvt32sToTif(buffer32s, (OPJ_BYTE *)buf, (OPJ_SIZE_T)width * numcomps);
		(void)TIFFWriteEncodedStrip(tif, i, (void*)buf, strip_size);
		planes[0] += width;
		planes[1] += width;
		planes[2] += width;
		planes[3] += width;
	}
	_TIFFfree((void*)buf);
	TIFFClose(tif);
	free(buffer32s);
		
	return 0;
}/* imagetotif() */

static void tif_10uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & ~(OPJ_SIZE_T)3U); i+=4U) {
		OPJ_UINT32 val0 = *pSrc++;
		OPJ_UINT32 val1 = *pSrc++;
		OPJ_UINT32 val2 = *pSrc++;
		OPJ_UINT32 val3 = *pSrc++;
		OPJ_UINT32 val4 = *pSrc++;
		
		pDst[i+0] = (OPJ_INT32)((val0 << 2) | (val1 >> 6));
		pDst[i+1] = (OPJ_INT32)(((val1 & 0x3FU) << 4) | (val2 >> 4));
		pDst[i+2] = (OPJ_INT32)(((val2 & 0xFU) << 6) | (val3 >> 2));
		pDst[i+3] = (OPJ_INT32)(((val3 & 0x3U) << 8) | val4);
		
	}
	if (length & 3U) {
		OPJ_UINT32 val0 = *pSrc++;
		OPJ_UINT32 val1 = *pSrc++;
		length = length & 3U;
		pDst[i+0] = (OPJ_INT32)((val0 << 2) | (val1 >> 6));
		
		if (length > 1U) {
			OPJ_UINT32 val2 = *pSrc++;
			pDst[i+1] = (OPJ_INT32)(((val1 & 0x3FU) << 4) | (val2 >> 4));
			if (length > 2U) {
				OPJ_UINT32 val3 = *pSrc++;
				pDst[i+2] = (OPJ_INT32)(((val2 & 0xFU) << 6) | (val3 >> 2));
			}
		}
	}
}
static void tif_12uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & ~(OPJ_SIZE_T)1U); i+=2U) {
		OPJ_UINT32 val0 = *pSrc++;
		OPJ_UINT32 val1 = *pSrc++;
		OPJ_UINT32 val2 = *pSrc++;

		pDst[i+0] = (OPJ_INT32)((val0 << 4) | (val1 >> 4));
		pDst[i+1] = (OPJ_INT32)(((val1 & 0xFU) << 8) | val2);
	}
	if (length & 1U) {
		OPJ_UINT32 val0 = *pSrc++;
		OPJ_UINT32 val1 = *pSrc++;
		pDst[i+0] = (OPJ_INT32)((val0 << 4) | (val1 >> 4));
	}
}
static void tif_14uto32s(const OPJ_BYTE* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < (length & ~(OPJ_SIZE_T)3U); i+=4U) {
		OPJ_UINT32 val0 = *pSrc++;
		OPJ_UINT32 val1 = *pSrc++;
		OPJ_UINT32 val2 = *pSrc++;
		OPJ_UINT32 val3 = *pSrc++;
		OPJ_UINT32 val4 = *pSrc++;
		OPJ_UINT32 val5 = *pSrc++;
		OPJ_UINT32 val6 = *pSrc++;
		
		pDst[i+0] = (OPJ_INT32)((val0 << 6) | (val1 >> 2));
		pDst[i+1] = (OPJ_INT32)(((val1 & 0x3U) << 12) | (val2 << 4) | (val3 >> 4));
		pDst[i+2] = (OPJ_INT32)(((val3 & 0xFU) << 10) | (val4 << 2) | (val5 >> 6));
		pDst[i+3] = (OPJ_INT32)(((val5 & 0x3FU) << 8) | val6);
		
	}
	if (length & 3U) {
		OPJ_UINT32 val0 = *pSrc++;
		OPJ_UINT32 val1 = *pSrc++;
		length = length & 3U;
		pDst[i+0] = (OPJ_INT32)((val0 << 6) | (val1 >> 2));
		
		if (length > 1U) {
			OPJ_UINT32 val2 = *pSrc++;
			OPJ_UINT32 val3 = *pSrc++;
			pDst[i+1] = (OPJ_INT32)(((val1 & 0x3U) << 12) | (val2 << 4) | (val3 >> 4));
			if (length > 2U) {
				OPJ_UINT32 val4 = *pSrc++;
				OPJ_UINT32 val5 = *pSrc++;
				pDst[i+2] = (OPJ_INT32)(((val3 & 0xFU) << 10) | (val4 << 2) | (val5 >> 6));
			}
		}
	}
}

/* seems that libtiff decodes this to machine endianness */
static void tif_16uto32s(const OPJ_UINT16* pSrc, OPJ_INT32* pDst, OPJ_SIZE_T length)
{
	OPJ_SIZE_T i;
	for (i = 0; i < length; i++) {
		pDst[i] = pSrc[i];
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
	OPJ_COLOR_SPACE color_space = OPJ_CLRSPC_UNKNOWN;
	opj_image_cmptparm_t cmptparm[4]; /* RGBA */
	opj_image_t *image = NULL;
	int has_alpha = 0;
	unsigned short tiBps, tiPhoto, tiSf, tiSpp, tiPC;
	unsigned int tiWidth, tiHeight;
	OPJ_BOOL is_cinema = OPJ_IS_CINEMA(parameters->rsiz);
	convert_XXx32s_C1R cvtTifTo32s = NULL;
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
	
	if((tiBps > 16U) || ((tiBps != 1U) && (tiBps & 1U))) {
		fprintf(stderr,"tiftoimage: Bits=%d, Only 1, 2, 4, 6, 8, 10, 12, 14 and 16 bits implemented\n",tiBps);
		fprintf(stderr,"\tAborting\n");
		TIFFClose(tif);
		return NULL;
	}
	if(tiPhoto != PHOTOMETRIC_MINISBLACK && tiPhoto != PHOTOMETRIC_RGB) {
		fprintf(stderr,"tiftoimage: Bad color format %d.\n\tOnly RGB(A) and GRAY(A) has been implemented\n",(int) tiPhoto);
		fprintf(stderr,"\tAborting\n");
		TIFFClose(tif);
		return NULL;
	}
	
	switch (tiBps) {
		case 1:
		case 2:
		case 4:
		case 6:
		case 8:
			cvtTifTo32s = convert_XXu32s_C1R_LUT[tiBps];
			break;
		/* others are specific to TIFF */
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
			cvtTifTo32s = (convert_XXx32s_C1R)tif_16uto32s;
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
	
	/* initialize image components */
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
	
	cvtCxToPx = convert_32s_CXPX_LUT[numcomps];
	if (tiPC == PLANARCONFIG_SEPARATE) {
		cvtCxToPx = convert_32s_CXPX_LUT[1]; /* override */
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
	}
	image->comps[numcomps - 1].alpha = (OPJ_UINT16)(1 - (numcomps & 1));
		
	strip_size = TIFFStripSize(tif);
	
	buf = _TIFFmalloc(strip_size);
	if (buf == NULL) {
		TIFFClose(tif);
		opj_image_destroy(image);
		return NULL;
	}
	rowStride = ((OPJ_SIZE_T)w * tiSpp * tiBps + 7U) / 8U;
	buffer32s = (OPJ_INT32 *)malloc((OPJ_SIZE_T)w * tiSpp * sizeof(OPJ_INT32));
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
				OPJ_SIZE_T ssize;
				
				ssize = (OPJ_SIZE_T)TIFFReadEncodedStrip(tif, strip, buf, strip_size);
				dat8 = (const OPJ_UINT8*)buf;
				
				while (ssize >= rowStride) {
					cvtTifTo32s(dat8, buffer32s, (OPJ_SIZE_T)w * tiSpp);
					cvtCxToPx(buffer32s, planes, (OPJ_SIZE_T)w);
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

