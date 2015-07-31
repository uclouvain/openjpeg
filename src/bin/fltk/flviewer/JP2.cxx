#include <config.h>
/*
 * author(s) and license
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define snprintf _snprintf
#define sprintf _scprintf
#define strdup _strdup
#else /* not _WIN32 */
#define __STDC_FORMAT_MACROS
#include <stdint.h>
#include <stddef.h>
#endif /* _WIN32 */

#include <FL/fl_ask.H>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#include "openjpeg.h"
#include "opj_apps_config.h"
#include "color.hh"
           }
#else
#include "openjpeg.h"
#include "opj_apps_config.h"
#include "color.hh"
#endif /* __cplusplus */


#include "flviewer.hh"
#include "viewerdefs.hh"
#include "tree.hh"
#include "JP2.hh"
#include "lang/opj_lang.h_utf8"
#include "viewerdefs.hh"

//#define DEBUG_NUMCOMPS

#define READ_JP2 1
#define WRITE_JP2 0

static void dst_destroy(ImageInfo *dst)
{
	if(dst->free_red) free(dst->red);
	if(dst->free_green) free(dst->green);
	if(dst->free_blue) free(dst->blue);
//	do NOT free dst->alpha
}

static unsigned char *read_image(opj_image_t *image, ImageInfo *dst,
	int *out_type, unsigned int *out_w, unsigned int *out_h)
{
	unsigned char *dst_buf, *d;
	int *red, *green, *blue, *alpha;
	int compw, comph, max_comp, i, has_alpha, triple;
	size_t j, max;
	int shiftR, shiftG, shiftB, shiftA;
	int addR, addG, addB, addA;
	unsigned int mask = 0xffff;

	if(image->color_space == OPJ_CLRSPC_SYCC)
   {

fprintf(stderr,"%s:%d:\n\tINTERNAL ERROR\n",__FILE__,__LINE__);

	return NULL; /* handled in COLOR_sycc_to_rgb() */
   }
	has_alpha = 0; triple = 0;
	max_comp = dst->numcomps;

#ifdef DEBUG_NUMCOMPS
fprintf(stderr,"%s:%d:\n\tENTER read_image\n\tmax_comp(%d)"
" color_space(%d)\n",__FILE__,__LINE__,max_comp,dst->color_space);

fprintf(stderr,"\tDX(%d,%d,%d) DY(%d,%d,%d) PREC(%d,%d,%d) SIGNED(%d,%d,%d)\n",
dst->dx[0],dst->dx[1],dst->dx[2],
dst->dy[0],dst->dy[1],dst->dy[2],
dst->prec[0],dst->prec[1],dst->prec[2],
dst->sgnd[0],dst->sgnd[1],dst->sgnd[2]);

fprintf(stderr,"\tRED(%p) GREEN(%p) BLUE(%p) ALPHA(%p)\n\n",(void*)dst->red,
(void*)dst->green,(void*)dst->blue,(void*)dst->alpha);
#endif

	if(max_comp > 4) max_comp = 4;

    if(dst->color_space == OPJ_CLRSPC_GRAY
    && max_comp > 2)
   {
    max_comp -= 2;
   }

	if((max_comp <= 2)
	|| (max_comp > 2 && max_comp < 5
		&& dst->dx[0] == dst->dx[1]
		&& dst->dx[1] == dst->dx[2]
		&& dst->dy[0] == dst->dy[1]
		&& dst->dy[1] == dst->dy[2]
		&& dst->prec[0] == dst->prec[1]
		&& dst->prec[1] == dst->prec[2]
	  ))
   {
	int pix, ushift, dshift, prec0, force8;

	has_alpha = (max_comp == 4 || max_comp == 2);
	triple = (max_comp > 2);

	compw = image->comps[0].w; 
	comph = image->comps[0].h;
	max = compw * comph;
	shiftR = shiftG = shiftB = shiftA = 0;
	green = blue = alpha = NULL;
	prec0 = dst->prec[0];
	force8 = (prec0 < 8);

	if(prec0 > 16)
  {
	fprintf(stderr,"%s:%d:\n\tPREC(%d) not handled. Quitting.\n",
	__FILE__,__LINE__,prec0);
	return NULL;
  }
	dshift = ushift = 0;

	if(prec0 > 8)
	 shiftR = prec0 - 8;

	if(triple)
  {
	if(dst->prec[1] > 8)
	 shiftG = dst->prec[1] - 8;

	if(dst->prec[2] > 8)
	 shiftB = dst->prec[2] - 8;
  }

    if(force8)
  {
    ushift = 8 - prec0; dshift = prec0 - ushift;

	if(prec0 == 7) mask = 0x0000007f;
	else
	if(prec0 == 6) mask = 0x0000003f;
	else
	if(prec0 == 5) mask = 0x0000001f;
	else
    if(prec0 == 4) mask = 0x0000000f;
    else
	if(prec0 == 3) mask = 0x00000007;
	else
    if(prec0 == 2) mask = 0x00000003;
    else
/*    if(prec0 == 1) */ mask = 0x00000001;

  }
	addR = (dst->sgnd[0] ? 1<<(prec0 - 1) : 0);
	addG = addB = addA = 0;

	if(triple)
  {
	addG = 
	(dst->sgnd[1] ? 1<<(dst->prec[1] - 1) : 0);
	addB = 
	(dst->sgnd[2] ? 1<<(dst->prec[2] - 1) : 0);
  }

	if(has_alpha)
  {
	i = (triple?3:1);

	alpha = dst->alpha;

    if(alpha == NULL)
     has_alpha = 0;
    else
 {
	if(dst->prec[i] > 8)
	 shiftA = dst->prec[i] - 8;

	addA = 
	(dst->sgnd[i] ? 1<<(dst->prec[i] - 1) : 0);
 }
  }
	*out_type = (int)max_comp;

	*out_w = compw; *out_h = comph;

	d = dst_buf = (unsigned char*)malloc(max * max_comp);

	red = dst->red;

	if(triple)
  {
	green = dst->green;

	blue = dst->blue;
  }

	for(j = 0; j < max; ++j)
  {
    pix = *red++ + addR;

    if(force8)
 {
	pix = pix&mask;
    pix = (pix<<ushift) | (pix>>dshift);
 }
	else// prec >= 8 and <= 16
 {
	if(shiftR) pix = ((pix>>shiftR)+((pix>>(shiftR-1))%2));

	if(pix > 255) pix = 255; else if(pix < 0) pix = 0;
 }
    *d++ = (unsigned char)pix;

    if(triple)
 {
    pix = *green++ + addG;
        if(force8)
       {
		pix = pix&mask;
        pix = (pix<<ushift) | (pix>>dshift);
       }
		else// prec >= 8 and <= 16
       {
		if(shiftG) pix = ((pix>>shiftG)+((pix>>(shiftG-1))%2));

		if(pix > 255) pix = 255; else if(pix < 0) pix = 0;		
       }
    *d++ = (unsigned char) pix;

    pix = *blue++ + addB;
        if(force8)
       {
		pix = pix&mask;
        pix = (pix<<ushift) | (pix>>dshift);
       }
		else// prec >= 8 and <= 16
       {
		if(shiftB) pix = ((pix>>shiftB)+((pix>>(shiftB-1))%2));

		if(pix > 255) pix = 255; else if(pix < 0) pix = 0;
       }
    *d++ = (unsigned char) pix;
 }	/* if(triple) */

    if(has_alpha)
 {
    pix = *alpha++ + addA;
        if(force8)
       {
		pix = pix&mask;
        pix = (pix<<ushift) | (pix>>dshift);
       }
		else
       {
		if(shiftA) pix = ((pix>>shiftA)+((pix>>(shiftA-1))%2));

		if(pix > 255) pix = 255; else if(pix < 0) pix = 0;
       }
    *d++ = (unsigned char) pix;
 }
  }	//for(j = 0; j < max; ++j)

	return dst_buf;
   }// if((max_comp <= 2) ... )

	return NULL;

}// read_image()

static OPJ_CODEC_FORMAT ext_codec_format(const char *filename)
{
    unsigned int i;

    static const char *extension[] =
   {
    "j2k", "jp2", "j2c", "jpc"
   };

    static const OPJ_CODEC_FORMAT format[] =
   {
/*	J2K_CFMT,      JP2_CFMT,      J2K_CFMT,      J2K_CFMT */
	OPJ_CODEC_J2K, OPJ_CODEC_JP2, OPJ_CODEC_J2K, OPJ_CODEC_J2K
   };

    const char *ext = strrchr(filename, '.');
    if(ext)
   {
    ext++;
    if(*ext)
  {
    for(i = 0; i < sizeof(format)/sizeof(*format); i++)
 {
    if(strncasecmp(ext, extension[i], 3) == 0)
     return format[i];
 }
  }
   }
	return OPJ_CODEC_UNKNOWN;
}

static opj_image_t *to_opj_image(const unsigned char *buf, 
	int width, int height,
	int nr_comp, int sub_dx, int sub_dy)
{
	const unsigned char *cs;
	opj_image_t *image;
	int *r, *g, *b, *a;
	int has_rgba, has_graya, has_rgb, comp;
	unsigned int i, max;
	opj_image_cmptparm_t cmptparm[4];

	memset(&cmptparm, 0, 4 * sizeof(opj_image_cmptparm_t));

	for(comp = 0; comp < nr_comp; ++comp)
   {
	cmptparm[comp].prec = 8;
	cmptparm[comp].bpp = 8;
	cmptparm[comp].sgnd = 0;
	cmptparm[comp].dx = sub_dx;
	cmptparm[comp].dy = sub_dy;
	cmptparm[comp].w = width;
	cmptparm[comp].h = height;
   }

   {
	OPJ_COLOR_SPACE csp = (nr_comp > 2?OPJ_CLRSPC_SRGB:OPJ_CLRSPC_GRAY);

	image = opj_image_create(nr_comp, &cmptparm[0], csp);
   }

	if(image == NULL) 
   {
	fl_alert("%s", GOT_NO_IMAGE_s);

	return NULL;
   }

    image->x0 = 0;
    image->y0 = 0;
    image->x1 = (width - 1) * sub_dx + 1;
    image->y1 = (height - 1) * sub_dy + 1;

	r = g = b = a = NULL; 
	has_rgba = has_graya = has_rgb = 0;

	if(nr_comp == 4) /* RGBA */
   {
	has_rgba = 1;
	r = image->comps[0].data;
	g = image->comps[1].data;
	b = image->comps[2].data;
	a = image->comps[3].data;
   }
	else
	if(nr_comp == 2) /* GA */
   {
	has_graya = 1;
	r = image->comps[0].data;
	a = image->comps[1].data;
   }
	else
	if(nr_comp == 3) /* RGB */
   {
	has_rgb = 1;
	r = image->comps[0].data;
	g = image->comps[1].data;
	b = image->comps[2].data;
   }
	else /* G */
   {
	r = image->comps[0].data;
   }
	cs = buf;
	max = height * width;

	for(i = 0; i < max; ++i)
   {
	if(has_rgba) 
  {
#ifdef OPJ_BIG_ENDIAN
//RGBA
	*r++ = (int)*cs++; *g++ = (int)*cs++; *b++ =(int)*cs++;; 
	*a++ = (int)*cs++;
	continue;
#else
//RGBA
	*r++ = (int)*cs++; *g++ = (int)*cs++; *b++ =(int)*cs++;; 
	*a++ = (int)*cs++;
	continue;
#endif
  }

	if(has_rgb)
  {
#ifdef OPJ_BIG_ENDIAN
//RGB
	*r++ = (int)*cs++; *g++ = (int)*cs++; *b++ = (int)*cs++;
	continue;
#else
//RGB
	*r++ = (int)*cs++; *g++ = (int)*cs++; *b++ = (int)*cs++;
	continue;
#endif
  }

	if(has_graya)
  {
#ifdef OPJ_BIG_ENDIAN
//RA
	*r++ = (int)*cs++; *a++ = (int)*cs++;
	continue;
#else
//RA
	*r++ = (int)*cs++; *a++ = (int)*cs++;
	continue;
#endif
  }
/* G */
	*r++ = *cs++;

   }/* for(i = 0; i < max; ++i) */

	return image;

}/* to_opj_image() */

void JP2_save_file(Canvas *canvas, const char *write_idf)
{
    FILE *writer;
	opj_cparameters_t parameters;
	opj_image_t *image;
    int width, height, nr_comp, sub_dx, sub_dy;
	OPJ_CODEC_FORMAT codec_format;

	codec_format = ext_codec_format(write_idf);

	if(codec_format < (OPJ_CODEC_FORMAT)0)
   {
	fl_alert(WRONG_DST_EXT_s, write_idf);
	return;
   }
    if((writer = fopen(write_idf, "wb")) == NULL)
   {
    fl_alert(DST_DID_NOT_OPEN_s, write_idf);
    return;
   }
	opj_set_default_encoder_parameters(&parameters);

	if(parameters.cp_comment == NULL)
   {
	char buf[80];
	snprintf(buf, 80, "Created by OpenJPEG version %s", opj_version());
	parameters.cp_comment = strdup(buf);
   }

    if (parameters.tcp_numlayers == 0)
   {
    parameters.tcp_rates[0] = 0;   /* MOD antonin : losslessbug */
    parameters.tcp_numlayers++;
    parameters.cp_disto_alloc = 1;
   }
	sub_dx = parameters.subsampling_dx;
	sub_dy = parameters.subsampling_dy;

    width = canvas->new_iwidth;
    height = canvas->new_iheight;
    nr_comp = canvas->new_idepth;
/*--------------------------------------------------------*/
	image = to_opj_image((unsigned char*)canvas->cbuf,
		width, height, nr_comp, sub_dx, sub_dy);
/*--------------------------------------------------------*/
	if(image == NULL)
   {
    fl_alert("%s", WRITE_JP2_FAILS_s);
	free(parameters.cp_comment);
    return;
   }
   {
	opj_stream_t *stream;
	opj_codec_t* codec;

	stream = NULL;	
	parameters.tcp_mct = image->numcomps == 3 ? 1 : 0;

	fclose(writer); writer = NULL;

	codec = opj_create_compress(codec_format);

	if(codec == NULL) goto fin;

	opj_setup_encoder(codec, &parameters, image);

	stream =
     opj_stream_create_default_file_stream(write_idf, WRITE_JP2);

	if(stream == NULL) goto fin;

	if( !opj_start_compress(codec,image,stream)) goto fin;

	if( !opj_encode(codec, stream)) goto fin;

	opj_end_compress(codec, stream);

fin:

    if(stream) opj_stream_destroy(stream);

	if(codec) opj_destroy_codec(codec);

	opj_image_destroy(image);

	free(parameters.cp_comment);
   }

}//JP2_save_file()

static unsigned char *read_image_component(opj_image_t *local_image,
	int selected_component, 
	int *out_type, unsigned int *out_w, unsigned int *out_h)
{
	unsigned char *dst_buf, *d;
	int *red;
	int compw, comph, max_comp;
	size_t j, max;
	int shiftR, addR;
	unsigned int mask = 0xffff;
	int pix, ushift, dshift, prec, force8;
	int is_rgb;

	prec = local_image->comps[0].prec;

	if(prec > 16)
  {
	fprintf(stderr,"%s:%d:\n\tread_image_component\n"
	 "\tPREC(%d) not handled. Quitting.\n",__FILE__,__LINE__,prec);

	return NULL;
  }
	max_comp = local_image->numcomps;

	if(max_comp > 4) max_comp = 4;

	if((max_comp == 1))
   {
//	Nothing to do:
	*out_w = local_image->comps[0].w;
	*out_h = local_image->comps[0].h;

	return NULL;
   }
	is_rgb =
	(  local_image->color_space <= OPJ_CLRSPC_SRGB
	|| local_image->color_space == OPJ_CLRSPC_GRAY
	);

	if(is_rgb
    && (max_comp == 2 || max_comp == 4) )//GRAYA or RGBA
   {
    --max_comp;//1 or 3 in SRGB
   }
//-----------------------------
	--selected_component;
//-----------------------------
//SRGB,GRAY,SYCC,EYCC,CMYK
	is_rgb =
	(  local_image->color_space <= OPJ_CLRSPC_SRGB
	|| local_image->color_space == OPJ_CLRSPC_GRAY
	|| local_image->color_space == OPJ_CLRSPC_CMYK
	);

	if(is_rgb)
   {
	compw = local_image->comps[selected_component].w; 
	comph = local_image->comps[selected_component].h;
   }
    else
//	YUV420_FORMAT
    if((local_image->comps[0].dx == 1)
    && (local_image->comps[1].dx == 2)
    && (local_image->comps[2].dx == 2)
    && (local_image->comps[0].dy == 1)
    && (local_image->comps[1].dy == 2)
    && (local_image->comps[2].dy == 2))// horizontal and vertical sub-sample
   {
	compw = local_image->comps[selected_component].w; 
	comph = local_image->comps[selected_component].h;
	
   }
    else
//	YUV422
    if((local_image->comps[0].dx == 1)
    && (local_image->comps[1].dx == 2)
    && (local_image->comps[2].dx == 2)
    && (local_image->comps[0].dy == 1)
    && (local_image->comps[1].dy == 1)
    && (local_image->comps[2].dy == 1))// horizontal sub-sample onl
   {
	compw = local_image->comps[selected_component].w; 
	comph = local_image->comps[selected_component].h;
   }
    else
//	YUV444
    if((local_image->comps[0].dx == 1)
    && (local_image->comps[1].dx == 1)
    && (local_image->comps[2].dx == 1)
    && (local_image->comps[0].dy == 1)
    && (local_image->comps[1].dy == 1)
    && (local_image->comps[2].dy == 1))// no sub-sample
   {
	compw = local_image->comps[selected_component].w; 
	comph = local_image->comps[selected_component].h;
   }
	else
   {
	fprintf(stderr,"%s:%d:\n\tread_image_component fails.\n"
	"\tImage type unknown. Quitting.\n",__FILE__,__LINE__);
//	Nothing to do:
	*out_w = local_image->comps[0].w;
	*out_h = local_image->comps[0].h;

	return NULL;
   }
	*out_type = 1;//MONO
	*out_w = compw; 
	*out_h = comph;

	max = compw * comph;

	shiftR = 0;
	force8 = (prec < 8);

	dshift = ushift = 0;

	if(prec > 8)
	 shiftR = prec - 8;

    if(force8)
   {
    ushift = 8 - prec; dshift = prec - ushift;

	if(prec == 7) mask = 0x0000007f;
	else
	if(prec == 6) mask = 0x0000003f;
	else
	if(prec == 5) mask = 0x0000001f;
	else
    if(prec == 4) mask = 0x0000000f;
    else
	if(prec == 3) mask = 0x00000007;
	else
    if(prec == 2) mask = 0x00000003;
    else
/*    if(prec == 1) */ mask = 0x00000001;
   }

	addR = 
	(local_image->comps[selected_component].sgnd ? 1<<(prec - 1) : 0);

	d = dst_buf = (unsigned char*)malloc(max);

	red = local_image->comps[selected_component].data;

	for(j = 0; j < max; ++j)
   {
    pix = *red++ + addR;

    if(force8)
  {
	pix = pix&mask;	
    pix = (pix<<ushift) | (pix>>dshift);
  }
	else// prec >= 8 and <= 16
  {
    if(shiftR) pix = ((pix>>shiftR)+((pix>>(shiftR-1))%2));

    if(pix > 255) pix = 255; else if(pix < 0) pix = 0;
  }
    *d++ = (unsigned char)pix;

   }//for(j = 0; j < max; ++j)

	return dst_buf;

}/* read_image_component() */

typedef struct buffer_info
{
    unsigned char *sample_buf;
    unsigned char *cur_buf;
    uint64_t sample_size;

}BufferInfo;

static  OPJ_SIZE_T read_from_buffer(void *p_buffer, OPJ_SIZE_T p_nb_bytes,
    BufferInfo *bufinfo)
{
    OPJ_SIZE_T l_nb_read;
	unsigned char *sample_end = 
		(bufinfo->sample_buf + bufinfo->sample_size);

    if((bufinfo->cur_buf + (ptrdiff_t)p_nb_bytes) < sample_end)
   {
    l_nb_read = p_nb_bytes;
   }
    else
   {
    l_nb_read = (OPJ_SIZE_T)(sample_end - bufinfo->cur_buf);
   }

	if(l_nb_read > 0)
   {
    memcpy(p_buffer, bufinfo->cur_buf, l_nb_read);
    bufinfo->cur_buf += l_nb_read;
   }
	return l_nb_read ? l_nb_read : (OPJ_SIZE_T)-1;
}

static OPJ_SIZE_T write_from_buffer(void *p_buffer, OPJ_SIZE_T p_nb_bytes,
    BufferInfo *bufinfo)
{
    memcpy(bufinfo->cur_buf,p_buffer, p_nb_bytes);
    bufinfo->cur_buf += (ptrdiff_t)p_nb_bytes;
    bufinfo->sample_size += p_nb_bytes;

    return p_nb_bytes;
}

static OPJ_OFF_T skip_from_buffer(OPJ_OFF_T p_nb_bytes,
    BufferInfo *bufinfo)
{
	unsigned char *sample_end = 
		(bufinfo->sample_buf + bufinfo->sample_size);
	OPJ_OFF_T delta;

	if(sample_end > bufinfo->cur_buf)
	 delta = (sample_end - bufinfo->cur_buf);
	else
	 delta = 0;

	if(p_nb_bytes <= delta)
   {
    bufinfo->cur_buf += p_nb_bytes;

    return p_nb_bytes;
   }
    bufinfo->cur_buf = sample_end;


    return p_nb_bytes;
}

static OPJ_BOOL seek_from_buffer(OPJ_OFF_T p_nb_bytes,
    BufferInfo *bufinfo)
{
    if((uint64_t)p_nb_bytes <= bufinfo->sample_size )
   {
	bufinfo->cur_buf = (bufinfo->sample_buf + (ptrdiff_t)p_nb_bytes);

    return OPJ_TRUE;
   }
    bufinfo->cur_buf = (bufinfo->sample_buf + (ptrdiff_t)bufinfo->sample_size);

    return OPJ_FALSE;
}

static opj_stream_t* stream_create_buffer_stream(BufferInfo *bufinfo,
	OPJ_SIZE_T p_size, OPJ_BOOL p_is_read_stream)
{
    opj_stream_t* l_stream;

    l_stream = opj_stream_create(p_size, p_is_read_stream);

    if(! l_stream) return NULL;

    opj_stream_set_user_data(l_stream, bufinfo, NULL);

    opj_stream_set_user_data_length(l_stream,
		bufinfo->sample_size);

    opj_stream_set_read_function(l_stream,
		(opj_stream_read_fn) read_from_buffer);

    opj_stream_set_write_function(l_stream,
    	(opj_stream_write_fn) write_from_buffer);

    opj_stream_set_skip_function(l_stream,
     (opj_stream_skip_fn) skip_from_buffer);

    opj_stream_set_seek_function(l_stream,
     (opj_stream_seek_fn) seek_from_buffer);

    return l_stream;
}

//
//  DO NOT FREE src; is freed outside in OPENMJ2.cxx
//
unsigned char *JP2_decode(const char *read_idf, unsigned char *src,
	uint64_t fsize, int is_still, int decod_format, 
	unsigned int *out_width, unsigned int *out_height, 
	int *out_type, int *out_selected)
{
	opj_image_t *l_image;
	opj_codec_t *l_codec; 
	opj_stream_t *l_stream;
	unsigned char *dst_buf;
	unsigned int numcomps;
	int cur_tile, cur_reduction, max_tiles, max_reduction;
	int selected_component, fails;
	BufferInfo bufinfo;
	opj_dparameters_t parameters;
	OPJ_CODEC_FORMAT codec_format;
	ImageInfo dst;

	*out_width = 0; *out_height = 0; *out_type = -1;
	*out_selected = -1;

	memset(&bufinfo, 0, sizeof(BufferInfo));
	fails = 1; 
	cur_tile = -1; cur_reduction = 0; max_tiles = max_reduction = 0;
	l_codec = NULL; l_stream = NULL; l_image = NULL;
	memset(&parameters, 0, sizeof(opj_dparameters_t));

	opj_set_default_decoder_parameters(&parameters);

	strcpy(parameters.infile, read_idf);
	parameters.decod_format = decod_format;

	parameters.cp_layer = FLViewer_layers();

    if(decod_format == J2K_CFMT)//0
     codec_format = OPJ_CODEC_J2K;
    else
    if(decod_format == JP2_CFMT)//1
     codec_format = OPJ_CODEC_JP2;
    else
    if(decod_format == JPT_CFMT)//2
     codec_format = OPJ_CODEC_JPT;
    else
   {
/* clarified in infile_format() : */
    return NULL;
   }

	if(is_still == 0)//MJ2 buffer
   {
    bufinfo.cur_buf = bufinfo.sample_buf = src;
    bufinfo.sample_size = fsize;

	l_stream =
	 stream_create_buffer_stream(&bufinfo, fsize, READ_JP2);
   }
	else //STILL image, src == NULL
   {
	l_stream =
	 opj_stream_create_default_file_stream(read_idf, READ_JP2);
   }

	if(l_stream == NULL)
   {
	fprintf(stderr,"%s:%d:\n\tSTREAM == NULL\n",__FILE__,__LINE__);
	goto fin;
   }

	l_codec = opj_create_decompress(codec_format);

	if(l_codec == NULL)
   {
	fprintf(stderr,"%s:%d:\n\tCODEC == NULL\n",__FILE__,__LINE__);
	goto fin;
   }

	if(is_still == 1) 
   {
	FLViewer_wait();
   }

	if( !opj_setup_decoder(l_codec, &parameters))
   {
	fprintf(stderr,"%s:%d:\n\topj_setup_decoder failed\n",__FILE__,__LINE__);
	goto fin;
   }

	FLViewer_get_tile_and_reduction(&cur_tile, &cur_reduction);

	opj_set_decoded_resolution_factor(l_codec, cur_reduction);

	if( !opj_read_header(l_stream, l_codec, &l_image)) 
   {
	fprintf(stderr,"%s:%d:\n\topj_read_header failed\n",__FILE__,__LINE__);
	goto fin;
   }

	FLViewer_get_max_tiles_and_reduction(&max_tiles, &max_reduction);

	if( !FLViewer_has_tile_and_reduction()
	|| (max_tiles <= 0) 
	|| (max_reduction <= 0) )
   {
	opj_codestream_info_v2_t *cstr;

	cstr = opj_get_cstr_info(l_codec);

	max_tiles = cstr->tw * cstr->th;
	max_reduction = cstr->m_default_tile_info.tccp_info->numresolutions;

	FLViewer_put_max_tiles_and_reduction(max_tiles, max_reduction);	
   }

	if(cur_tile < 0)
   {
	int x0, y0, x1, y1;

	x0 = y0 = x1 = y1 = 0;

	if(FLViewer_has_area_values())
  {
	FLViewer_get_area_values(&x0, &y0, &x1, &y1);
  }
	if( !opj_set_decode_area(l_codec, l_image, x0, y0, x1, y1))
  {
	fprintf(stderr,"%s:%d:\n\topj_set_decode_area failed\n",__FILE__,__LINE__);
	goto fin;
  }

	if( !opj_decode(l_codec, l_stream, l_image))
  {
	fprintf(stderr,"%s:%d:\n\topj_decode failed\n",__FILE__,__LINE__);
	goto fin;
  }
   }
	else /* decode a tile */
   {
	if( !opj_get_decoded_tile(l_codec, l_stream, l_image, cur_tile))
  {
	fprintf(stderr,"%s:%d:\n\topj_get_decoded_tile %d failed.\n",
	 __FILE__,__LINE__, cur_tile);
	goto fin;
  }
   }

	if( !opj_end_decompress(l_codec, l_stream))
   {
	fprintf(stderr,"%s:%d:\n\topj_end_decompress failed\n",__FILE__,__LINE__);
	goto fin;
   }
	fails = 0;

fin:
//	DO NOT FREE src; is freed outside in OPENMJ2.cxx

	if(l_stream) opj_stream_destroy(l_stream);

	if(l_codec) opj_destroy_codec(l_codec);

	if(fails)
   {
	if(l_image) opj_image_destroy(l_image);

	fl_alert("%s\n %s", JP2_DECODE_FAILS_s, read_idf);

	FLViewer_reset_tiles_and_reduction();

	return NULL;
   }

	numcomps = l_image->numcomps;

	if(numcomps > 4) numcomps = 4;
#ifdef DEBUG_NUMCOMPS
fprintf(stderr,"%s:%d:\n\tTRACE numcomps(%d) color_space(%d)\n",
__FILE__,__LINE__,numcomps,l_image->color_space);
#endif

	memset(&dst, 0, sizeof(ImageInfo));

	dst.is_still = is_still;

	dst.red = l_image->comps[0].data;
	dst.dx[0] = l_image->comps[0].dx;
	dst.dy[0] = l_image->comps[0].dy;
	dst.prec[0] = l_image->comps[0].prec;
	dst.sgnd[0] = l_image->comps[0].sgnd;

	if(numcomps > 1)
   {
	dst.green = l_image->comps[1].data;
	dst.dx[1] = l_image->comps[1].dx;
	dst.dy[1] = l_image->comps[1].dy;
	dst.prec[1] = l_image->comps[1].prec;
	dst.sgnd[1] = l_image->comps[1].sgnd;
   }
	if(numcomps > 2)
   {
	dst.blue = l_image->comps[2].data;
	dst.dx[2] = l_image->comps[2].dx;
	dst.dy[2] = l_image->comps[2].dy;
	dst.prec[2] = l_image->comps[2].prec;
	dst.sgnd[2] = l_image->comps[2].sgnd;
   }
	if(numcomps > 3)
   {
	dst.alpha = l_image->comps[3].data;
	dst.dx[3] = l_image->comps[3].dx;
	dst.dy[3] = l_image->comps[3].dy;
	dst.prec[3] = l_image->comps[3].prec;
	dst.sgnd[3] = l_image->comps[3].sgnd;
   }
	if(numcomps == 2)//GA
   {
	dst.alpha = l_image->comps[1].data;
	dst.dx[3] = l_image->comps[1].dx;
	dst.dy[3] = l_image->comps[1].dy;
	dst.prec[3] = l_image->comps[1].prec;
	dst.sgnd[3] = l_image->comps[1].sgnd;
   }

	if(l_image->color_space < OPJ_CLRSPC_SRGB
	&& l_image->color_space >= 0
	&& numcomps == 3
	&& l_image->comps[0].dx == l_image->comps[0].dy
	&& l_image->comps[1].dx != 1
	  )
	 l_image->color_space = OPJ_CLRSPC_SYCC;
	else
	if(numcomps <= 2)
     l_image->color_space = OPJ_CLRSPC_GRAY;

	dst.color_space = l_image->color_space;
	dst.numcomps = numcomps;

	selected_component = FLViewer_component();

	*out_selected = selected_component;

	if(selected_component)
   {
	dst_buf =
	read_image_component(l_image, selected_component,
		out_type, out_width, out_height);
   }
	else
   {
    if(l_image->color_space == OPJ_CLRSPC_GRAY)
     FLViewer_set_max_components(1);
    else
    if(l_image->color_space <= OPJ_CLRSPC_SRGB
	&& numcomps > 3)
     FLViewer_set_max_components(3);
    else
     FLViewer_set_max_components(l_image->numcomps);

#ifdef DEBUG_NUMCOMPS
fprintf(stderr,"%s:%d:\n\tTRACE numcomps(%d) color_space(%d)\n"
"\tICC buf(%p) buflen(%d)\n",
__FILE__,__LINE__,numcomps,l_image->color_space,
(void*)l_image->icc_profile_buf,l_image->icc_profile_len);
#endif

	if(l_image->color_space == OPJ_CLRSPC_SYCC)
  {
	COLOR_sycc_to_rgb(l_image, &dst);
  }
	else
	if(l_image->color_space == OPJ_CLRSPC_CMYK)
  {
	COLOR_cmyk_to_rgb(l_image, &dst);
  }
	else
	if(l_image->color_space == OPJ_CLRSPC_EYCC)
  {
	COLOR_esycc_to_rgb(l_image, &dst);
  }

	if(l_image->icc_profile_buf)
  {
#if defined(OPJ_HAVE_LIBLCMS1) || defined(OPJ_HAVE_LIBLCMS2)
	if(l_image->icc_profile_len)
 {
	COLOR_apply_icc_profile(l_image, &dst);
 }
	else
 {
	COLOR_apply_conversion(l_image, &dst);
 }
#endif
  }

	if(l_image->color_space == OPJ_CLRSPC_SYCC)
  {
	l_image->color_space = OPJ_CLRSPC_UNSPECIFIED;
  }

	dst_buf = 
	read_image(l_image, &dst, out_type, out_width, out_height);

	dst_destroy(&dst);

   }//if(selected_component)

	opj_image_destroy(l_image);


	return dst_buf;

}// JP2_decode()

//
// rgb_buffer ==> image ==> file
//
void JP2_file_from_rgb(const unsigned char *buf, unsigned int width,
	unsigned int height, unsigned int numcomps, const char *write_idf)
{
    FILE *writer;
	opj_cparameters_t parameters;
	opj_image_t *image;
    int sub_dx, sub_dy;
	OPJ_CODEC_FORMAT codec_format;

	codec_format = OPJ_CODEC_JP2;

    if((writer = fopen(write_idf, "wb")) == NULL)
   {
    fl_alert(DST_DID_NOT_OPEN_s, write_idf);
    return;
   }
	opj_set_default_encoder_parameters(&parameters);

	if(parameters.cp_comment == NULL)
   {
	char buf[80];
	snprintf(buf, 80, "Created by OpenJPEG version %s", opj_version());
	parameters.cp_comment = strdup(buf);
   }

    if (parameters.tcp_numlayers == 0)
   {
    parameters.tcp_rates[0] = 0;   /* MOD antonin : losslessbug */
    parameters.tcp_numlayers++;
    parameters.cp_disto_alloc = 1;
   }
	sub_dx = parameters.subsampling_dx;
	sub_dy = parameters.subsampling_dy;

//--------------------------------------------------------
	image = to_opj_image(buf, (int)width, (int)height,
		(int)numcomps, sub_dx, sub_dy);
//--------------------------------------------------------
	if(image == NULL)
   {
    fl_alert("%s", WRITE_JP2_FAILS_s);
	free(parameters.cp_comment);
    return;
   }
   {
	opj_stream_t *stream;
	opj_codec_t* codec;

	stream = NULL;	
	parameters.tcp_mct = image->numcomps == 3 ? 1 : 0;

	fclose(writer); writer = NULL;

	codec = opj_create_compress(codec_format);

	if(codec == NULL) goto fin;

	opj_setup_encoder(codec, &parameters, image);

	stream =
     opj_stream_create_default_file_stream(write_idf, WRITE_JP2);

	if(stream == NULL) goto fin;

	if( !opj_start_compress(codec,image,stream)) goto fin;

	if( !opj_encode(codec, stream)) goto fin;

	opj_end_compress(codec, stream);

fin:

    if(stream) opj_stream_destroy(stream);

	if(codec) opj_destroy_codec(codec);

	opj_image_destroy(image);

	free(parameters.cp_comment);
   }

}//JP2_file_from_rgb()
