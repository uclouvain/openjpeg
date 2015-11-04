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
#include "opj_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_LIBPNG
#include <zlib.h>
#include <png.h>
#endif /* HAVE_LIBPNG */

#include "openjpeg.h"
#include "convert.h"

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#if defined(_WIN32)
typedef   signed __int8   int8_t;
typedef unsigned __int8   uint8_t;
typedef   signed __int16  int16_t;
typedef unsigned __int16  uint16_t;
typedef   signed __int32  int32_t;
typedef unsigned __int32  uint32_t;
typedef   signed __int64  int64_t;
typedef unsigned __int64  uint64_t;
#else
#error unsupported platform
#endif
#endif

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

/* Component precision scaling */
void clip_component(opj_image_comp_t* component, int precision)
{
	size_t i, len;
	unsigned int umax = (unsigned int)((int)-1);
	
	len = (size_t)component->w * (size_t)component->h;
	if (precision < 32) {
		umax = (1U << precision) - 1U;
	}
	
	if (component->sgnd) {
		int* l_data = component->data;
		int max = (int)(umax / 2U);
		int min = -max - 1;
		for (i = 0U; i < len; ++i) {
			if (l_data[i] > max) {
				l_data[i] = max;
			} else if (l_data[i] < min) {
				l_data[i] = min;
			}
		}
	} else {
		unsigned int* l_data = (unsigned int*)component->data;
		for (i = 0U; i < len; ++i) {
			if (l_data[i] > umax) {
				l_data[i] = umax;
			}
		}
	}
	component->prec = precision;
}

/* Component precision scaling */
static void scale_component_up(opj_image_comp_t* component, int precision)
{
	size_t i, len;
	
	len = (size_t)component->w * (size_t)component->h;
	if (component->sgnd) {
		int64_t newMax = (int64_t)(1U << (precision - 1));
		int64_t oldMax = (int64_t)(1U << (component->prec - 1));
		int* l_data = component->data;
		for (i = 0; i < len; ++i) {
			l_data[i] = (int)(((int64_t)l_data[i] * newMax) / oldMax);
		}
	} else {
		uint64_t newMax = (uint64_t)((1U << precision) - 1U);
		uint64_t oldMax = (uint64_t)((1U << component->prec) - 1U);
		unsigned int* l_data = (unsigned int*)component->data;
		for (i = 0; i < len; ++i) {
			l_data[i] = (unsigned int)(((uint64_t)l_data[i] * newMax) / oldMax);
		}
	}
	component->prec = precision;
	component->bpp = precision;
}
void scale_component(opj_image_comp_t* component, int precision)
{
	int shift;
	size_t i, len;
	
	if (component->prec == precision) {
		return;
	}
	if (component->prec < precision) {
		scale_component_up(component, precision);
		return;
	}
	shift = (int)(component->prec - precision);
	len = (size_t)component->w * (size_t)component->h;
	if (component->sgnd) {
		int* l_data = component->data;
		for (i = 0U; i < len; ++i) {
			l_data[i] >>= shift;
		}
	} else {
		unsigned int* l_data = (unsigned int*)component->data;
		for (i = 0U; i < len; ++i) {
			l_data[i] >>= shift;
		}
	}
	component->bpp = precision;
	component->prec = precision;
}


/* planar / interleaved conversions */
/* used by PNG/TIFF */
static void convert_32s_C1P1(const int32_t* pSrc, int32_t* const* pDst, size_t length)
{
	memcpy(pDst[0], pSrc, length * sizeof(int32_t));
}
static void convert_32s_C2P2(const int32_t* pSrc, int32_t* const* pDst, size_t length)
{
	size_t i;
	int32_t* pDst0 = pDst[0];
	int32_t* pDst1 = pDst[1];
	
	for (i = 0; i < length; i++) {
		pDst0[i] = pSrc[2*i+0];
		pDst1[i] = pSrc[2*i+1];
	}
}
static void convert_32s_C3P3(const int32_t* pSrc, int32_t* const* pDst, size_t length)
{
	size_t i;
	int32_t* pDst0 = pDst[0];
	int32_t* pDst1 = pDst[1];
	int32_t* pDst2 = pDst[2];
	
	for (i = 0; i < length; i++) {
		pDst0[i] = pSrc[3*i+0];
		pDst1[i] = pSrc[3*i+1];
		pDst2[i] = pSrc[3*i+2];
	}
}
static void convert_32s_C4P4(const int32_t* pSrc, int32_t* const* pDst, size_t length)
{
	size_t i;
	int32_t* pDst0 = pDst[0];
	int32_t* pDst1 = pDst[1];
	int32_t* pDst2 = pDst[2];
	int32_t* pDst3 = pDst[3];
	
	for (i = 0; i < length; i++) {
		pDst0[i] = pSrc[4*i+0];
		pDst1[i] = pSrc[4*i+1];
		pDst2[i] = pSrc[4*i+2];
		pDst3[i] = pSrc[4*i+3];
	}
}
const convert_32s_CXPX convert_32s_CXPX_LUT[5] = {
	NULL,
	convert_32s_C1P1,
	convert_32s_C2P2,
	convert_32s_C3P3,
	convert_32s_C4P4
};

static void convert_32s_P1C1(int32_t const* const* pSrc, int32_t* pDst, size_t length, int32_t adjust)
{
	size_t i;
	const int32_t* pSrc0 = pSrc[0];
	
	for (i = 0; i < length; i++) {
		pDst[i] = pSrc0[i] + adjust;
	}
}
static void convert_32s_P2C2(int32_t const* const* pSrc, int32_t* pDst, size_t length, int32_t adjust)
{
	size_t i;
	const int32_t* pSrc0 = pSrc[0];
	const int32_t* pSrc1 = pSrc[1];
	
	for (i = 0; i < length; i++) {
		pDst[2*i+0] = pSrc0[i] + adjust;
		pDst[2*i+1] = pSrc1[i] + adjust;
	}
}
static void convert_32s_P3C3(int32_t const* const* pSrc, int32_t* pDst, size_t length, int32_t adjust)
{
	size_t i;
	const int32_t* pSrc0 = pSrc[0];
	const int32_t* pSrc1 = pSrc[1];
	const int32_t* pSrc2 = pSrc[2];
	
	for (i = 0; i < length; i++) {
		pDst[3*i+0] = pSrc0[i] + adjust;
		pDst[3*i+1] = pSrc1[i] + adjust;
		pDst[3*i+2] = pSrc2[i] + adjust;
	}
}
static void convert_32s_P4C4(int32_t const* const* pSrc, int32_t* pDst, size_t length, int32_t adjust)
{
	size_t i;
	const int32_t* pSrc0 = pSrc[0];
	const int32_t* pSrc1 = pSrc[1];
	const int32_t* pSrc2 = pSrc[2];
	const int32_t* pSrc3 = pSrc[3];
	
	for (i = 0; i < length; i++) {
		pDst[4*i+0] = pSrc0[i] + adjust;
		pDst[4*i+1] = pSrc1[i] + adjust;
		pDst[4*i+2] = pSrc2[i] + adjust;
		pDst[4*i+3] = pSrc3[i] + adjust;
	}
}
const convert_32s_PXCX convert_32s_PXCX_LUT[5] = {
	NULL,
	convert_32s_P1C1,
	convert_32s_P2C2,
	convert_32s_P3C3,
	convert_32s_P4C4
};

/* bit depth conversions */
/* used by PNG/TIFF up to 8bpp */
static void convert_1u32s_C1R(const uint8_t* pSrc, int32_t* pDst, size_t length)
{
	size_t i;
	for (i = 0; i < (length & ~(size_t)7U); i+=8U) {
		uint32_t val = *pSrc++;
		pDst[i+0] = (int32_t)( val >> 7);
		pDst[i+1] = (int32_t)((val >> 6) & 0x1U);
		pDst[i+2] = (int32_t)((val >> 5) & 0x1U);
		pDst[i+3] = (int32_t)((val >> 4) & 0x1U);
		pDst[i+4] = (int32_t)((val >> 3) & 0x1U);
		pDst[i+5] = (int32_t)((val >> 2) & 0x1U);
		pDst[i+6] = (int32_t)((val >> 1) & 0x1U);
		pDst[i+7] = (int32_t)(val & 0x1U);
	}
	if (length & 7U) {
		uint32_t val = *pSrc++;
		length = length & 7U;
		pDst[i+0] = (int32_t)(val >> 7);
		
		if (length > 1U) {
			pDst[i+1] = (int32_t)((val >> 6) & 0x1U);
			if (length > 2U) {
				pDst[i+2] = (int32_t)((val >> 5) & 0x1U);
				if (length > 3U) {
					pDst[i+3] = (int32_t)((val >> 4) & 0x1U);
					if (length > 4U) {
						pDst[i+4] = (int32_t)((val >> 3) & 0x1U);
						if (length > 5U) {
							pDst[i+5] = (int32_t)((val >> 2) & 0x1U);
							if (length > 6U) {
								pDst[i+6] = (int32_t)((val >> 1) & 0x1U);
							}
						}
					}
				}
			}
		}
	}
}
static void convert_2u32s_C1R(const uint8_t* pSrc, int32_t* pDst, size_t length)
{
	size_t i;
	for (i = 0; i < (length & ~(size_t)3U); i+=4U) {
		uint32_t val = *pSrc++;
		pDst[i+0] = (int32_t)( val >> 6);
		pDst[i+1] = (int32_t)((val >> 4) & 0x3U);
		pDst[i+2] = (int32_t)((val >> 2) & 0x3U);
		pDst[i+3] = (int32_t)(val & 0x3U);
	}
	if (length & 3U) {
		uint32_t val = *pSrc++;
		length = length & 3U;
		pDst[i+0] =  (int32_t)(val >> 6);
		
		if (length > 1U) {
			pDst[i+1] = (int32_t)((val >> 4) & 0x3U);
			if (length > 2U) {
				pDst[i+2] = (int32_t)((val >> 2) & 0x3U);
				
			}
		}
	}
}
static void convert_4u32s_C1R(const uint8_t* pSrc, int32_t* pDst, size_t length)
{
	size_t i;
	for (i = 0; i < (length & ~(size_t)1U); i+=2U) {
		uint32_t val = *pSrc++;
		pDst[i+0] = (int32_t)(val >> 4);
		pDst[i+1] = (int32_t)(val & 0xFU);
	}
	if (length & 1U) {
		uint8_t val = *pSrc++;
		pDst[i+0] = (int32_t)(val >> 4);
	}
}
static void convert_6u32s_C1R(const uint8_t* pSrc, int32_t* pDst, size_t length)
{
	size_t i;
	for (i = 0; i < (length & ~(size_t)3U); i+=4U) {
		uint32_t val0 = *pSrc++;
		uint32_t val1 = *pSrc++;
		uint32_t val2 = *pSrc++;
		pDst[i+0] = (int32_t)(val0 >> 2);
		pDst[i+1] = (int32_t)(((val0 & 0x3U) << 4) | (val1 >> 4));
		pDst[i+2] = (int32_t)(((val1 & 0xFU) << 2) | (val2 >> 6));
		pDst[i+3] = (int32_t)(val2 & 0x3FU);
		
	}
	if (length & 3U) {
		uint32_t val0 = *pSrc++;
		length = length & 3U;
		pDst[i+0] = (int32_t)(val0 >> 2);
		
		if (length > 1U) {
			uint32_t val1 = *pSrc++;
			pDst[i+1] = (int32_t)(((val0 & 0x3U) << 4) | (val1 >> 4));
			if (length > 2U) {
				uint32_t val2 = *pSrc++;
				pDst[i+2] = (int32_t)(((val1 & 0xFU) << 2) | (val2 >> 6));
			}
		}
	}
}
static void convert_8u32s_C1R(const uint8_t* pSrc, int32_t* pDst, size_t length)
{
	size_t i;
	for (i = 0; i < length; i++) {
		pDst[i] = pSrc[i];
	}
}
const convert_XXx32s_C1R convert_XXu32s_C1R_LUT[9] = {
	NULL,
	convert_1u32s_C1R,
	convert_2u32s_C1R,
	NULL,
	convert_4u32s_C1R,
	NULL,
	convert_6u32s_C1R,
	NULL,
	convert_8u32s_C1R
};


static void convert_32s1u_C1R(const int32_t* pSrc, uint8_t* pDst, size_t length)
{
	size_t i;
	for (i = 0; i < (length & ~(size_t)7U); i+=8U) {
		uint32_t src0 = (uint32_t)pSrc[i+0];
		uint32_t src1 = (uint32_t)pSrc[i+1];
		uint32_t src2 = (uint32_t)pSrc[i+2];
		uint32_t src3 = (uint32_t)pSrc[i+3];
		uint32_t src4 = (uint32_t)pSrc[i+4];
		uint32_t src5 = (uint32_t)pSrc[i+5];
		uint32_t src6 = (uint32_t)pSrc[i+6];
		uint32_t src7 = (uint32_t)pSrc[i+7];
		
		*pDst++ = (uint8_t)((src0 << 7) | (src1 << 6) | (src2 << 5) | (src3 << 4) | (src4 << 3) | (src5 << 2) | (src6 << 1) | src7);
	}
	
	if (length & 7U) {
		uint32_t src0 = (uint32_t)pSrc[i+0];
		uint32_t src1 = 0U;
		uint32_t src2 = 0U;
		uint32_t src3 = 0U;
		uint32_t src4 = 0U;
		uint32_t src5 = 0U;
		uint32_t src6 = 0U;
		length = length & 7U;
		
		if (length > 1U) {
			src1 = (uint32_t)pSrc[i+1];
			if (length > 2U) {
				src2 = (uint32_t)pSrc[i+2];
				if (length > 3U) {
					src3 = (uint32_t)pSrc[i+3];
					if (length > 4U) {
						src4 = (uint32_t)pSrc[i+4];
						if (length > 5U) {
							src5 = (uint32_t)pSrc[i+5];
							if (length > 6U) {
								src6 = (uint32_t)pSrc[i+6];
							}
						}
					}
				}
			}
		}
		*pDst++ = (uint8_t)((src0 << 7) | (src1 << 6) | (src2 << 5) | (src3 << 4) | (src4 << 3) | (src5 << 2) | (src6 << 1));
	}
}

static void convert_32s2u_C1R(const int32_t* pSrc, uint8_t* pDst, size_t length)
{
	size_t i;
	for (i = 0; i < (length & ~(size_t)3U); i+=4U) {
		uint32_t src0 = (uint32_t)pSrc[i+0];
		uint32_t src1 = (uint32_t)pSrc[i+1];
		uint32_t src2 = (uint32_t)pSrc[i+2];
		uint32_t src3 = (uint32_t)pSrc[i+3];
		
		*pDst++ = (uint8_t)((src0 << 6) | (src1 << 4) | (src2 << 2) | src3);
	}
	
	if (length & 3U) {
		uint32_t src0 = (uint32_t)pSrc[i+0];
		uint32_t src1 = 0U;
		uint32_t src2 = 0U;
		length = length & 3U;
		
		if (length > 1U) {
			src1 = (uint32_t)pSrc[i+1];
			if (length > 2U) {
				src2 = (uint32_t)pSrc[i+2];
			}
		}
		*pDst++ = (uint8_t)((src0 << 6) | (src1 << 4) | (src2 << 2));
	}
}

static void convert_32s4u_C1R(const int32_t* pSrc, uint8_t* pDst, size_t length)
{
	size_t i;
	for (i = 0; i < (length & ~(size_t)1U); i+=2U) {
		uint32_t src0 = (uint32_t)pSrc[i+0];
		uint32_t src1 = (uint32_t)pSrc[i+1];
		
		*pDst++ = (uint8_t)((src0 << 4) | src1);
	}
	
	if (length & 1U) {
		uint32_t src0 = (uint32_t)pSrc[i+0];
		*pDst++ = (uint8_t)((src0 << 4));
	}
}

static void convert_32s6u_C1R(const int32_t* pSrc, uint8_t* pDst, size_t length)
{
	size_t i;
	for (i = 0; i < (length & ~(size_t)3U); i+=4U) {
		uint32_t src0 = (uint32_t)pSrc[i+0];
		uint32_t src1 = (uint32_t)pSrc[i+1];
		uint32_t src2 = (uint32_t)pSrc[i+2];
		uint32_t src3 = (uint32_t)pSrc[i+3];
		
		*pDst++ = (uint8_t)((src0 << 2) | (src1 >> 4));
		*pDst++ = (uint8_t)(((src1 & 0xFU) << 4) | (src2 >> 2));
		*pDst++ = (uint8_t)(((src2 & 0x3U) << 6) | src3);
	}
	
	if (length & 3U) {
		uint32_t src0 = (uint32_t)pSrc[i+0];
		uint32_t src1 = 0U;
		uint32_t src2 = 0U;
		length = length & 3U;
		
		if (length > 1U) {
			src1 = (uint32_t)pSrc[i+1];
			if (length > 2U) {
				src2 = (uint32_t)pSrc[i+2];
			}
		}
		*pDst++ = (uint8_t)((src0 << 2) | (src1 >> 4));
		if (length > 1U) {
			*pDst++ = (uint8_t)(((src1 & 0xFU) << 4) | (src2 >> 2));
			if (length > 2U) {
				*pDst++ = (uint8_t)(((src2 & 0x3U) << 6));
			}
		}
	}
}
static void convert_32s8u_C1R(const int32_t* pSrc, uint8_t* pDst, size_t length)
{
	size_t i;
	for (i = 0; i < length; ++i) {
		pDst[i] = (uint8_t)pSrc[i];
	}
}
const convert_32sXXx_C1R convert_32sXXu_C1R_LUT[9] = {
	NULL,
	convert_32s1u_C1R,
	convert_32s2u_C1R,
	NULL,
	convert_32s4u_C1R,
	NULL,
	convert_32s6u_C1R,
	NULL,
	convert_32s8u_C1R
};


/* -->> -->> -->> -->>

  TGA IMAGE FORMAT

 <<-- <<-- <<-- <<-- */

#ifdef INFORMATION_ONLY
/* TGA header definition. */
struct tga_header
{                           
    unsigned char   id_length;              /* Image id field length    */
    unsigned char   colour_map_type;        /* Colour map type          */
    unsigned char   image_type;             /* Image type               */
    /*
    ** Colour map specification
    */
    unsigned short  colour_map_index;       /* First entry index        */
    unsigned short  colour_map_length;      /* Colour map length        */
    unsigned char   colour_map_entry_size;  /* Colour map entry size    */
    /*
    ** Image specification
    */
    unsigned short  x_origin;               /* x origin of image        */
    unsigned short  y_origin;               /* u origin of image        */
    unsigned short  image_width;            /* Image width              */
    unsigned short  image_height;           /* Image height             */
    unsigned char   pixel_depth;            /* Pixel depth              */
    unsigned char   image_desc;             /* Image descriptor         */
};
#endif /* INFORMATION_ONLY */

static unsigned short get_ushort(unsigned short val) {

#ifdef OPJ_BIG_ENDIAN
	return( ((val & 0xff) << 8) + (val >> 8) );
#else
    return( val );
#endif

}

#define TGA_HEADER_SIZE 18

static int tga_readheader(FILE *fp, unsigned int *bits_per_pixel, 
	unsigned int *width, unsigned int *height, int *flip_image)
{
	int palette_size;
	unsigned char *tga ;
	unsigned char id_len, cmap_type, image_type;
	unsigned char pixel_depth, image_desc;
	unsigned short cmap_index, cmap_len, cmap_entry_size;
	unsigned short x_origin, y_origin, image_w, image_h;

	if (!bits_per_pixel || !width || !height || !flip_image)
		return 0;
	tga = (unsigned char*)malloc(TGA_HEADER_SIZE);

	if ( fread(tga, TGA_HEADER_SIZE, 1, fp) != 1 )
	{
		fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
		free(tga);
		return 0 ;
	}
	id_len = (unsigned char)tga[0];
	cmap_type = (unsigned char)tga[1];
  (void)cmap_type;
	image_type = (unsigned char)tga[2];
	cmap_index = get_ushort(*(unsigned short*)(&tga[3]));
  (void)cmap_index;
	cmap_len = get_ushort(*(unsigned short*)(&tga[5]));
	cmap_entry_size = (unsigned char)tga[7];


	x_origin = get_ushort(*(unsigned short*)(&tga[8]));
  (void)x_origin;
	y_origin = get_ushort(*(unsigned short*)(&tga[10]));
  (void)y_origin;
	image_w = get_ushort(*(unsigned short*)(&tga[12]));
	image_h = get_ushort(*(unsigned short*)(&tga[14]));
	pixel_depth = (unsigned char)tga[16];
	image_desc  = (unsigned char)tga[17];

	free(tga);

	*bits_per_pixel = (unsigned int)pixel_depth;
	*width  = (unsigned int)image_w;
	*height = (unsigned int)image_h;

	/* Ignore tga identifier, if present ... */
	if (id_len)
	{
		unsigned char *id = (unsigned char *) malloc(id_len);
		if ( !fread(id, id_len, 1, fp) )
		{
			fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
			free(id);
		    return 0 ;
		}
		free(id);  
	}

	/* Test for compressed formats ... not yet supported ...
	// Note :-  9 - RLE encoded palettized.
	//	  	   10 - RLE encoded RGB. */
	if (image_type > 8)
	{
		fprintf(stderr, "Sorry, compressed tga files are not currently supported.\n");
		return 0 ;
	}

	*flip_image = !(image_desc & 32);

	/* Palettized formats are not yet supported, skip over the palette, if present ... */
	palette_size = cmap_len * (cmap_entry_size/8);
	
	if (palette_size>0)
	{
		fprintf(stderr, "File contains a palette - not yet supported.");
		fseek(fp, palette_size, SEEK_CUR);
	}
	return 1;
}

#ifdef OPJ_BIG_ENDIAN

static inline int16_t swap16(int16_t x)
{
  return((((u_int16_t)x & 0x00ffU) <<  8) |
     (((u_int16_t)x & 0xff00U) >>  8));
}

#endif

static int tga_writeheader(FILE *fp, int bits_per_pixel, int width, int height, 
	opj_bool flip_image)
{
	unsigned short image_w, image_h, us0;
	unsigned char uc0, image_type;
	unsigned char pixel_depth, image_desc;

	if (!bits_per_pixel || !width || !height)
		return 0;

	pixel_depth = 0;

	if ( bits_per_pixel < 256 )
		pixel_depth = (unsigned char)bits_per_pixel;
	else{
		fprintf(stderr,"ERROR: Wrong bits per pixel inside tga_header");
		return 0;
	}
	uc0 = 0;

	if(fwrite(&uc0, 1, 1, fp) != 1) goto fails; /* id_length */
	if(fwrite(&uc0, 1, 1, fp) != 1) goto fails; /* colour_map_type */

	image_type = 2; /* Uncompressed. */
	if(fwrite(&image_type, 1, 1, fp) != 1) goto fails;

	us0 = 0;
	if(fwrite(&us0, 2, 1, fp) != 1) goto fails; /* colour_map_index */
	if(fwrite(&us0, 2, 1, fp) != 1) goto fails; /* colour_map_length */
	if(fwrite(&uc0, 1, 1, fp) != 1) goto fails; /* colour_map_entry_size */

	if(fwrite(&us0, 2, 1, fp) != 1) goto fails; /* x_origin */
	if(fwrite(&us0, 2, 1, fp) != 1) goto fails; /* y_origin */

	image_w = (unsigned short)width;
	image_h = (unsigned short) height;

#ifndef OPJ_BIG_ENDIAN
	if(fwrite(&image_w, 2, 1, fp) != 1) goto fails;
	if(fwrite(&image_h, 2, 1, fp) != 1) goto fails;
#else
	image_w = swap16(image_w);
	image_h = swap16(image_h);
	if(fwrite(&image_w, 2, 1, fp) != 1) goto fails;
	if(fwrite(&image_h, 2, 1, fp) != 1) goto fails;
#endif

	if(fwrite(&pixel_depth, 1, 1, fp) != 1) goto fails;

	image_desc = // bits 0-3 are # of alpha bits per pixel
		bits_per_pixel == 16 ? 1 :
		bits_per_pixel == 32 ? 8 :
		0;

	if (flip_image)
		image_desc |= 32;
	if(fwrite(&image_desc, 1, 1, fp) != 1) goto fails;

	return 1;

fails:
	fputs("\nwrite_tgaheader: write ERROR\n", stderr);
	return 0;
}

opj_image_t* tgatoimage(const char *filename, opj_cparameters_t *parameters) {
	FILE *f;
	opj_image_t *image;
	unsigned int image_width, image_height, pixel_bit_depth;
	unsigned int x, y;
	int flip_image=0;
	opj_image_cmptparm_t cmptparm[4];	/* maximum 4 components */
	int numcomps;
	OPJ_COLOR_SPACE color_space;
	opj_bool mono ;
	opj_bool save_alpha;
	int subsampling_dx, subsampling_dy;
	int i;	

	f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "Failed to open %s for reading !!\n", filename);
		return 0;
	}

	if (!tga_readheader(f, &pixel_bit_depth, &image_width, &image_height, &flip_image)) {
		fclose(f);
		return NULL;
	}

	/* We currently only support 24 & 32 bit tga's ... */
	if (!((pixel_bit_depth == 24) || (pixel_bit_depth == 32))) {
		fclose(f);
		return NULL;
	}

	/* initialize image components */   
	memset(&cmptparm[0], 0, 4 * sizeof(opj_image_cmptparm_t));

	mono = (pixel_bit_depth == 8) || (pixel_bit_depth == 16);  /* Mono with & without alpha. */
	save_alpha = (pixel_bit_depth == 16) || (pixel_bit_depth == 32); /* Mono with alpha, or RGB with alpha */

	if (mono) {
		color_space = CLRSPC_GRAY;
		numcomps = save_alpha ? 2 : 1;
	}	
	else {
		numcomps = save_alpha ? 4 : 3;
		color_space = CLRSPC_SRGB;
	}

	subsampling_dx = parameters->subsampling_dx;
	subsampling_dy = parameters->subsampling_dy;

	for (i = 0; i < numcomps; i++) {
		cmptparm[i].prec = 8;
		cmptparm[i].bpp = 8;
		cmptparm[i].sgnd = 0;
		cmptparm[i].dx = subsampling_dx;
		cmptparm[i].dy = subsampling_dy;
		cmptparm[i].w = image_width;
		cmptparm[i].h = image_height;
	}

	/* create the image */
	image = opj_image_create(numcomps, &cmptparm[0], color_space);

	if (!image) {
		fclose(f);
		return NULL;
	}

	/* set image offset and reference grid */
	image->x0 = parameters->image_offset_x0;
	image->y0 = parameters->image_offset_y0;
	image->x1 =	!image->x0 ? (image_width - 1) * subsampling_dx + 1 : image->x0 + (image_width - 1) * subsampling_dx + 1;
	image->y1 =	!image->y0 ? (image_height - 1) * subsampling_dy + 1 : image->y0 + (image_height - 1) * subsampling_dy + 1;

	/* set image data */
	for (y=0; y < image_height; y++) 
	{
		int index;

		if (flip_image)
			index = (image_height-y-1)*image_width;
		else
			index = y*image_width;

		if (numcomps==3)
		{
			for (x=0;x<image_width;x++) 
			{
				unsigned char r,g,b;

				if( !fread(&b, 1, 1, f) )
				{
					fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
					opj_image_destroy(image);
					fclose(f);
					return NULL;
				}
				if ( !fread(&g, 1, 1, f) )
				{
					fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
					opj_image_destroy(image);
					fclose(f);
					return NULL;
				}
				if ( !fread(&r, 1, 1, f) )
				{
					fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
					opj_image_destroy(image);
					fclose(f);
					return NULL;
				}

				image->comps[0].data[index]=r;
				image->comps[1].data[index]=g;
				image->comps[2].data[index]=b;
				index++;
			}
		}
		else if (numcomps==4)
		{
			for (x=0;x<image_width;x++) 
			{
				unsigned char r,g,b,a;
				if ( !fread(&b, 1, 1, f) )
				{
					fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
					opj_image_destroy(image);
					fclose(f);
					return NULL;
				}
				if ( !fread(&g, 1, 1, f) )
				{
					fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
					opj_image_destroy(image);
					fclose(f);
					return NULL;
				}
				if ( !fread(&r, 1, 1, f) )
				{
					fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
					opj_image_destroy(image);
					fclose(f);
					return NULL;
				}
				if ( !fread(&a, 1, 1, f) )
				{
					fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
					opj_image_destroy(image);
					fclose(f);
					return NULL;
				}

				image->comps[0].data[index]=r;
				image->comps[1].data[index]=g;
				image->comps[2].data[index]=b;
				image->comps[3].data[index]=a;
				index++;
			}
		}
		else {
			fprintf(stderr, "Currently unsupported bit depth : %s\n", filename);
		}
	}
	fclose(f);
	return image;
}

int imagetotga(opj_image_t * image, const char *outfile) {
	int width, height, bpp, x, y;
	opj_bool write_alpha;
	int i, adjustR, adjustG, adjustB;
	unsigned int alpha_channel;
	float r,g,b,a;
	unsigned char value;
	float scale;
	FILE *fdest;
  size_t res;

	fdest = fopen(outfile, "wb");
	if (!fdest) {
		fprintf(stderr, "ERROR -> failed to open %s for writing\n", outfile);
		return 1;
	}

	for (i = 0; i < image->numcomps-1; i++)	{
		if ((image->comps[0].dx != image->comps[i+1].dx) 
			||(image->comps[0].dy != image->comps[i+1].dy) 
			||(image->comps[0].prec != image->comps[i+1].prec))	{
      fprintf(stderr, "Unable to create a tga file with such J2K image charateristics.");
      fclose(fdest);
      return 1;
   }
	}

	width = image->comps[0].w;
	height = image->comps[0].h; 

	/* Mono with alpha, or RGB with alpha. */
	write_alpha = (image->numcomps==2) || (image->numcomps==4);   

	/* Write TGA header  */
	bpp = write_alpha ? 32 : 24;
	if (!tga_writeheader(fdest, bpp, width , height, OPJ_TRUE)) {
		fclose(fdest);
		return 1;
	}

	alpha_channel = image->numcomps-1; 

	scale = 255.0f / (float)((1<<image->comps[0].prec)-1);

	adjustR = (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
	adjustG = (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
	adjustB = (image->comps[2].sgnd ? 1 << (image->comps[2].prec - 1) : 0);

	for (y=0; y < height; y++) {
		unsigned int index=y*width;

		for (x=0; x < width; x++, index++)	{
			r = (float)(image->comps[0].data[index] + adjustR);

			if (image->numcomps>2) {
				g = (float)(image->comps[1].data[index] + adjustG);
				b = (float)(image->comps[2].data[index] + adjustB);
			}
			else  {/* Greyscale ... */
				g = r;
				b = r;
			}

			/* TGA format writes BGR ... */
			value = (unsigned char)(b*scale);
			res = fwrite(&value,1,1,fdest);
      if( res < 1 ) {
        fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
        fclose(fdest);
        return 1;
      }

			value = (unsigned char)(g*scale);
			res = fwrite(&value,1,1,fdest);
      if( res < 1 ) {
        fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
        fclose(fdest);
        return 1;
      }

			value = (unsigned char)(r*scale);
			res = fwrite(&value,1,1,fdest);
      if( res < 1 ) {
        fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
        fclose(fdest);
        return 1;
      }

			if (write_alpha) {
				a = (float)(image->comps[alpha_channel].data[index]);
				value = (unsigned char)(a*scale);
				res = fwrite(&value,1,1,fdest);
        if( res < 1 ) {
          fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
          fclose(fdest);
          return 1;
        }
			}
		}
	}

	fclose(fdest);
	return 0;
}

/* -->> -->> -->> -->>

PGX IMAGE FORMAT

<<-- <<-- <<-- <<-- */


static unsigned char readuchar(FILE * f)
{
  unsigned char c1;
  if ( !fread(&c1, 1, 1, f) )
  {
	  fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
	  return 0;
  }
  return c1;
}

static unsigned short readushort(FILE * f, int bigendian)
{
  unsigned char c1, c2;
  if ( !fread(&c1, 1, 1, f) )
  {
	  fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
	  return 0;
  }
  if ( !fread(&c2, 1, 1, f) )
  {
	  fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
	  return 0;
  }
  if (bigendian)
    return (c1 << 8) + c2;
  else
    return (c2 << 8) + c1;
}

static unsigned int readuint(FILE * f, int bigendian)
{
  unsigned char c1, c2, c3, c4;
  if ( !fread(&c1, 1, 1, f) )
  {
	  fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
	  return 0;
  }
  if ( !fread(&c2, 1, 1, f) )
  {
	  fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
	  return 0;
  }
  if ( !fread(&c3, 1, 1, f) )
  {
	  fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
	  return 0;
  }
  if ( !fread(&c4, 1, 1, f) )
  {
	  fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
	  return 0;
  }
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
	int adjustS, ushift, dshift, force8;

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
	if( fscanf(f, "PG%[ \t]%c%c%[ \t+-]%d%[ \t]%d%[ \t]%d",temp,&endian1,&endian2,signtmp,&prec,temp,&w,temp,&h) != 9){
		fprintf(stderr, "ERROR: Failed to read the right number of element from the fscanf() function!\n");
		fclose(f);
		return NULL;
	}

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
		fclose(f);
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
	if(prec < 8)
   {
	force8 = 1;
	ushift = 8 - prec; dshift = prec - ushift;
	if(cmptparm.sgnd) adjustS = (1<<(prec - 1)); else adjustS = 0;
	cmptparm.sgnd = 0;
	prec = 8;
   }
	else ushift = dshift = force8 = adjustS = 0;

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
		if(force8)
	   {
		v = readuchar(f) + adjustS;
		v = (v<<ushift) + (v>>dshift);
		comp->data[i] = (unsigned char)v;

		if(v > max) max = v;

		continue;
	   }
		if (comp->prec == 8) {
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
	int w, h;
	int i, j, compno;
	FILE *fdest = NULL;

	for (compno = 0; compno < image->numcomps; compno++) {
		opj_image_comp_t *comp = &image->comps[compno];
		char bname[256]; /* buffer for name */
    char *name = bname; /* pointer */
    int nbytes = 0;
    size_t res;
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
		/*if (image->numcomps > 1) {*/
			sprintf(name+dotpos, "_%d.pgx", compno);
		/*} else {
			strcpy(name+dotpos, ".pgx");
		}*/
		fdest = fopen(name, "wb");
		if (!fdest) {
			fprintf(stderr, "ERROR -> failed to open %s for writing\n", name);
			return 1;
		}

		w = image->comps[compno].w;
		h = image->comps[compno].h;
	    
		fprintf(fdest, "PG ML %c %d %d %d\n", comp->sgnd ? '-' : '+', comp->prec, w, h);
		if (comp->prec <= 8) {
			nbytes = 1;
		} else if (comp->prec <= 16) {
			nbytes = 2;
		} else {
			nbytes = 4;
		}
		for (i = 0; i < w * h; i++) {
			int v = image->comps[compno].data[i];
			for (j = nbytes - 1; j >= 0; j--) {
				char byte = (char) (v >> (j * 8));
				res = fwrite(&byte, 1, 1, fdest);
        if( res < 1 ) {
          fprintf(stderr, "failed to write 1 byte for %s\n", name);
          if( total > 256 ) {
            free(name);
            }
          fclose(fdest);
          return 1;
        }
			}
		}
		/* dont need name anymore */
		if( total > 256 ) {
			free(name);
			}

		fclose(fdest);
	}

	return 0;
}

/* -->> -->> -->> -->>

PNM IMAGE FORMAT

<<-- <<-- <<-- <<-- */

struct pnm_header
{
    int width, height, maxval, depth, format;
    char rgb, rgba, gray, graya, bw;
    char ok;
};

static char *skip_white(char *s)
{
    while(*s)
   {
    if(*s == '\n' || *s == '\r') return NULL;
    if(isspace(*s)) { ++s; continue; }
    return s;
   }
    return NULL;
}

static char *skip_int(char *start, int *out_n)
{
    char *s;
    char c;

    *out_n = 0; s = start;

    s = skip_white(start);
    if(s == NULL) return NULL;
    start = s;

    while(*s)
   {
    if( !isdigit(*s)) break;
    ++s;
   }
    c = *s; *s = 0; *out_n = atoi(start); *s = c;
    return s;
}

static char *skip_idf(char *start, char out_idf[256])
{
    char *s;
    char c;

    s = skip_white(start);
    if(s == NULL) return NULL;
    start = s;

    while(*s)
   {
    if(isalpha(*s) || *s == '_') { ++s; continue; }
    break;
   }
    c = *s; *s = 0; strncpy(out_idf, start, 255); *s = c;
    return s;
}

static void read_pnm_header(FILE *reader, struct pnm_header *ph)
{
    char *s;
    int format, have_wh, end, ttype;
    char idf[256], type[256];
    char line[256];

    if (fgets(line, 250, reader) == NULL)
    {
    	fprintf(stderr,"\nWARNING: fgets return a NULL value");
    	return;
    }

    if(line[0] != 'P')
   {
    fprintf(stderr,"read_pnm_header:PNM:magic P missing\n"); return;
   }
    format = atoi(line + 1);
    if(format < 1 || format > 7)
   {
    fprintf(stderr,"read_pnm_header:magic format %d invalid\n", format);
    return;
   }
    ph->format = format;
    ttype = end = have_wh = 0;

    while(fgets(line, 250, reader))
   {
    if(*line == '#') continue;

    s = line;

    if(format == 7)
  {
    s = skip_idf(s, idf);

    if(s == NULL || *s == 0) return;

    if(strcmp(idf, "ENDHDR") == 0)
 {
    end = 1; break;
 }
    if(strcmp(idf, "WIDTH") == 0)
 {
    s = skip_int(s, &ph->width);
    if(s == NULL || *s == 0) return;

    continue;
 }
    if(strcmp(idf, "HEIGHT") == 0)
 {
    s = skip_int(s, &ph->height);
    if(s == NULL || *s == 0) return;

    continue;
 }
    if(strcmp(idf, "DEPTH") == 0)
 {
    s = skip_int(s, &ph->depth);
    if(s == NULL || *s == 0) return;

    continue;
 }
    if(strcmp(idf, "MAXVAL") == 0)
 {
    s = skip_int(s, &ph->maxval);
    if(s == NULL || *s == 0) return;

    continue;
 }
    if(strcmp(idf, "TUPLTYPE") == 0)
 {
    s = skip_idf(s, type);
    if(s == NULL || *s == 0) return;

        if(strcmp(type, "BLACKANDWHITE") == 0)
       {
        ph->bw = 1; ttype = 1; continue;
       }
        if(strcmp(type, "GRAYSCALE") == 0)
       {
        ph->gray = 1; ttype = 1; continue;
       }
        if(strcmp(type, "GRAYSCALE_ALPHA") == 0)
       {
        ph->graya = 1; ttype = 1; continue;
       }
        if(strcmp(type, "RGB") == 0)
       {
        ph->rgb = 1; ttype = 1; continue;
       }
        if(strcmp(type, "RGB_ALPHA") == 0)
       {
        ph->rgba = 1; ttype = 1; continue;
       }
    fprintf(stderr,"read_pnm_header:unknown P7 TUPLTYPE %s\n",type);
    return;
 }
    fprintf(stderr,"read_pnm_header:unknown P7 idf %s\n",idf);
    return;
  } /* if(format == 7) */

    if( !have_wh)
  {
    s = skip_int(s, &ph->width);

    s = skip_int(s, &ph->height);

    have_wh = 1;

    if(format == 1 || format == 4) break;

    continue;
  }
    if(format == 2 || format == 3 || format == 5 || format == 6)
  {
/* P2, P3, P5, P6: */
    s = skip_int(s, &ph->maxval);

    if(ph->maxval > 65535) return;
  }
    break;
   }/* while(fgets( ) */
    if(format == 2 || format == 3 || format > 4)
   {
    if(ph->maxval < 1 || ph->maxval > 65535) return;
   }
    if(ph->width < 1 || ph->height < 1) return;

    if(format == 7)
   {
    if(!end)
  {
    fprintf(stderr,"read_pnm_header:P7 without ENDHDR\n"); return;
  }
    if(ph->depth < 1 || ph->depth > 4) return;

    if(ph->width && ph->height && ph->depth & ph->maxval && ttype)
     ph->ok = 1;
   }
    else
   {
    if(format != 1 && format != 4)
  {
    if(ph->width && ph->height && ph->maxval) ph->ok = 1;
  }
    else
  {
    if(ph->width && ph->height) ph->ok = 1;
    ph->maxval = 255;
  }
   }
}

static int has_prec(int val)
{
    if(val < 2) return 1;
    if(val < 4) return 2;
    if(val < 8) return 3;
    if(val < 16) return 4;
    if(val < 32) return 5;
    if(val < 64) return 6;
    if(val < 128) return 7;
    if(val < 256) return 8;
    if(val < 512) return 9;
    if(val < 1024) return 10;
    if(val < 2048) return 11;
    if(val < 4096) return 12;
    if(val < 8192) return 13;
    if(val < 16384) return 14;
    if(val < 32768) return 15;
    return 16;
}

opj_image_t* pnmtoimage(const char *filename, opj_cparameters_t *parameters) {
	int subsampling_dx = parameters->subsampling_dx;
	int subsampling_dy = parameters->subsampling_dy;

	FILE *fp = NULL;
	int i, compno, numcomps, w, h, prec, format;
	OPJ_COLOR_SPACE color_space;
	opj_image_cmptparm_t cmptparm[4]; /* RGBA: max. 4 components */
	opj_image_t * image = NULL;
	struct pnm_header header_info;
	
	if((fp = fopen(filename, "rb")) == NULL)
   {
	fprintf(stderr, "pnmtoimage:Failed to open %s for reading!\n",filename);
	return NULL;
   }
	memset(&header_info, 0, sizeof(struct pnm_header));

	read_pnm_header(fp, &header_info);

	if(!header_info.ok) { fclose(fp); return NULL; }

	format = header_info.format;

    switch(format)
   {
    case 1: /* ascii bitmap */
    case 4: /* raw bitmap */
        numcomps = 1;
        break;

    case 2: /* ascii greymap */
    case 5: /* raw greymap */
        numcomps = 1;
        break;

    case 3: /* ascii pixmap */
    case 6: /* raw pixmap */
        numcomps = 3;
        break;

    case 7: /* arbitrary map */
        numcomps = header_info.depth;
		break;

    default: fclose(fp); return NULL;
   }
    if(numcomps < 3)
     color_space = CLRSPC_GRAY;/* GRAY, GRAYA */
    else
     color_space = CLRSPC_SRGB;/* RGB, RGBA */

    prec = has_prec(header_info.maxval);

	if(prec < 8) prec = 8;

    w = header_info.width;
    h = header_info.height;
    subsampling_dx = parameters->subsampling_dx;
    subsampling_dy = parameters->subsampling_dy;

    memset(&cmptparm[0], 0, numcomps * sizeof(opj_image_cmptparm_t));

    for(i = 0; i < numcomps; i++)
   {
    cmptparm[i].prec = prec;
    cmptparm[i].bpp = prec;
    cmptparm[i].sgnd = 0;
    cmptparm[i].dx = subsampling_dx;
    cmptparm[i].dy = subsampling_dy;
    cmptparm[i].w = w;
    cmptparm[i].h = h;
   }
    image = opj_image_create(numcomps, &cmptparm[0], color_space);

    if(!image) { fclose(fp); return NULL; }

/* set image offset and reference grid */
	image->x0 = parameters->image_offset_x0;
	image->y0 = parameters->image_offset_y0;
	image->x1 = parameters->image_offset_x0 + (w - 1) *	subsampling_dx + 1;
	image->y1 = parameters->image_offset_y0 + (h - 1) *	subsampling_dy + 1;

    if((format == 2) || (format == 3)) /* ascii pixmap */
   {
    unsigned int index;

    for (i = 0; i < w * h; i++)
  {
    for(compno = 0; compno < numcomps; compno++)
 {
	index = 0;
    if (fscanf(fp, "%u", &index) != 1)
    	fprintf(stderr, "\nWARNING: fscanf return a number of element different from the expected.\n");

    image->comps[compno].data[i] = (index * 255)/header_info.maxval;
 }
  }
   }
    else
    if((format == 5)
    || (format == 6)
    ||((format == 7)
        && (   header_info.gray || header_info.graya
            || header_info.rgb || header_info.rgba)))/* binary pixmap */
   {
    unsigned char c0, c1, one;

    one = (prec < 9); 

    for (i = 0; i < w * h; i++)
  {
    for(compno = 0; compno < numcomps; compno++)
 {
    	  if ( !fread(&c0, 1, 1, fp) )
    		  fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
        if(one)
       {
        image->comps[compno].data[i] = c0;
       }
        else
       {
      	  if ( !fread(&c1, 1, 1, fp) )
      		  fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
/* netpbm: */
		image->comps[compno].data[i] = ((c0<<8) | c1);
       }
 }
  }
   }
    else
    if(format == 1) /* ascii bitmap */
   {
    for (i = 0; i < w * h; i++)
  {
    unsigned int index;

    if ( fscanf(fp, "%u", &index) != 1)
    	fprintf(stderr, "\nWARNING: fscanf return a number of element different from the expected.\n");

    image->comps[0].data[i] = (index?0:255);
  }
   }
    else
    if(format == 4)
   {
    int x, y, bit;
    unsigned char uc;

    i = 0;
    for(y = 0; y < h; ++y)
  {
    bit = -1; uc = 0;

    for(x = 0; x < w; ++x)
 {
        if(bit == -1)
       {
        bit = 7;
        uc = (unsigned char)getc(fp);
       }
    image->comps[0].data[i] = (((uc>>bit) & 1)?0:255);
    --bit; ++i;
 }
  }
   }
	else
	if((format == 7 && header_info.bw)) /*MONO*/
   {
	unsigned char uc;

	for(i = 0; i < w * h; ++i)
  {
  	  if ( !fread(&uc, 1, 1, fp) )
  		  fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
	image->comps[0].data[i] = (uc & 1)?0:255;
  }
   }
    fclose(fp);

    return image;
}/* pnmtoimage() */

int imagetopnm(opj_image_t * image, const char *outfile) 
{
	int *red, *green, *blue, *alpha;
	int wr, hr, max;
	int i, compno, ncomp;
	int adjustR, adjustG, adjustB, adjustA;
	int fails, two, want_gray, has_alpha, triple;
	int prec, v;
	FILE *fdest = NULL;
	const char *tmp = outfile;
	char *destname;
  alpha = NULL;
    if((prec = image->comps[0].prec) > 16)
   {
	fprintf(stderr,"%s:%d:imagetopnm\n\tprecision %d is larger than 16"
	"\n\t: refused.\n",__FILE__,__LINE__,prec);
	return 1;
   }
    two = has_alpha = 0; fails = 1;
	ncomp = image->numcomps;

	while (*tmp) ++tmp; tmp -= 2; 
	want_gray = (*tmp == 'g' || *tmp == 'G'); 
	ncomp = image->numcomps;

	if(want_gray) ncomp = 1;

	if (ncomp == 2 /* GRAYA */
	|| (ncomp > 2 /* RGB, RGBA */
		&& image->comps[0].dx == image->comps[1].dx
		&& image->comps[1].dx == image->comps[2].dx
		&& image->comps[0].dy == image->comps[1].dy
		&& image->comps[1].dy == image->comps[2].dy
		&& image->comps[0].prec == image->comps[1].prec
		&& image->comps[1].prec == image->comps[2].prec
	   ))
   {
	fdest = fopen(outfile, "wb");

	if (!fdest) 
  {
	fprintf(stderr, "ERROR -> failed to open %s for writing\n", outfile);
	return fails;
  }
	two = (prec > 8);
	triple = (ncomp > 2);
	wr = image->comps[0].w; hr = image->comps[0].h;
	max = (1<<prec) - 1; has_alpha = (ncomp == 4 || ncomp == 2);

    red = image->comps[0].data;

	if(triple)
  {
    green = image->comps[1].data;
    blue = image->comps[2].data;
  }
	else green = blue = NULL;
	
	if(has_alpha)
  {
	const char *tt = (triple?"RGB_ALPHA":"GRAYSCALE_ALPHA");

	fprintf(fdest, "P7\n# OpenJPEG-%s\nWIDTH %d\nHEIGHT %d\nDEPTH %d\n"
		"MAXVAL %d\nTUPLTYPE %s\nENDHDR\n", opj_version(),
		wr, hr, ncomp, max, tt);
	alpha = image->comps[ncomp - 1].data;
	adjustA = (image->comps[ncomp - 1].sgnd ?
	 1 << (image->comps[ncomp - 1].prec - 1) : 0);
  }
	else
  {
	fprintf(fdest, "P6\n# OpenJPEG-%s\n%d %d\n%d\n", 
		opj_version(), wr, hr, max);
	adjustA = 0;
  }
    adjustR = (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);

	if(triple)
  {
    adjustG = (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
    adjustB = (image->comps[2].sgnd ? 1 << (image->comps[2].prec - 1) : 0);
  }
	else adjustG = adjustB = 0;

    for(i = 0; i < wr * hr; ++i)
  {
	if(two)
 {
	v = *red + adjustR; ++red;
/* netpbm: */
	fprintf(fdest, "%c%c",(unsigned char)(v>>8), (unsigned char)v);

		if(triple)
	   {
		v = *green + adjustG; ++green;
/* netpbm: */
		fprintf(fdest, "%c%c",(unsigned char)(v>>8), (unsigned char)v);

		v =  *blue + adjustB; ++blue;
/* netpbm: */
		fprintf(fdest, "%c%c",(unsigned char)(v>>8), (unsigned char)v);

	   }/* if(triple) */

        if(has_alpha)
       {
        v = *alpha + adjustA; ++alpha;
/* netpbm: */
		fprintf(fdest, "%c%c",(unsigned char)(v>>8), (unsigned char)v);
       }
        continue;

 }	/* if(two) */

/* prec <= 8: */

	fprintf(fdest, "%c", (unsigned char)*red++);
	if(triple)
	 fprintf(fdest, "%c%c",(unsigned char)*green++, (unsigned char)*blue++);

	if(has_alpha)
 	 fprintf(fdest, "%c", (unsigned char)*alpha++);

  }	/* for(i */

	fclose(fdest); return 0;
   }

/* YUV or MONO: */

	if (image->numcomps > ncomp) 
   {
	fprintf(stderr,"WARNING -> [PGM file] Only the first component\n");
	fprintf(stderr,"           is written to the file\n");
   }
	destname = (char*)malloc(strlen(outfile) + 8);

	for (compno = 0; compno < ncomp; compno++) 
   {
	if (ncomp > 1) 
	 sprintf(destname, "%d.%s", compno, outfile);
	else
	 sprintf(destname, "%s", outfile);

	fdest = fopen(destname, "wb");
	if (!fdest) 
  {
	fprintf(stderr, "ERROR -> failed to open %s for writing\n", destname);
	free(destname);
	return 1;
  }
	wr = image->comps[compno].w; hr = image->comps[compno].h;
	prec = image->comps[compno].prec;
	max = (1<<prec) - 1;

	fprintf(fdest, "P5\n#OpenJPEG-%s\n%d %d\n%d\n", 
		opj_version(), wr, hr, max);

	red = image->comps[compno].data;
	adjustR = 
	(image->comps[compno].sgnd ? 1 << (image->comps[compno].prec - 1) : 0);

    if(prec > 8)
  {
	for (i = 0; i < wr * hr; i++) 
 {
	v = *red + adjustR; ++red;
/* netpbm: */
	fprintf(fdest, "%c%c",(unsigned char)(v>>8), (unsigned char)v);

        if(has_alpha)
      {
        v = *alpha++;
/* netpbm: */
		fprintf(fdest, "%c%c",(unsigned char)(v>>8), (unsigned char)v);
      }
 }/* for(i */
  }
	else /* prec <= 8 */
  {
	for(i = 0; i < wr * hr; ++i)
 {
	 fprintf(fdest, "%c", (unsigned char)(*red + adjustR)); ++red;
 }
  }
	fclose(fdest);
   } /* for (compno */
	free(destname);

	return 0;
}/* imagetopnm() */

/* -->> -->> -->> -->>

	RAW IMAGE FORMAT

 <<-- <<-- <<-- <<-- */

opj_image_t* rawtoimage(const char *filename, opj_cparameters_t *parameters, raw_cparameters_t *raw_cp) {
	int subsampling_dx = parameters->subsampling_dx;
	int subsampling_dy = parameters->subsampling_dy;

	FILE *f = NULL;
	int i, compno, numcomps, w, h;
	OPJ_COLOR_SPACE color_space;
	opj_image_cmptparm_t *cmptparm;	
	opj_image_t * image = NULL;
	unsigned short ch;
	
	if((! (raw_cp->rawWidth & raw_cp->rawHeight & raw_cp->rawComp & raw_cp->rawBitDepth)) == 0)
	{
		fprintf(stderr,"\nError: invalid raw image parameters\n");
		fprintf(stderr,"Please use the Format option -F:\n");
		fprintf(stderr,"-F rawWidth,rawHeight,rawComp,rawBitDepth,s/u (Signed/Unsigned)\n");
		fprintf(stderr,"Example: -i lena.raw -o lena.j2k -F 512,512,3,8,u\n");
		fprintf(stderr,"Aborting\n");
		return NULL;
	}

	f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "Failed to open %s for reading !!\n", filename);
		fprintf(stderr,"Aborting\n");
		return NULL;
	}
	numcomps = raw_cp->rawComp;
	color_space = CLRSPC_SRGB;
	w = raw_cp->rawWidth;
	h = raw_cp->rawHeight;
	cmptparm = (opj_image_cmptparm_t*) malloc(numcomps * sizeof(opj_image_cmptparm_t));
	
	/* initialize image components */	
	memset(&cmptparm[0], 0, numcomps * sizeof(opj_image_cmptparm_t));
	for(i = 0; i < numcomps; i++) {		
		cmptparm[i].prec = raw_cp->rawBitDepth;
		cmptparm[i].bpp = raw_cp->rawBitDepth;
		cmptparm[i].sgnd = raw_cp->rawSigned;
		cmptparm[i].dx = subsampling_dx;
		cmptparm[i].dy = subsampling_dy;
		cmptparm[i].w = w;
		cmptparm[i].h = h;
	}
	/* create the image */
	image = opj_image_create(numcomps, &cmptparm[0], color_space);
	if(!image) {
		fclose(f);
		free(cmptparm);
		return NULL;
	}
	/* set image offset and reference grid */
	image->x0 = parameters->image_offset_x0;
	image->y0 = parameters->image_offset_y0;
	image->x1 = parameters->image_offset_x0 + (w - 1) *	subsampling_dx + 1;
	image->y1 = parameters->image_offset_y0 + (h - 1) *	subsampling_dy + 1;

	if(raw_cp->rawBitDepth <= 8)
	{
		unsigned char value = 0;
		for(compno = 0; compno < numcomps; compno++) {
			for (i = 0; i < w * h; i++) {
				if (!fread(&value, 1, 1, f)) {
					fprintf(stderr,"Error reading raw file. End of file probably reached.\n");
					fclose(f);
					free(cmptparm);
					opj_image_destroy(image);
					return NULL;
				}
				image->comps[compno].data[i] = raw_cp->rawSigned?(char)value:value;
			}
		}
	}
	else if(raw_cp->rawBitDepth <= 16)
	{
		unsigned short value;
		for(compno = 0; compno < numcomps; compno++) {
			for (i = 0; i < w * h; i++) {
				unsigned char temp;
				if (!fread(&temp, 1, 1, f)) {
					fprintf(stderr,"Error reading raw file. End of file probably reached.\n");
					fclose(f);
					free(cmptparm);
					opj_image_destroy(image);
					return NULL;
				}
				value = temp << 8;
				if (!fread(&temp, 1, 1, f)) {
					fprintf(stderr,"Error reading raw file. End of file probably reached.\n");
					fclose(f);
					free(cmptparm);
					opj_image_destroy(image);
					return NULL;
				}
				value += temp;
				image->comps[compno].data[i] = raw_cp->rawSigned?(short)value:value;
			}
		}
	}
	else {
		fprintf(stderr,"OpenJPEG cannot encode raw components with bit depth higher than 16 bits.\n");
		fclose(f);
		free(cmptparm);
		opj_image_destroy(image);
		return NULL;
	}

	if (fread(&ch, 1, 1, f)) {
		fprintf(stderr,"Warning. End of raw file not reached... processing anyway\n");
	}
	fclose(f);
	free(cmptparm);

	return image;
}

int imagetoraw(opj_image_t * image, const char *outfile)
{
	FILE *rawFile = NULL;
  size_t res;
	int compno;
	int w, h;
	int line, row;
	int *ptr;

	if((image->numcomps * image->x1 * image->y1) == 0)
	{
		fprintf(stderr,"\nError: invalid raw image parameters\n");
		return 1;
	}

	rawFile = fopen(outfile, "wb");
	if (!rawFile) {
		fprintf(stderr, "Failed to open %s for writing !!\n", outfile);
		return 1;
	}

	fprintf(stdout,"Raw image characteristics: %d components\n", image->numcomps);

	for(compno = 0; compno < image->numcomps; compno++)
	{
		fprintf(stdout,"Component %d characteristics: %dx%dx%d %s\n", compno, image->comps[compno].w,
			image->comps[compno].h, image->comps[compno].prec, image->comps[compno].sgnd==1 ? "signed": "unsigned");

		w = image->comps[compno].w;
		h = image->comps[compno].h;

		if(image->comps[compno].prec <= 8)
		{
			if(image->comps[compno].sgnd == 1)
			{
				signed char curr;
				int mask = (1 << image->comps[compno].prec) - 1;
				ptr = image->comps[compno].data;
				for (line = 0; line < h; line++) {
					for(row = 0; row < w; row++)	{				
						curr = (signed char) (*ptr & mask);
						res = fwrite(&curr, sizeof(signed char), 1, rawFile);
            if( res < 1 ) {
              fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
              fclose(rawFile);
              return 1;
            }
						ptr++;
					}
				}
			}
			else if(image->comps[compno].sgnd == 0)
			{
				unsigned char curr;
				int mask = (1 << image->comps[compno].prec) - 1;
				ptr = image->comps[compno].data;
				for (line = 0; line < h; line++) {
					for(row = 0; row < w; row++)	{	
						curr = (unsigned char) (*ptr & mask);
						res = fwrite(&curr, sizeof(unsigned char), 1, rawFile);
            if( res < 1 ) {
              fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
              fclose(rawFile);
              return 1;
            }
						ptr++;
					}
				}
			}
		}
		else if(image->comps[compno].prec <= 16)
		{
			if(image->comps[compno].sgnd == 1)
			{
				signed short int curr;
				int mask = (1 << image->comps[compno].prec) - 1;
				ptr = image->comps[compno].data;
				for (line = 0; line < h; line++) {
					for(row = 0; row < w; row++)	{					
						unsigned char temp;
						curr = (signed short int) (*ptr & mask);
						temp = (unsigned char) (curr >> 8);
						res = fwrite(&temp, 1, 1, rawFile);
            if( res < 1 ) {
              fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
              fclose(rawFile);
              return 1;
            }
						temp = (unsigned char) curr;
						res = fwrite(&temp, 1, 1, rawFile);
            if( res < 1 ) {
              fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
              fclose(rawFile);
              return 1;
            }
						ptr++;
					}
				}
			}
			else if(image->comps[compno].sgnd == 0)
			{
				unsigned short int curr;
				int mask = (1 << image->comps[compno].prec) - 1;
				ptr = image->comps[compno].data;
				for (line = 0; line < h; line++) {
					for(row = 0; row < w; row++)	{				
						unsigned char temp;
						curr = (unsigned short int) (*ptr & mask);
						temp = (unsigned char) (curr >> 8);
						res = fwrite(&temp, 1, 1, rawFile);
            if( res < 1 ) {
              fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
              fclose(rawFile);
              return 1;
            }
						temp = (unsigned char) curr;
						res = fwrite(&temp, 1, 1, rawFile);
            if( res < 1 ) {
              fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
              fclose(rawFile);
              return 1;
            }
						ptr++;
					}
				}
			}
		}
		else if (image->comps[compno].prec <= 32)
		{
			fprintf(stderr,"More than 16 bits per component no handled yet\n");
			fclose(rawFile);
			return 1;
		}
		else
		{
			fprintf(stderr,"Error: invalid precision: %d\n", image->comps[compno].prec);
			fclose(rawFile);
			return 1;
		}
	}
	fclose(rawFile);
	return 0;
}

#ifdef HAVE_LIBPNG

#define PNG_MAGIC "\x89PNG\x0d\x0a\x1a\x0a"
#define MAGIC_SIZE 8
/* PNG allows bits per sample: 1, 2, 4, 8, 16 */

opj_image_t *pngtoimage(const char *read_idf, opj_cparameters_t * params)
{
	png_structp  png;
	png_infop    info;
	double gamma, display_exponent;
	int bit_depth, interlace_type,compression_type, filter_type;
	int unit;
	png_uint_32 resx, resy;
	unsigned int i, j;
	png_uint_32  width, height;
	int color_type, has_alpha, is16;
	unsigned char *s;
	FILE *reader;
	unsigned char **rows;
/* j2k: */
	opj_image_t *image;
	opj_image_cmptparm_t cmptparm[4];
	int sub_dx, sub_dy;
	unsigned int nr_comp;
	int *r, *g, *b, *a;
	unsigned char sigbuf[8];

	if((reader = fopen(read_idf, "rb")) == NULL)
   {
	fprintf(stderr,"pngtoimage: can not open %s\n",read_idf);
	return NULL;
   }
	image = NULL; png = NULL; rows = NULL;

	if(fread(sigbuf, 1, MAGIC_SIZE, reader) != MAGIC_SIZE
	|| memcmp(sigbuf, PNG_MAGIC, MAGIC_SIZE) != 0)
   {
	fprintf(stderr,"pngtoimage: %s is no valid PNG file\n",read_idf);
	goto fin;
   }
/* libpng-VERSION/example.c: 
 * PC : screen_gamma = 2.2;
 * Mac: screen_gamma = 1.7 or 1.0;
*/
	display_exponent = 2.2;

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
	if(color_type == PNG_COLOR_TYPE_PALETTE)
	  png_set_expand(png);
	else
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	  png_set_expand(png);

	if(png_get_valid(png, info, PNG_INFO_tRNS))
	  png_set_expand(png);

	is16 = (bit_depth == 16);

/* GRAY => RGB; GRAY_ALPHA => RGBA
*/
	if(color_type == PNG_COLOR_TYPE_GRAY
	|| color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
   {
	png_set_gray_to_rgb(png);
	color_type = 
	 (color_type == PNG_COLOR_TYPE_GRAY? PNG_COLOR_TYPE_RGB:
		PNG_COLOR_TYPE_RGB_ALPHA);
   }
	if( !png_get_gAMA(png, info, &gamma))
	  gamma = 0.45455;

	png_set_gamma(png, display_exponent, gamma);

	png_read_update_info(png, info);

	png_get_pHYs(png, info, &resx, &resy, &unit);

	color_type = png_get_color_type(png, info);

	has_alpha = (color_type == PNG_COLOR_TYPE_RGB_ALPHA);

	nr_comp = 3 + has_alpha;

	bit_depth = png_get_bit_depth(png, info);

	rows = (unsigned char**)calloc(height+1, sizeof(unsigned char*));
	for(i = 0; i < height; ++i)
	 rows[i] = (unsigned char*)malloc(png_get_rowbytes(png,info));

	png_read_image(png, rows);

	memset(&cmptparm, 0, 4 * sizeof(opj_image_cmptparm_t));

	sub_dx = params->subsampling_dx; sub_dy = params->subsampling_dy;

	for(i = 0; i < nr_comp; ++i)
   {
	cmptparm[i].prec = bit_depth;
/* bits_per_pixel: 8 or 16 */
	cmptparm[i].bpp = bit_depth;
	cmptparm[i].sgnd = 0;
	cmptparm[i].dx = sub_dx;
	cmptparm[i].dy = sub_dy;
	cmptparm[i].w = width;
	cmptparm[i].h = height;
   }

	image = opj_image_create(nr_comp, &cmptparm[0], CLRSPC_SRGB);

	if(image == NULL) goto fin;

    image->x0 = params->image_offset_x0;
    image->y0 = params->image_offset_y0;
    image->x1 = image->x0 + (width  - 1) * sub_dx + 1 + image->x0;
    image->y1 = image->y0 + (height - 1) * sub_dy + 1 + image->y0;

	r = image->comps[0].data;
	g = image->comps[1].data;
	b = image->comps[2].data;
	if(has_alpha) {
		a = image->comps[3].data;
	}

	for(i = 0; i < height; ++i)
   {
	s = rows[i];

	for(j = 0; j < width; ++j)
  {
	if(is16)
 {
	*r++ = s[0]<<8|s[1]; s += 2;

	*g++ = s[0]<<8|s[1]; s += 2;
	
	*b++ = s[0]<<8|s[1]; s += 2;
	
	if(has_alpha) { *a++ = s[0]<<8|s[1]; s += 2; }

	continue;
 }
	*r++ = *s++; *g++ = *s++; *b++ = *s++;

	if(has_alpha) *a++ = *s++;
  }
   }
fin:
	if(rows)
   {
	for(i = 0; i < height; ++i)
	 free(rows[i]);
	free(rows);
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
	prec = image->comps[0].prec;
	nr_comp = image->numcomps;

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
	
    width = image->comps[0].w;
    height = image->comps[0].h;

	red = image->comps[0].data;
	green = image->comps[1].data;
	blue = image->comps[2].data;

    sig_bit.red = sig_bit.green = sig_bit.blue = prec;

	if(has_alpha) 
  {
	sig_bit.alpha = prec;
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

	png_set_IHDR(png, info, width, height, prec, 
	 color_type,
	 PNG_INTERLACE_NONE,
	 PNG_COMPRESSION_TYPE_BASE,  PNG_FILTER_TYPE_BASE);
/*=============================*/
	png_write_info(png, info);
/*=============================*/
	if(prec < 8)
  {
	png_set_packing(png);
  }
    adjustR = (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
    adjustG = (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
    adjustB = (image->comps[2].sgnd ? 1 << (image->comps[2].prec - 1) : 0);

	row_buf = (unsigned char*)malloc(width * nr_comp * 2);

	for(y = 0; y < height; ++y)
  {
	d = row_buf;

	for(x = 0; x < width; ++x)
 {
		if(is16)
	   {
		v = *red + adjustR; ++red;
		
		if(force16) { v = (v<<ushift) + (v>>dshift); }

		*d++ = (unsigned char)(v>>8); *d++ = (unsigned char)v;

		v = *green + adjustG; ++green;
		
		if(force16) { v = (v<<ushift) + (v>>dshift); }

		*d++ = (unsigned char)(v>>8); *d++ = (unsigned char)v;

		v =  *blue + adjustB; ++blue;
		
		if(force16) { v = (v<<ushift) + (v>>dshift); }

		*d++ = (unsigned char)(v>>8); *d++ = (unsigned char)v;

		if(has_alpha)
	  {
		v = *alpha + adjustA; ++alpha;
		
		if(force16) { v = (v<<ushift) + (v>>dshift); }

		*d++ = (unsigned char)(v>>8); *d++ = (unsigned char)v;
	  }
		continue;
	   }/* if(is16) */

		v = *red + adjustR; ++red;

		if(force8) { v = (v<<ushift) + (v>>dshift); }

		*d++ = (unsigned char)(v & mask);

		v = *green + adjustG; ++green;

		if(force8) { v = (v<<ushift) + (v>>dshift); }

		*d++ = (unsigned char)(v & mask);

		v = *blue + adjustB; ++blue;

		if(force8) { v = (v<<ushift) + (v>>dshift); }

		*d++ = (unsigned char)(v & mask);

		if(has_alpha)
	   {
		v = *alpha + adjustA; ++alpha;

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

    sig_bit.gray = prec;
    sig_bit.red = sig_bit.green = sig_bit.blue = sig_bit.alpha = 0;
	alpha = NULL; adjustA = 0;
	color_type = PNG_COLOR_TYPE_GRAY;

    if(nr_comp == 2) 
  { 
	has_alpha = 1; sig_bit.alpha = prec;
	alpha = image->comps[1].data;
	color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
	adjustA = (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
  }
    width = image->comps[0].w;
    height = image->comps[0].h;

	png_set_IHDR(png, info, width, height, sig_bit.gray,
     color_type,
     PNG_INTERLACE_NONE,
     PNG_COMPRESSION_TYPE_BASE,  PNG_FILTER_TYPE_BASE);

	png_set_sBIT(png, info, &sig_bit);
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
	 malloc(width * nr_comp * sizeof(unsigned short));

	for(y = 0; y < height; ++y)
 {
	d = row_buf;

		for(x = 0; x < width; ++x)
	   {
		v = *red + adjustR; ++red;

		if(force16) { v = (v<<ushift) + (v>>dshift); }

		*d++ = (unsigned char)(v>>8); *d++ = (unsigned char)v;

		if(has_alpha)
	  {
		v = *alpha++;

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
	row_buf = (unsigned char*)calloc(width, nr_comp * 2);

	for(y = 0; y < height; ++y)
 {
	d = row_buf;

		for(x = 0; x < width; ++x)
	   {
		v = *red + adjustR; ++red;

		if(force8) { v = (v<<ushift) + (v>>dshift); }

		*d++ = (unsigned char)(v & mask);

		if(has_alpha)
	  {
		v = *alpha + adjustA; ++alpha;

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
#endif /* HAVE_LIBPNG */
