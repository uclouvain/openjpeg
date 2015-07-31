#include <config.h>
/*
 * Copyright (c) 2002-2007, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2007, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux and Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "openjpeg.h"
#include "opj_apps_config.h"

#ifdef OPJ_HAVE_LIBLCMS2
#include <lcms2.h>
#endif
#ifdef OPJ_HAVE_LIBLCMS1
#include <lcms.h>
#endif

#include "viewerdefs.hh"
#include "color.hh"

//#define DEBUG_SYCC

/*--------------------------------------------------------
Matrix für sYCC, Amendment 1 to IEC 61966-2-1

Y :   0.299   0.587    0.114   :R
Cb:  -0.1687 -0.3312   0.5     :G
Cr:   0.5    -0.4187  -0.0812  :B

Inverse:

R: 1        -3.68213e-05    1.40199      :Y
G: 1.00003  -0.344125      -0.714128     :Cb - 2^(prec - 1)
B: 0.999823  1.77204       -8.04142e-06  :Cr - 2^(prec - 1)

-----------------------------------------------------------*/
static void sycc_to_rgb(int offset, int upb, int y, int cb, int cr,
	int *out_r, int *out_g, int *out_b)
{
	int r, g, b;

	cb -= offset; cr -= offset;
	r = y + (int)((float)1.402 * (float)cr);
	if(r < 0) r = 0; else if(r > upb) r = upb; 
	*out_r = r;

	g = y - (int)((float)0.344 * (float)cb + (float)0.714 * (float)cr);
	if(g < 0) g = 0; else if(g > upb) g = upb; 
	*out_g = g;

	b = y + (int)((float)1.772 * (float)cb);
	if(b < 0) b = 0; else if(b > upb) b = upb; 
	*out_b = b;
}

static void sycc444_to_rgb(opj_image_t *image, ImageInfo *dst)
{
	int *r, *g, *b;
	const int *y, *cb, *cr;
	int maxw, maxh, max, i, offset, upb;
	size_t imax;

	i = image->comps[0].prec;
	offset = 1<<(i - 1); upb = (1<<i)-1;

	maxw = image->comps[0].w; maxh = image->comps[0].h;
	max = maxw * maxh;
	imax = max * sizeof(int);

	y = image->comps[0].data;
	cb = image->comps[1].data;
	cr = image->comps[2].data;

	dst->red = r = (int*)malloc(imax);
	dst->green = g = (int*)malloc(imax);
	dst->blue = b = (int*)malloc(imax);

	for(i = 0; i < max; ++i)
   {
	sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);	

	++y; ++cb; ++cr; ++r; ++g; ++b;
   }
	dst->free_red = 1;
	dst->free_green = 1;
	dst->free_blue = 1;
	dst->numcomps = 3;
	dst->color_space = OPJ_CLRSPC_SRGB;

	dst->dx[1] = dst->dx[0];
	dst->dx[2] = dst->dx[0];

	dst->dy[1] = dst->dy[0];
	dst->dy[2] = dst->dy[0];

	dst->prec[1] = dst->prec[0];
	dst->prec[2] = dst->prec[0];
}// sycc444_to_rgb()

static void sycc422_to_rgb(opj_image_t *image, ImageInfo *dst)
{	
	int *r, *g, *b;
	const int *y, *cb, *cr;
	int maxw, maxh, max, offset, upb;
	int i, j;
	size_t imax;

	i = image->comps[0].prec;
	offset = 1<<(i - 1); upb = (1<<i)-1;

	maxw = image->comps[0].w; 
	maxh = image->comps[0].h;
	max = maxw * maxh; 
	imax = max *sizeof(int);

	y = image->comps[0].data;
	cb = image->comps[1].data;
	cr = image->comps[2].data;

	dst->red = r = (int*)malloc(imax);
	dst->green = g = (int*)malloc(imax);
	dst->blue = b = (int*)malloc(imax);

	for(i = 0; i < maxh; ++i)
   {
	for(j = 0; j < (maxw & ~1); j += 2)
  {
	sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);

	++y; ++r; ++g; ++b;

	sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);

	++y; ++r; ++g; ++b; ++cb; ++cr;
  }
	if(j < maxw) 
  {
	sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);

	++y; ++r; ++g; ++b; ++cb; ++cr;
  }
   }
	dst->free_red = 1;
	dst->free_green = 1;
	dst->free_blue = 1;
	dst->color_space = OPJ_CLRSPC_SRGB;
	dst->numcomps = 3;

	dst->dx[1] = dst->dx[0];
	dst->dx[2] = dst->dx[0];

	dst->dy[1] = dst->dy[0];
	dst->dy[2] = dst->dy[0];

	dst->prec[1] = dst->prec[0];
	dst->prec[2] = dst->prec[0];

	image->color_space = OPJ_CLRSPC_SRGB;
	image->comps[1].w = maxw; image->comps[1].h = maxh;
	image->comps[2].w = maxw; image->comps[2].h = maxh;
	image->comps[1].dx = image->comps[0].dx;
	image->comps[2].dx = image->comps[0].dx;
	image->comps[1].dy = image->comps[0].dy;
	image->comps[2].dy = image->comps[0].dy;

}// sycc422_to_rgb()

static void sycc420_to_rgb(opj_image_t *image, ImageInfo *dst)
{
	int *R, *G, *B, *nr, *ng, *nb;
	const int *y, *cb, *cr, *ny;
	int maxw, maxh, max, offset, upb;
	int i, j;
	size_t imax;

	i = image->comps[0].prec;
	offset = 1<<(i - 1); upb = (1<<i)-1;

	maxw = image->comps[0].w; maxh = image->comps[0].h;
	max = maxw * maxh;
	imax = sizeof(int) * max;

	y = image->comps[0].data;
	cb = image->comps[1].data;
	cr = image->comps[2].data;

	R = dst->red = (int*)malloc(imax);
	G = dst->green = (int*)malloc(imax);
	B = dst->blue = (int*)malloc(imax);
#ifdef DEBUG_SYCC
fprintf(stderr,"%s:%d:\n\tsycc420_to_rgb\n\tis_still(%d) W(%d) H(%d)\n"
"\tRED(%p) GREEN(%p) BLUE(%p)\n",__FILE__,__LINE__,
dst->is_still,maxw,maxh,(void*)dst->red,(void*)dst->green,
(void*)dst->blue);
#endif

	for(i=0; i < (maxh & ~1); i += 2)
   {
	ny = y + maxw;
	nr = R + maxw; ng = G + maxw; nb = B + maxw;

	for(j=0; j < (maxw & ~1);  j += 2)
  {
	sycc_to_rgb(offset, upb, *y, *cb, *cr, R, G, B);

	++y; ++R; ++G; ++B;

	sycc_to_rgb(offset, upb, *y, *cb, *cr, R, G, B);

	++y; ++R; ++G; ++B;

	sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);

	++ny; ++nr; ++ng; ++nb;

	sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);

	++ny; ++nr; ++ng; ++nb; ++cb; ++cr;
  }

	if(j < maxw)
  {
	sycc_to_rgb(offset, upb, *y, *cb, *cr, R, G, B);

	++y; ++R; ++G; ++B;

	sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);

	++ny; ++nr; ++ng; ++nb; ++cb; ++cr;
  }

	y += maxw; R += maxw; G += maxw; B += maxw;
   }

	if(i < maxh)
   {
	for(j = 0; j < (maxw & ~1); j += 2)
  {
	sycc_to_rgb(offset, upb, *y, *cb, *cr, R, G, B);

	++y; ++R; ++G; ++B;

	sycc_to_rgb(offset, upb, *y, *cb, *cr, R, G, B);

	++y; ++R; ++G; ++B; ++cb; ++cr;
  }
	if(j < maxw)
  {
	sycc_to_rgb(offset, upb, *y, *cb, *cr, R, G, B);
  }
   }

	dst->free_red = 1;
	dst->free_green = 1;
	dst->free_blue = 1;
	dst->numcomps = 3;
	dst->color_space = OPJ_CLRSPC_SRGB;

	dst->dx[1] = dst->dx[0];
	dst->dx[2] = dst->dx[0];

	dst->dy[1] = dst->dy[0];
	dst->dy[2] = dst->dy[0];

	dst->prec[1] = dst->prec[0];
	dst->prec[2] = dst->prec[0];

	image->color_space = OPJ_CLRSPC_SRGB;
	image->comps[1].w = maxw; image->comps[1].h = maxh;
	image->comps[2].w = maxw; image->comps[2].h = maxh;
	image->comps[1].dx = image->comps[0].dx;
	image->comps[2].dx = image->comps[0].dx;
	image->comps[1].dy = image->comps[0].dy;
	image->comps[2].dy = image->comps[0].dy;

}// sycc420_to_rgb()

void COLOR_sycc_to_rgb(opj_image_t *image, ImageInfo *dst)
{
#ifdef DEBUG_SYCC
fprintf(stderr,"%s:%d:COLOR_sycc_to_rgb\n",
__FILE__,__LINE__);
 fprintf(stderr,"\tcomponents(%d) dx(%d,%d,%d) dy(%d,%d,%d)\n",
 image->numcomps,
 image->comps[0].dx,image->comps[1].dx,image->comps[2].dx,
 image->comps[0].dy,image->comps[1].dy,image->comps[2].dy);
#endif

	if(image->numcomps < 3) 
   {
	image->color_space = OPJ_CLRSPC_GRAY;
	dst->color_space = OPJ_CLRSPC_GRAY;
	dst->numcomps = image->numcomps;

	return;
   }

	if((image->comps[0].dx == 1)
	&& (image->comps[1].dx == 2)
	&& (image->comps[2].dx == 2)
	&& (image->comps[0].dy == 1)
	&& (image->comps[1].dy == 2)
	&& (image->comps[2].dy == 2))// horizontal and vertical sub-sample
   {
	sycc420_to_rgb(image, dst);
	return;
   }

	if((image->comps[0].dx == 1)
	&& (image->comps[1].dx == 2)
	&& (image->comps[2].dx == 2)
	&& (image->comps[0].dy == 1)
	&& (image->comps[1].dy == 1)
	&& (image->comps[2].dy == 1))// horizontal sub-sample only
   {
	sycc422_to_rgb(image, dst);
	return;
   }

	if((image->comps[0].dx == 1)
	&& (image->comps[1].dx == 1)
	&& (image->comps[2].dx == 1)
	&& (image->comps[0].dy == 1)
	&& (image->comps[1].dy == 1)
	&& (image->comps[2].dy == 1))// no sub-sample
   {
	sycc444_to_rgb(image, dst);
	return;
   }

	if((image->comps[0].dx == 4)
	&& (image->comps[1].dx == 4)
	&& (image->comps[2].dx == 4)
	&& (image->comps[0].dy == 4)
	&& (image->comps[1].dy == 4)
	&& (image->comps[2].dy == 4))
   {
// OK
	fprintf(stderr,"%s:%d:COLOR_sycc_to_rgb\n\tNO COVERSION\n",
	 __FILE__,__LINE__);
    fprintf(stderr,"\tcomponents(%d) dx(%d,%d,%d) dy(%d,%d,%d)\n",
    image->numcomps,
    image->comps[0].dx,image->comps[1].dx,image->comps[2].dx,
    image->comps[0].dy,image->comps[1].dy,image->comps[2].dy);

	return;	
   }
	fprintf(stderr,"%s:%d:COLOR_sycc_to_rgb\n\tCAN NOT CONVERT\n",
	 __FILE__,__LINE__);
	fprintf(stderr,"\tcomponents(%d) dx(%d,%d,%d) dy(%d,%d,%d)\n",
	image->numcomps,
	image->comps[0].dx,image->comps[1].dx,image->comps[2].dx,
	image->comps[0].dy,image->comps[1].dy,image->comps[2].dy);


}// COLOR_sycc_to_rgb()

#if defined(OPJ_HAVE_LIBLCMS2) || defined(OPJ_HAVE_LIBLCMS1)

#ifdef OPJ_HAVE_LIBLCMS1
// Bob Friesenhahn proposed:
//
#define cmsSigXYZData   icSigXYZData
#define cmsSigLabData   icSigLabData
#define cmsSigCmykData  icSigCmykData
#define cmsSigYCbCrData icSigYCbCrData
#define cmsSigLuvData   icSigLuvData
#define cmsSigGrayData  icSigGrayData
#define cmsSigRgbData   icSigRgbData
#define cmsUInt32Number DWORD
#define cmsUInt16Number WORD

#define cmsColorSpaceSignature icColorSpaceSignature
#define cmsGetHeaderRenderingIntent cmsTakeRenderingIntent

#endif // OPJ_HAVE_LIBLCMS1

//#define DEBUG_PROFILE

void COLOR_apply_icc_profile(opj_image_t *image, ImageInfo *dst)
{
	cmsHPROFILE in_prof, out_prof;
	cmsHTRANSFORM transform;
	cmsColorSpaceSignature out_space;
	cmsUInt32Number intent, in_type, out_type;
	size_t nr_samples;
	int *r, *g, *b;
	unsigned int prec, i, max, max_w, max_h, dmax, has_alpha2 = 0;
	OPJ_COLOR_SPACE oldspace;

#ifdef DEBUG_PROFILE
   {
	FILE *writer = fopen("debug.icm","wb");
	fwrite(image->icc_profile_buf, 1, image->icc_profile_len, writer);
	fclose(writer);
   }
#endif

	in_prof = 
	 cmsOpenProfileFromMem(image->icc_profile_buf, 
		image->icc_profile_len);

	if(in_prof == NULL) return ;

	(void)cmsGetPCS(in_prof);
	out_space = cmsGetColorSpace(in_prof);
	intent = cmsGetHeaderRenderingIntent(in_prof);
	
	max_w = image->comps[0].w; 
	max_h = image->comps[0].h;
	prec = image->comps[0].prec;

	oldspace = image->color_space;

	if(out_space == cmsSigRgbData) // enumCS 16
   {
	if( prec <= 8 )
  {
	in_type = TYPE_RGB_8;
	out_type = TYPE_RGB_8;
  }
	else
  {
	in_type = TYPE_RGB_16;
	out_type = TYPE_RGB_16;
  }
	out_prof = cmsCreate_sRGBProfile();
	image->color_space = OPJ_CLRSPC_SRGB;
   }
	else
	if(out_space == cmsSigGrayData) // enumCS 17
   {
	if( prec <= 8 )
  {
	in_type = TYPE_GRAY_8;
	out_type = TYPE_RGB_8;
  }
	else //prec > 8
  {
	in_type = TYPE_GRAY_16;
	out_type = TYPE_RGB_16;
  }
	out_prof = cmsCreate_sRGBProfile();
	image->color_space = OPJ_CLRSPC_SRGB;
   }
	else
	if(out_space == cmsSigYCbCrData) // enumCS 18
   {
	fprintf(stderr,"\n%s:%d:\n\tout_space == cmsSigYCbCrData NOT HANDLED\n\n",
	__FILE__,__LINE__);
	cmsCloseProfile(in_prof);
	return ;
#ifdef HIDDEN_CODE

	in_type = TYPE_YCbCr_16;
	out_type = TYPE_RGB_16;
	out_prof = cmsCreate_sRGBProfile();
	image->color_space = OPJ_CLRSPC_SRGB;

#endif //HIDDEN_CODE
   }
	else
   {
	cmsCloseProfile(in_prof);
	return ;
   }

	transform = cmsCreateTransform(in_prof, in_type,
	 out_prof, out_type, intent, 0);

#ifdef OPJ_HAVE_LIBLCMS2
// Possible for: LCMS_VERSION >= 2000 :
	cmsCloseProfile(in_prof);
	cmsCloseProfile(out_prof);
#endif

	if(transform == NULL)
   {
	image->color_space = oldspace;
#ifdef OPJ_HAVE_LIBLCMS1
	cmsCloseProfile(in_prof);
	cmsCloseProfile(out_prof);
#endif
	return ;
   }
	max = max_w * max_h;
	dmax = max * sizeof(int);

	if(image->numcomps > 2)// RGB, RGBA
   {
	if( prec <= 8 )
  {
	unsigned char *inbuf, *outbuf, *in, *out;

	nr_samples = 
	 (cmsUInt32Number)max * 3 * (cmsUInt32Number)sizeof(unsigned char);

	in = inbuf = (unsigned char*)malloc(nr_samples);
	out = outbuf = (unsigned char*)malloc(nr_samples);

	r = image->comps[0].data;
	g = image->comps[1].data;
	b = image->comps[2].data;
//  max = max_w * max_h; dmax = max * sizeof(int);

	for(i = 0; i < max; ++i)
 {
	*in++ = (unsigned char)*r++;
	*in++ = (unsigned char)*g++;
	*in++ = (unsigned char)*b++;
 }
	cmsDoTransform(transform, inbuf, outbuf, (cmsUInt32Number)max);

	r = dst->red = (int*)malloc(dmax);
	g = dst->green = (int*)malloc(dmax);
	b = dst->blue = (int*)malloc(dmax);

	dst->free_red = 1;
	dst->free_green = 1;
	dst->free_blue = 1;
	dst->numcomps = 3;
	dst->color_space = OPJ_CLRSPC_SRGB;

	for(i = 0; i < max; ++i)
 {
	*r++ = (int)*out++;
	*g++ = (int)*out++;
	*b++ = (int)*out++;
 }
	free(inbuf); free(outbuf);
  }
	else //prec > 8
  {
	unsigned short *inbuf, *outbuf, *in, *out;

	nr_samples = max * 3 * sizeof(unsigned short);

	in = inbuf = (unsigned short*)malloc(nr_samples);
	out = outbuf = (unsigned short*)malloc(nr_samples);

	r = image->comps[0].data;
	g = image->comps[1].data;
	b = image->comps[2].data;
//  max = max_w * max_h; dmax = max * sizeof(int);

	for(i = 0; i < max; ++i)
 {
	*in++ = (unsigned short)*r++;
	*in++ = (unsigned short)*g++;
	*in++ = (unsigned short)*b++;
 }
	cmsDoTransform(transform, inbuf, outbuf, max);

	r = dst->red = (int*)malloc(dmax);
	g = dst->green = (int*)malloc(dmax);
	b = dst->blue = (int*)malloc(dmax);

	dst->free_red = 1;
	dst->free_green = 1;
	dst->free_blue = 1;
	dst->numcomps = 3;
	dst->color_space = OPJ_CLRSPC_SRGB;

	for(i = 0; i < max; ++i)
 {
	*r++ = (int)*out++;
	*g++ = (int)*out++;
	*b++ = (int)*out++;
 }
	free(inbuf); free(outbuf);
  }
   }
	else // image->numcomps <= 2 : GRAY -> RGB, GRAYA -> RGBA
   {
	has_alpha2 = (image->numcomps == 2);

	if(prec <= 8)
  {
	unsigned char *in, *inbuf, *out, *outbuf;

	nr_samples = max * 3 * sizeof(unsigned char);

	in = inbuf = (unsigned char*)malloc(nr_samples);
	out = outbuf = (unsigned char*)malloc(nr_samples);

	r = image->comps[0].data;

	if(has_alpha2)
  {
	dst->alpha = image->comps[1].data;
	dst->prec[3] = image->comps[1].prec;
	dst->sgnd[3] = image->comps[1].sgnd;
	dst->has_alpha = 1;
  }
//	max = max_w * max_h; dmax = max * sizeof(int);

	for(i = 0; i < max; ++i)
 {
	*in++ = (unsigned char)*r++;
 }
	cmsDoTransform(transform, inbuf, outbuf, max);

	r = dst->red = (int*)malloc(dmax);
	g = dst->green = (int*)malloc(dmax);
	b = dst->blue = (int*)malloc(dmax);

	dst->free_red = 1;
	dst->free_green = 1;
	dst->free_blue = 1;
	dst->numcomps = 3 + has_alpha2;
	dst->color_space = OPJ_CLRSPC_SRGB;

	for(i = 0; i < max; ++i)
 {
	*r++ = (int)*out++; 
	*g++ = (int)*out++; 
	*b++ = (int)*out++;
 }
	free(inbuf); free(outbuf);
  }
	else // prec > 8
  {
    unsigned short *inbuf, *outbuf, *in, *out;

    nr_samples = 
	 (cmsUInt32Number)max * 3 * (cmsUInt32Number)sizeof(unsigned short);

    in = inbuf = (unsigned short*)malloc(nr_samples);
    out = outbuf = (unsigned short*)malloc(nr_samples);

    r = image->comps[0].data;

	if(has_alpha2)
  {
	dst->alpha = image->comps[1].data;
	dst->prec[3] = image->comps[1].prec;
	dst->sgnd[3] = image->comps[1].sgnd;
	dst->has_alpha = 1;
  }
//  max = max_w * max_h; dmax = max * sizeof(int);

    for(i = 0; i < max; ++i)
 {
    *in++ = (unsigned short)*r++;
 }
    cmsDoTransform(transform, inbuf, outbuf, (cmsUInt32Number)max);

    r = dst->red = (int*)malloc(dmax);
    g = dst->green = (int*)malloc(dmax);
    b = dst->blue = (int*)malloc(dmax);

	dst->free_red = 1;
	dst->free_green = 1;
	dst->free_blue = 1;
	dst->numcomps = 3 + has_alpha2;
	dst->color_space = OPJ_CLRSPC_SRGB;

    for(i = 0; i < max; ++i)
 {
    *r++ = (int)*out++; 
	*g++ = (int)*out++; 
	*b++ = (int)*out++;
 }
    free(inbuf); free(outbuf);
  }	//if(prec <= 8)

	dst->dx[0] = image->comps[0].dx;
	dst->dy[0] = image->comps[0].dy;
	dst->prec[0] = image->comps[0].prec;
	dst->sgnd[0] = image->comps[0].sgnd;

	dst->dx[1] = image->comps[0].dx;
	dst->dy[1] = image->comps[0].dy;
	dst->prec[1] = image->comps[0].prec;
	dst->sgnd[1] = image->comps[0].sgnd;

	dst->dx[2] = image->comps[0].dx;
	dst->dy[2] = image->comps[0].dy;
	dst->prec[2] = image->comps[0].prec;
	dst->sgnd[2] = image->comps[0].sgnd;

	if(has_alpha2 == 0)
  {
	dst->dx[3] = image->comps[0].dx;
	dst->dy[3] = image->comps[0].dy;
	dst->prec[3] = image->comps[0].prec;
	dst->sgnd[3] = image->comps[0].sgnd;
  }
   }// if(image->numcomps > 2)

	cmsDeleteTransform(transform);

#ifdef OPJ_HAVE_LIBLCMS1
	cmsCloseProfile(in_prof);
	cmsCloseProfile(out_prof);
#endif

}// COLOR_apply_icc_profile()

void COLOR_apply_conversion(opj_image_t *image, ImageInfo *dst)
{
	int *row;
	int enumcs, numcomps;

	image->color_space = OPJ_CLRSPC_SRGB;
	dst->color_space = OPJ_CLRSPC_SRGB;

	numcomps = dst->numcomps;

	if(numcomps != 3)
   {
	fprintf(stderr,"%s:%d:\n\tnumcomps %d not handled. Quitting.\n",
	 __FILE__,__LINE__,numcomps);
	return;
   }

	row = (int*)image->icc_profile_buf;
	enumcs = row[0];

	if(enumcs == 14)// CIELab
   {
	int *L, *a, *b, *red, *green, *blue;
	double rl, ol, ra, oa, rb, ob, prec0, prec1, prec2;
	double minL, maxL, mina, maxa, minb, maxb;
	unsigned int default_type;
	unsigned int i, max;
	cmsHPROFILE in, out;
	cmsHTRANSFORM transform;
	cmsUInt16Number RGB[3];
	cmsCIELab Lab;

	in = cmsCreateLab4Profile(NULL);
	out = cmsCreate_sRGBProfile();

	transform = 
	 cmsCreateTransform(in, TYPE_Lab_DBL, out, TYPE_RGB_16, 
	  INTENT_PERCEPTUAL, 0);

#ifdef OPJ_HAVE_LIBLCMS2
	cmsCloseProfile(in);
	cmsCloseProfile(out);
#endif
	if(transform == NULL)
  {
#ifdef OPJ_HAVE_LIBLCMS1
	cmsCloseProfile(in);
	cmsCloseProfile(out);
#endif
	return;
  }
	prec0 = (double)image->comps[0].prec;
	prec1 = (double)image->comps[1].prec;
	prec2 = (double)image->comps[2].prec;

	default_type = row[1];

	if(default_type == 0x44454600)// DEF : default
  {
	rl = 100; ra = 170; rb = 200;
	ol = 0;   
	oa = pow(2, prec1 - 1);
	ob = pow(2, prec2 - 2) +  pow(2, prec2 - 3);
  }
	else
  {
	rl = row[2]; ra = row[4]; rb = row[6];
	ol = row[3]; oa = row[5]; ob = row[7];
  }
	L = image->comps[0].data;
	a = image->comps[1].data;
	b = image->comps[2].data;

	max = image->comps[0].w * image->comps[0].h;

	red = dst->red = (int*)malloc(max * sizeof(int));
	green = dst->green = (int*)malloc(max * sizeof(int));
	blue = dst->blue = (int*)malloc(max * sizeof(int));

	minL = -(rl * ol)/(pow(2, prec0)-1);
	maxL = minL + rl;

	mina = -(ra * oa)/(pow(2, prec1)-1);
	maxa = mina + ra;

	minb = -(rb * ob)/(pow(2, prec2)-1);
	maxb = minb + rb;

	for(i = 0; i < max; ++i)
  {
	Lab.L = minL + (double)(*L) * (maxL - minL)/(pow(2, prec0)-1); ++L;
	Lab.a = mina + (double)(*a) * (maxa - mina)/(pow(2, prec1)-1); ++a;
	Lab.b = minb + (double)(*b) * (maxb - minb)/(pow(2, prec2)-1); ++b;

	cmsDoTransform(transform, &Lab, RGB, 1);

	*red++ = RGB[0];
	*green++ = RGB[1];
	*blue++ = RGB[2];
  }
	cmsDeleteTransform(transform);
#ifdef OPJ_HAVE_LIBLCMS1
	cmsCloseProfile(in);
	cmsCloseProfile(out);
#endif

	dst->free_red = 1;
	dst->free_green = 1;
	dst->free_blue = 1;
	dst->numcomps = 3;
	dst->color_space = OPJ_CLRSPC_SRGB;
	dst->prec[0] = 16;
	dst->prec[1] = 16;
	dst->prec[2] = 16;

	dst->alpha = NULL;
	dst->has_alpha = 0;

	image->color_space = OPJ_CLRSPC_SRGB;
	image->comps[0].prec = 16;
	image->comps[1].prec = 16;
	image->comps[2].prec = 16;

	return;
   }

	fprintf(stderr,"%s:%d:\n\tenumCS %d not handled. Ignoring.\n",
	 __FILE__,__LINE__, enumcs);

}// COLOR_apply_conversion()

#endif // OPJ_HAVE_LIBLCMS2 || OPJ_HAVE_LIBLCMS1

void COLOR_cmyk_to_rgb(opj_image_t *image, ImageInfo *dst)
{
	int *R, *G, *B;
	int *sc, *sm, *sy, *sk;
	float C, M, Y, K;
	unsigned int w, h, max, prec, len, i;

	w = image->comps[0].w;
	h = image->comps[0].h;
	prec = image->comps[0].prec;

	if(prec != 8) return;
	if(image->numcomps != 4) return;

	max = w * h;
	len = max * sizeof(int);

	R = dst->red = (int*)malloc(len);
	G = dst->green = (int*)malloc(len);
	B = dst->blue = (int*)malloc(len);

	sc = image->comps[0].data;
	sm = image->comps[1].data;
	sy = image->comps[2].data;
	sk = image->comps[3].data;

	for(i = 0; i < max; ++i)
   {
// CMYK and CMY values from 0 to 1 
//
	C = (float)(*sc++)/(float)255.; 
	M = (float)(*sm++)/(float)255; 
	Y = (float)(*sy++)/(float)255; 
	K = (float)(*sk++)/(float)255;

// CMYK -> CMY 
//
	C = ( C * ( (float)1. - K ) + K );
	M = ( M * ( (float)1. - K ) + K );
	Y = ( Y * ( (float)1. - K ) + K );

// CMY -> RGB : RGB results from 0 to 255 
//
	*R++ = (int)(unsigned char)(( (float)1. - C ) * (float)255.);
	*G++ = (int)(unsigned char)(( (float)1. - M ) * (float)255.);
	*B++ = (int)(unsigned char)(( (float)1. - Y ) * (float)255.);
   }

	dst->free_red = 1;
	dst->free_green = 1;
	dst->free_blue = 1;
	dst->numcomps = 3;
    dst->color_space = OPJ_CLRSPC_SRGB;

	image->color_space = OPJ_CLRSPC_SRGB;

}// COLOR_cmyk_to_rgb()

//
// This code has been adopted from sjpx_openjpeg.c of ghostscript
//
void COLOR_esycc_to_rgb(opj_image_t *image, ImageInfo *dst)
{
    int *s0, *s1, *s2;
    int *r, *g, *b;
    int y, cb, cr, sign1, sign2, val;
    unsigned int w, h, max, i;
    int flip_value = (1 << (image->comps[0].prec-1));
	int max_value = (~(-1 << image->comps[0].prec));

    if(dst->numcomps != 3) return;

    w = image->comps[0].w;
    h = image->comps[0].h;

    s0 = image->comps[0].data;
    s1 = image->comps[1].data;
    s2 = image->comps[2].data;

    sign1 = image->comps[1].sgnd;
    sign2 = image->comps[2].sgnd;

    max = w * h;

	r = dst->red = (int*)malloc(max * sizeof(int));
    g = dst->green = (int*)malloc(max * sizeof(int));
    b = dst->blue = (int*)malloc(max * sizeof(int));

    for(i = 0; i < max; ++i)
   {

    y = *s0++; cb = *s1++; cr = *s2++;

    if( !sign1) cb -= flip_value;
    if( !sign2) cr -= flip_value;

    val = (int)
	((float)y - (float)0.0000368 * (float)cb 
		+ (float)1.40199 * (float)cr + (float)0.5);

	if(val > max_value) val = max_value; else if(val < 0) val = 0;
    *r++ = val;

    val = (int)
	((float)1.0003 * (float)y - (float)0.344125 * (float)cb 
		- (float)0.7141128 * (float)cr + (float)0.5);

	if(val > max_value) val = max_value; else if(val < 0) val = 0;
    *g++ = val;

    val = (int)
	((float)0.999823 * (float)y + (float)1.77204 * (float)cb 
		- (float)0.000008 *(float)cr + (float)0.5);

	if(val > max_value) val = max_value; else if(val < 0) val = 0;
    *b++ = val;
   }
	dst->free_red = 1;
	dst->free_green = 1;
	dst->free_blue = 1;
	dst->numcomps = 3;
    dst->color_space = OPJ_CLRSPC_SRGB;

	image->color_space = OPJ_CLRSPC_SRGB;

}// COLOR_esycc_to_rgb()


#if defined(__cplusplus) || defined(c_plusplus)
           }
#endif
