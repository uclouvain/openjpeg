/*
 * $Id: jp2k_decoder.c 53 2011-05-09 16:55:39Z kaori $
 *
 * Copyright (c) 2002-2011, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2011, Professor Benoit Macq
 * Copyright (c) 2010-2011, Kaori Hagihara
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
#include <string.h>
#include <stdlib.h>
#include "jp2k_decoder.h"
#include "openjpeg.h"


void error_callback(const char *msg, void *client_data);
void warning_callback(const char *msg, void *client_data);
void info_callback(const char *msg, void *client_data);

Byte_t * imagetopnm(opj_image_t *image, ihdrbox_param_t **ihdrbox);

Byte_t * j2k_to_pnm( FILE *fp, ihdrbox_param_t **ihdrbox)
{
  Byte_t *pnmstream = NULL;
  opj_dparameters_t parameters;	/* decompression parameters */
  opj_event_mgr_t event_mgr;		/* event manager */
  opj_image_t *image = NULL;
  opj_codec_t *dinfo = NULL;	/* handle to a decompressor */
  opj_stream_t *cio = NULL;

  /* configure the event callbacks (not required) */
  memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
  event_mgr.error_handler = error_callback;
  event_mgr.warning_handler = warning_callback;
  event_mgr.info_handler = info_callback;

  /* set decoding parameters to default values */
  opj_set_default_decoder_parameters(&parameters);

  /* Set default event mgr */
  opj_initialize_default_event_handler(&event_mgr, 1);

  /* set a byte stream */
  cio = opj_stream_create_default_file_stream( fp, 1);
  if (!cio){
    fprintf(stderr, "ERROR -> failed to create the stream from the file\n");
    return NULL;
  }

  /* decode the code-stream */
  /* ---------------------- */

  /* JPEG-2000 codestream */
  /* get a decoder handle */
  dinfo = opj_create_decompress_v2(CODEC_J2K);

  /* setup the decoder decoding parameters using user parameters */
  if ( !opj_setup_decoder_v2(dinfo, &parameters, &event_mgr) ){
    fprintf(stderr, "ERROR -> j2k_dump: failed to setup the decoder\n");
    return NULL;
  }

  /* Read the main header of the codestream and if necessary the JP2 boxes*/
  if(! opj_read_header( cio, dinfo, &image)){
    fprintf(stderr, "ERROR -> j2k_to_image: failed to read the header\n");
    opj_stream_destroy(cio);
    opj_destroy_codec(dinfo);
    opj_image_destroy(image);
    return NULL;
  }

#ifdef TODO /*decode area could be set from j2k_to_pnm call, modify the protocol between JPIP viewer and opj_dec_server*/
  if (! opj_set_decode_area( dinfo, image, parameters.DA_x0, parameters.DA_y0, parameters.DA_x1, parameters.DA_y1)){
    fprintf(stderr, "ERROR -> j2k_to_image: failed to set the decoded area\n");
    opj_stream_destroy(cio);
    opj_destroy_codec(dinfo);
    opj_image_destroy(image);
    return NULL;
  }
#endif /*TODO*/

  /* Get the decoded image */
  if ( !( opj_decode_v2(dinfo, cio, image) && opj_end_decompress(dinfo,cio) ) ) {
    fprintf(stderr, "ERROR -> j2k_to_image: failed to decode image!\n");
    opj_stream_destroy(cio);
    opj_destroy_codec(dinfo);
    opj_image_destroy(image);
    return NULL;
  }

  fprintf(stderr, "image is decoded!\n");

  /* close the byte stream */
  opj_stream_destroy(cio);
  
  /* create output image */
  /* ------------------- */
  if( (pnmstream = imagetopnm( image, ihdrbox))==NULL)
    fprintf( stderr, "PNM image not generated\n");

  /* free remaining structures */
  if(dinfo) {
    opj_destroy_codec(dinfo);
  }

  /* free image data structure */
  opj_image_destroy(image);
  
  return pnmstream;
}


/**
   sample error callback expecting a FILE* client object
*/
void error_callback(const char *msg, void *client_data) {
  FILE *stream = (FILE*)client_data;
  fprintf(stream, "[ERROR] %s", msg);
}
/**
   sample warning callback expecting a FILE* client object
*/
void warning_callback(const char *msg, void *client_data) {
  FILE *stream = (FILE*)client_data;
  fprintf(stream, "[WARNING] %s", msg);
}
/**
   sample debug callback expecting no client object
*/
void info_callback(const char *msg, void *client_data) {
  (void)client_data;
  (void)msg;
  /*  fprintf(stdout, "[INFO] %s", msg); */
}


Byte_t * imagetopnm(opj_image_t *image, ihdrbox_param_t **ihdrbox)
{
  int adjustR, adjustG=0, adjustB=0;
  int datasize;
  Byte_t *pix=NULL, *ptr=NULL;
  OPJ_UINT32 i;
  
  if(*ihdrbox){
    if( (*ihdrbox)->nc != image->numcomps)
      fprintf( stderr, "Exception: num of components not identical, codestream: %d, ihdrbox: %d\n", image->numcomps, (*ihdrbox)->nc);

    if( (*ihdrbox)->width != image->comps[0].w)
      (*ihdrbox)->width = image->comps[0].w;
    
    if( (*ihdrbox)->height != image->comps[0].h)
      (*ihdrbox)->height = image->comps[0].h;

    if( (*ihdrbox)->bpc != image->comps[0].prec)
      fprintf( stderr, "Exception: bits per component not identical, codestream: %d, ihdrbox: %d\n", image->comps[0].prec, (*ihdrbox)->bpc);
  }
  else{
    *ihdrbox = (ihdrbox_param_t *)malloc( sizeof(ihdrbox_param_t));
    (*ihdrbox)->width  = image->comps[0].w;
    (*ihdrbox)->height = image->comps[0].h;
    (*ihdrbox)->bpc    = image->comps[0].prec;
    (*ihdrbox)->nc     = image->numcomps;
  }
  
  datasize = (image->numcomps)*(image->comps[0].w)*(image->comps[0].h);
  
  if (image->comps[0].prec > 8) {
    adjustR = image->comps[0].prec - 8;
    printf("PNM CONVERSION: Truncating component 0 from %d bits to 8 bits\n", image->comps[0].prec);
  }
  else
    adjustR = 0;
  
  if( image->numcomps == 3){
    if (image->comps[1].prec > 8) {
      adjustG = image->comps[1].prec - 8;
      printf("PNM CONVERSION: Truncating component 1 from %d bits to 8 bits\n", image->comps[1].prec);
    }
    else 
      adjustG = 0;
    
    if (image->comps[2].prec > 8) {
      adjustB = image->comps[2].prec - 8;
      printf("PNM CONVERSION: Truncating component 2 from %d bits to 8 bits\n", image->comps[2].prec);
    }
    else 
      adjustB = 0;
  }

  pix = (Byte_t *)malloc( datasize);
  ptr = pix;

  for( i = 0; i < image->comps[0].w * image->comps[0].h; i++){
    int r, g, b;
    r = image->comps[0].data[i];
    r += (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
    
    /*    if( adjustR > 0) */
    *(ptr++) = (Byte_t) ((r >> adjustR)+((r >> (adjustR-1))%2));

    if( image->numcomps == 3){
      g = image->comps[1].data[i];
      g += (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
      *(ptr++) = (Byte_t) ((g >> adjustG)+((g >> (adjustG-1))%2));
      
      b = image->comps[2].data[i];
      b += (image->comps[2].sgnd ? 1 << (image->comps[2].prec - 1) : 0);
      *(ptr++) = (Byte_t) ((b >> adjustB)+((b >> (adjustB-1))%2));
    }
  }

  return pix;
}
