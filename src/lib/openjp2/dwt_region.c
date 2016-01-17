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
 * Copyright (c) 2007, Jonathan Ballard <dzonatas@dzonux.net>
 * Copyright (c) 2007, Callum Lerwick <seg@haxxed.com>
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


#include "opj_includes.h"

/** @defgroup DWT DWT - Implementation of a discrete wavelet transform */
/*@{*/


/** @name Local data structures */
/*@{*/

/*
Note: the letter s is used to denote even locations in a given dimension, while the letter
d is used to denote odd locations.

*/

typedef struct opj_dwt53 {
	OPJ_INT32* data;
	OPJ_INT32 d_dim;
	OPJ_INT32 s_dim;

	/* if dimension is odd, then number of even locations equals one more than number of
	odd locations.  So, odd_dimension == 1 in case of odd dimension */
	OPJ_INT32 odd_dimension;
} opj_dwt53_t;

/* process four coefficients at a time*/
typedef union {
	OPJ_FLOAT32	f[4];
} opj_coeff97_t;

typedef struct opj_dwt97 {
	opj_coeff97_t*	data ;
	OPJ_INT32		d_dim ;
	OPJ_INT32		s_dim ;
	OPJ_INT32		odd_dimension ;
} opj_dwt97_t ;

static const OPJ_FLOAT32 opj_dwt_alpha =  1.586134342f; /*  12994 */
static const OPJ_FLOAT32 opj_dwt_beta  =  0.052980118f; /*    434 */
static const OPJ_FLOAT32 opj_dwt_gamma = -0.882911075f; /*  -7233 */
static const OPJ_FLOAT32 opj_dwt_delta = -0.443506852f; /*  -3633 */

static const OPJ_FLOAT32 opj_K      = 1.230174105f; /*  10078 */
static const OPJ_FLOAT32 opj_c13318 = 1.625732422f;

/*@}*/


/** @name Local static functions */
/*@{*/
/**
Inverse lazy transform (horizontal)
*/
static void opj_dwt_region_interleave53_h(opj_dwt53_t* buffer_h, 
										OPJ_INT32 *tile_data);
/**
Inverse lazy transform (vertical)
*/
static void opj_dwt_region_interleave53_v(opj_dwt53_t* buffer_v, 
										OPJ_INT32 *tile_data,
										OPJ_INT32 stride);
/**
Inverse 5-3 data transform in 1-D
*/
static void opj_dwt_region_decode53_1d(opj_dwt53_t *buffer_v);



/* <summary>                             */
/* Inverse 9-7 data transform in 1-D. */
/* </summary>                            */
static void opj_region_decode97(opj_dwt97_t* restrict dwt);

static void opj_region_interleave97_h(opj_dwt97_t* restrict w,
											OPJ_FLOAT32* restrict tile_data,
											OPJ_INT32 stride, 
											OPJ_INT32 size);

static void opj_region_interleave97_v(opj_dwt97_t* restrict buffer_v ,
											OPJ_FLOAT32* restrict tile_data ,
											OPJ_INT32 stride,
											OPJ_INT32 nb_elts_read);

static void opj_region_decode97_scale(opj_coeff97_t* w, 
											OPJ_INT32 count,
											const OPJ_FLOAT32 c);

static void opj_region_decode97_lift(opj_coeff97_t* l,
											opj_coeff97_t* w, 
											OPJ_INT32 k,
											OPJ_INT32 m,
											OPJ_FLOAT32 c);



/*@}*/

/*@}*/

#define OPJ_S(i) a[(i)<<1]
#define OPJ_D(i) a[(1+((i)<<1))]
#define OPJ_S_(i) ((i)<0?OPJ_S(0):((i)>=s_dim?OPJ_S(s_dim-1):OPJ_S(i)))
#define OPJ_D_(i) ((i)<0?OPJ_D(0):((i)>=d_dim?OPJ_D(d_dim-1):OPJ_D(i)))


#define OPJ_SS_(i) ((i)<0?OPJ_S(0):((i)>=d_dim?OPJ_S(d_dim-1):OPJ_S(i)))
#define OPJ_DD_(i) ((i)<0?OPJ_D(0):((i)>=s_dim?OPJ_D(s_dim-1):OPJ_D(i)))

/* 
==========================================================
   local functions
==========================================================
*/

/* <summary>                             */
/* Inverse lazy transform (horizontal).  */
/* </summary>                            */
static void opj_dwt_region_interleave53_h(opj_dwt53_t* buffer_h, OPJ_INT32 *tile_data) {
	OPJ_INT32 *tile_data_ptr = tile_data;
	OPJ_INT32 *buffer_data_ptr = buffer_h->data + buffer_h->odd_dimension;
	OPJ_INT32  i = buffer_h->s_dim;
	while (i--) {
		*buffer_data_ptr = *(tile_data_ptr++);
		buffer_data_ptr += 2;
	}
	tile_data_ptr	= tile_data + buffer_h->s_dim;
	buffer_data_ptr = buffer_h->data + 1 - buffer_h->odd_dimension;
	i = buffer_h->d_dim;
	while (i--) {
		*buffer_data_ptr = *(tile_data_ptr++);
		buffer_data_ptr += 2;
	}
}

/* <summary>                             */
/* Inverse lazy transform (vertical).    */
/* </summary>                            */
static void opj_dwt_region_interleave53_v(opj_dwt53_t* buffer_v, OPJ_INT32 *tile_data, OPJ_INT32 stride) {
	OPJ_INT32 *tile_data_ptr = tile_data;
	OPJ_INT32 *buffer_data_ptr = buffer_v->data + buffer_v->odd_dimension;
	OPJ_INT32  i = buffer_v->s_dim;
	while (i--) {
		*buffer_data_ptr	= *tile_data_ptr;
		buffer_data_ptr		+= 2;
		tile_data_ptr		+= stride;
	}
	tile_data_ptr	= tile_data + (buffer_v->s_dim * stride);
	buffer_data_ptr = buffer_v->data + 1 - buffer_v->odd_dimension;
	i = buffer_v->d_dim;
	while (i--) {
		*buffer_data_ptr	= *tile_data_ptr;
		buffer_data_ptr		+= 2;
		tile_data_ptr		+= stride;
	}
}


/* <summary>                            */
/* Inverse 5-3 data transform in 1-D. */
/* </summary>                           */ 
static void opj_dwt_region_decode53_1d(opj_dwt53_t *buffer_v) {
	OPJ_INT32 *a = buffer_v->data;
	OPJ_INT32 d_dim = buffer_v->d_dim;
	OPJ_INT32 s_dim = buffer_v->s_dim;
	OPJ_INT32 odd_dimension = buffer_v->odd_dimension;

	OPJ_INT32 i;

	if (!odd_dimension) {
		if ((d_dim > 0) || (s_dim > 1)) { 
			for (i = 0; i < s_dim; i++)
				OPJ_S(i) -= (OPJ_D_(i - 1) + OPJ_D_(i) + 2) >> 2;
			for (i = 0; i < d_dim; i++) 
				OPJ_D(i) += (OPJ_S_(i) + OPJ_S_(i + 1)) >> 1;
		}
	}
	else {
		if (!s_dim  && d_dim == 1)        
			OPJ_S(0) >>=1;
		else {
			for (i = 0; i < s_dim; i++) 
				OPJ_D(i) -= (OPJ_SS_(i) + OPJ_SS_(i + 1) + 2) >> 2;
			for (i = 0; i < d_dim; i++)
				OPJ_S(i) += (OPJ_DD_(i) + OPJ_DD_(i - 1)) >> 1;
		}
	}
}

/* 
==========================================================
   DWT interface
==========================================================
*/


/* <summary>                            */
/* Inverse 5-3 data transform in 2-D. */
/* </summary>                           */
OPJ_BOOL opj_dwt_region_decode53(opj_tcd_tilecomp_t* tilec, OPJ_UINT32 numres) {
	opj_dwt53_t buffer_h;
	opj_dwt53_t buffer_v;

	opj_tcd_resolution_t* tr = tilec->resolutions;

	OPJ_UINT32 rw = (OPJ_UINT32)(tr->x1 - tr->x0);	/* width of the resolution level computed */
	OPJ_UINT32 rh = (OPJ_UINT32)(tr->y1 - tr->y0);	/* height of the resolution level computed */

	OPJ_UINT32 w = (OPJ_UINT32)(tilec->x1 - tilec->x0);

	if (numres == 1U) {
		return OPJ_TRUE;
	}
	buffer_h.data = (OPJ_INT32*)opj_aligned_malloc(opj_dwt_max_resolution(tr, numres) * sizeof(OPJ_INT32));
	if (!buffer_h.data) {
		/* FIXME event manager error callback */
		return OPJ_FALSE;
	}

	buffer_v.data = buffer_h.data;

	while (--numres) {
		OPJ_INT32 * restrict tiledp = tilec->buf->data;
		OPJ_UINT32 j;

		++tr;
		buffer_h.s_dim = (OPJ_INT32)rw;
		buffer_v.s_dim = (OPJ_INT32)rh;

		rw = (OPJ_UINT32)(tr->x1 - tr->x0);
		rh = (OPJ_UINT32)(tr->y1 - tr->y0);

		buffer_h.d_dim = (OPJ_INT32)(rw - (OPJ_UINT32)buffer_h.s_dim);
		buffer_h.odd_dimension = tr->x0 &1;

		for (j = 0; j < rh; ++j) {
			opj_dwt_region_interleave53_h(&buffer_h, tiledp + j*w);
			opj_dwt_region_decode53_1d(&buffer_h);
			memcpy(tiledp + j*w, buffer_h.data, rw * sizeof(OPJ_INT32));
		}

		buffer_v.d_dim = (OPJ_INT32)(rh - (OPJ_UINT32)buffer_v.s_dim);
		buffer_v.odd_dimension = tr->y0 &1;

		for (j = 0; j < rw; ++j) {
			OPJ_UINT32 k;
			opj_dwt_region_interleave53_v(&buffer_v, tiledp + j, (OPJ_INT32)w);
			opj_dwt_region_decode53_1d(&buffer_v);
			for (k = 0; k < rh; ++k) {
				tiledp[k * w + j] = buffer_v.data[k];
			}
		}
	}
	opj_aligned_free(buffer_h.data);
	return OPJ_TRUE;
}

static void opj_region_interleave97_h(opj_dwt97_t* restrict buffer, 
											OPJ_FLOAT32* restrict tile_data,
											OPJ_INT32 stride, 
											OPJ_INT32 size){

	OPJ_FLOAT32* restrict buffer_data_ptr = (OPJ_FLOAT32*) (buffer->data + buffer->odd_dimension);
	OPJ_INT32 count = buffer->s_dim;
	OPJ_INT32 i, k;

	for(k = 0; k < 2; ++k){
		if ( count + 3 * stride < size && 
				((size_t) tile_data & 0x0f) == 0 &&
					((size_t) buffer_data_ptr & 0x0f) == 0 && 
									(stride & 0x0f) == 0 ) {
			/* Fast code path */
			for(i = 0; i < count; ++i){
				OPJ_INT32 j = i;
				buffer_data_ptr[i<<3    ] = tile_data[j];
				j += stride;

				buffer_data_ptr[(i<<3) + 1] = tile_data[j];
				j += stride;

				buffer_data_ptr[(i<<3) + 2] = tile_data[j];
				j += stride;

				buffer_data_ptr[(i<<3) + 3] = tile_data[j];
			}
		}
		else {
			/* Slow code path */
			for(i = 0; i < count; ++i){
				OPJ_INT32 j = i;

				buffer_data_ptr[i<<3    ] = tile_data[j];
				j += stride;
				if(j >= size) 
					continue;

				buffer_data_ptr[(i<<3) + 1] = tile_data[j];
				j += stride;
				if(j >= size) 
					continue;

				buffer_data_ptr[(i<<3) + 2] = tile_data[j];
				j += stride;
				if(j >= size) 
					continue;

				buffer_data_ptr[(i<<3) + 3] = tile_data[j]; 
			}
		}

		buffer_data_ptr = (OPJ_FLOAT32*) (buffer->data + 1 - buffer->odd_dimension);
		tile_data	+= buffer->s_dim;
		size		-= buffer->s_dim;
		count		= buffer->d_dim;
	}
}

static void opj_region_interleave97_v(opj_dwt97_t* restrict buffer_v , 
										OPJ_FLOAT32* restrict tile_data ,
										OPJ_INT32 stride, 
										OPJ_INT32 nb_elts_read){
	opj_coeff97_t* restrict buffer_data_ptr = buffer_v->data + buffer_v->odd_dimension;
	OPJ_INT32 i;

	for(i = 0; i < buffer_v->s_dim; ++i){
		memcpy(buffer_data_ptr + (i<<1), 
					tile_data + i*stride, 
						(size_t)nb_elts_read * sizeof(OPJ_FLOAT32));
	}

	tile_data += buffer_v->s_dim * stride;
	buffer_data_ptr = buffer_v->data + 1 - buffer_v->odd_dimension;

	for(i = 0; i < buffer_v->d_dim; ++i){
		memcpy(buffer_data_ptr + (i<<1), 
					tile_data + i*stride, 
						(size_t)nb_elts_read * sizeof(OPJ_FLOAT32));
	}
}

static void opj_region_decode97_scale(opj_coeff97_t* buffer, 
										OPJ_INT32 count, 
										const OPJ_FLOAT32 scale)
{
	OPJ_FLOAT32* restrict fw = (OPJ_FLOAT32*) buffer;
	OPJ_INT32 i;
	for(i = 0; i < count; ++i){
		fw[(i<<3)    ] *= scale;
		fw[(i<<3) + 1] *= scale;
		fw[(i<<3) + 2] *= scale;
		fw[(i<<3) + 3] *= scale;
	}
}

static void opj_region_decode97_lift(opj_coeff97_t* l, 
											opj_coeff97_t* w,
											OPJ_INT32 k,
											OPJ_INT32 m,
											OPJ_FLOAT32 c)
{
	OPJ_FLOAT32* fl = (OPJ_FLOAT32*) l;
	OPJ_FLOAT32* fw = (OPJ_FLOAT32*) w;
	OPJ_INT32 i;
	for(i = 0; i < m; ++i){
		OPJ_FLOAT32 tmp1_1 = fl[0];
		OPJ_FLOAT32 tmp1_2 = fl[1];
		OPJ_FLOAT32 tmp1_3 = fl[2];
		OPJ_FLOAT32 tmp1_4 = fl[3];
		OPJ_FLOAT32 tmp2_1 = fw[-4];
		OPJ_FLOAT32 tmp2_2 = fw[-3];
		OPJ_FLOAT32 tmp2_3 = fw[-2];
		OPJ_FLOAT32 tmp2_4 = fw[-1];
		OPJ_FLOAT32 tmp3_1 = fw[0];
		OPJ_FLOAT32 tmp3_2 = fw[1];
		OPJ_FLOAT32 tmp3_3 = fw[2];
		OPJ_FLOAT32 tmp3_4 = fw[3];
		fw[-4] = tmp2_1 + ((tmp1_1 + tmp3_1) * c);
		fw[-3] = tmp2_2 + ((tmp1_2 + tmp3_2) * c);
		fw[-2] = tmp2_3 + ((tmp1_3 + tmp3_3) * c);
		fw[-1] = tmp2_4 + ((tmp1_4 + tmp3_4) * c);
		fl = fw;
		fw += 8;
	}
	if(m < k){
		OPJ_FLOAT32 c1;
		OPJ_FLOAT32 c2;
		OPJ_FLOAT32 c3;
		OPJ_FLOAT32 c4;
		c += c;
		c1 = fl[0] * c;
		c2 = fl[1] * c;
		c3 = fl[2] * c;
		c4 = fl[3] * c;
		for(; m < k; ++m){
			OPJ_FLOAT32 tmp1 = fw[-4];
			OPJ_FLOAT32 tmp2 = fw[-3];
			OPJ_FLOAT32 tmp3 = fw[-2];
			OPJ_FLOAT32 tmp4 = fw[-1];
			fw[-4] = tmp1 + c1;
			fw[-3] = tmp2 + c2;
			fw[-2] = tmp3 + c3;
			fw[-1] = tmp4 + c4;
			fw += 8;
		}
	}
}


/* <summary>                             */
/* Inverse 9-7 data transform in 1-D. */
/* </summary>                            */
static void opj_region_decode97(opj_dwt97_t* restrict dwt)
{
	/* a,b are either 0 or 1 */
	OPJ_INT32 a = dwt->odd_dimension != 0;
	OPJ_INT32 b = dwt->odd_dimension == 0;

	if (!((dwt->d_dim > a) || (dwt->s_dim > b))) {
		return;
	}

	opj_region_decode97_scale(dwt->data+a, dwt->s_dim, opj_K);
	opj_region_decode97_scale(dwt->data+b, dwt->d_dim, opj_c13318);
	opj_region_decode97_lift(dwt->data+b, dwt->data+a+1, dwt->s_dim, opj_int_min(dwt->s_dim, dwt->d_dim-a), opj_dwt_delta);
	opj_region_decode97_lift(dwt->data+a, dwt->data+b+1, dwt->d_dim, opj_int_min(dwt->d_dim, dwt->s_dim-b), opj_dwt_gamma);
	opj_region_decode97_lift(dwt->data+b, dwt->data+a+1, dwt->s_dim, opj_int_min(dwt->s_dim, dwt->d_dim-a), opj_dwt_beta);
	opj_region_decode97_lift(dwt->data+a, dwt->data+b+1, dwt->d_dim, opj_int_min(dwt->d_dim, dwt->s_dim-b), opj_dwt_alpha);

}


/* <summary>                             */
/* Inverse 9-7 data transform in 2-D. */
/* </summary>                            */
OPJ_BOOL opj_dwt_region_decode97(opj_tcd_tilecomp_t* restrict tilec, OPJ_UINT32 numres)
{
	opj_dwt97_t buffer_h;
	opj_dwt97_t buffer_v;

	opj_tcd_resolution_t* res = tilec->resolutions;

	/* start with lowest resolution */
	OPJ_UINT32 rw = (OPJ_UINT32)(res->x1 - res->x0);	/* width of the resolution level computed */
	OPJ_UINT32 rh = (OPJ_UINT32)(res->y1 - res->y0);	/* height of the resolution level computed */

	OPJ_UINT32 w = (OPJ_UINT32)(tilec->x1 - tilec->x0);

	buffer_h.data = (opj_coeff97_t*) opj_aligned_malloc((opj_dwt_max_resolution(res, numres)) * sizeof(opj_coeff97_t));
	if (!buffer_h.data) {
		/* FIXME event manager error callback */
		return OPJ_FALSE;
	}
	/* share data buffer between vertical and horizontal lifting steps*/
	buffer_v.data = buffer_h.data;

	while( --numres) {
		OPJ_FLOAT32 * restrict tile_data = (OPJ_FLOAT32*) tilec->buf->data;
		OPJ_UINT32 bufsize = (OPJ_UINT32)((tilec->x1 - tilec->x0) * (tilec->y1 - tilec->y0));
		OPJ_INT32 j;

		buffer_h.s_dim = (OPJ_INT32)rw;
		buffer_v.s_dim = (OPJ_INT32)rh;

		++res;

		/* next higher resolution */
		rw = (OPJ_UINT32)(res->x1 - res->x0);	/* width of the resolution level computed */
		rh = (OPJ_UINT32)(res->y1 - res->y0);	/* height of the resolution level computed */

		buffer_h.d_dim = (OPJ_INT32)(rw - (OPJ_UINT32)buffer_h.s_dim);
		buffer_h.odd_dimension = res->x0 &1;

		for(j = (OPJ_INT32)rh; j > 3; j -= 4) {
			OPJ_INT32 k;
			opj_region_interleave97_h(&buffer_h, tile_data, (OPJ_INT32)w, (OPJ_INT32)bufsize);
			opj_region_decode97(&buffer_h);

			for(k = (OPJ_INT32)rw; --k >= 0;){
				tile_data[k               ]		= buffer_h.data[k].f[0];
				tile_data[k+(OPJ_INT32)w  ]		= buffer_h.data[k].f[1];
				tile_data[k+((OPJ_INT32)w<<1)]	= buffer_h.data[k].f[2];
				tile_data[k+(OPJ_INT32)w*3]		= buffer_h.data[k].f[3];
			}

			tile_data += w<<2;
			bufsize -= w<<2;
		}

		if (j > 0) {
			OPJ_INT32 k;
			opj_region_interleave97_h(&buffer_h, tile_data, (OPJ_INT32)w, (OPJ_INT32)bufsize);
			opj_region_decode97(&buffer_h);
			for(k = (OPJ_INT32)rw; --k >= 0;){
				switch(j) {
					case 3: 
						tile_data[k+((OPJ_INT32)w<<1)] = buffer_h.data[k].f[2];
					case 2: 
						tile_data[k+(OPJ_INT32)w  ]		= buffer_h.data[k].f[1];
					case 1: 
						tile_data[k               ]		= buffer_h.data[k].f[0];
				}
			}
		}

		buffer_v.d_dim = (OPJ_INT32)(rh - (OPJ_UINT32)buffer_v.s_dim);
		buffer_v.odd_dimension = res->y0 &1;

		tile_data = (OPJ_FLOAT32*) tilec->buf->data;
		for(j = (OPJ_INT32)rw; j > 3; j -= 4){
			OPJ_UINT32 k;

			opj_region_interleave97_v(&buffer_v, tile_data, (OPJ_INT32)w, 4);
			opj_region_decode97(&buffer_v);

			for(k = 0; k < rh; ++k){
				memcpy(tile_data +k*w, buffer_v.data+k, 4 * sizeof(OPJ_FLOAT32));
			}
			tile_data += 4;
		}

		if (j > 0){
			OPJ_UINT32 k;
			opj_region_interleave97_v(&buffer_v, tile_data, (OPJ_INT32)w, j);
			opj_region_decode97(&buffer_v);

			for(k = 0; k < rh; ++k){
				memcpy(tile_data + k*w, buffer_v.data+k, (size_t)j * sizeof(OPJ_FLOAT32));
			}
		}
	}

	opj_aligned_free(buffer_h.data);
	return OPJ_TRUE;
}
