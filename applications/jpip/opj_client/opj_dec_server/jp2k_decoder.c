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

#include <string.h>
#include <stdlib.h>
#include "jp2k_decoder.h"
#include "openjpeg.h"


void error_callback(const char *msg, void *client_data);
void warning_callback(const char *msg, void *client_data);
void info_callback(const char *msg, void *client_data);

Byte_t * imagetopnm(opj_image_t *image, ihdrbox_param_t **ihdrbox);

Byte_t * j2k_to_pnm( Byte_t *j2kstream, Byte8_t j2klen, ihdrbox_param_t **ihdrbox)
{
  Byte_t *pnmstream = NULL;
  opj_dparameters_t parameters;	/* decompression parameters */
  opj_event_mgr_t event_mgr;		/* event manager */
  opj_image_t *image = NULL;
  opj_dinfo_t* dinfo = NULL;	/* handle to a decompressor */
  opj_cio_t *cio = NULL;

  /* configure the event callbacks (not required) */
  memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
  event_mgr.error_handler = error_callback;
  event_mgr.warning_handler = warning_callback;
  event_mgr.info_handler = info_callback;

  /* set decoding parameters to default values */
  opj_set_default_decoder_parameters(&parameters);

  /* decode the code-stream */
  /* ---------------------- */

  /* JPEG-2000 codestream */
  /* get a decoder handle */
  dinfo = opj_create_decompress( CODEC_J2K);

  /* catch events using our callbacks and give a local context */
  opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);

  /* setup the decoder decoding parameters using user parameters */
  opj_setup_decoder(dinfo, &parameters);
  /* open a byte stream */
  cio = opj_cio_open((opj_common_ptr)dinfo, j2kstream, j2klen);

  /* decode the stream and fill the image structure */
  image = opj_decode(dinfo, cio);
  if(!image) {
    fprintf(stderr, "ERROR -> jp2_to_image: failed to decode image!\n");
    opj_destroy_decompress(dinfo);
    opj_cio_close(cio);
    return NULL;
  }

  /* close the byte stream */
  opj_cio_close(cio);
  
  /* create output image */
  /* ------------------- */
  if( (pnmstream = imagetopnm( image, ihdrbox))==NULL)
    fprintf( stderr, "PNM image not generated\n");

  /* free remaining structures */
  if(dinfo) {
    opj_destroy_decompress(dinfo);
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
  //  fprintf(stdout, "[INFO] %s", msg);
}


Byte_t * imagetopnm(opj_image_t *image, ihdrbox_param_t **ihdrbox)
{
  int adjustR, adjustG=0, adjustB=0;
  int datasize;
  Byte_t *pix=NULL, *ptr=NULL;
  int i;
  
  if(*ihdrbox){
    if( (*ihdrbox)->nc != image->numcomps)
      fprintf( stderr, "Exception: num of components not identical, codestream: %d, ihdrbox: %d\n", image->numcomps, (*ihdrbox)->nc);
    
    if( (*ihdrbox)->width != image->comps[0].w)
      fprintf( stderr, "Exception: width value not identical, codestream: %d, ihdrbox: %d\n", image->comps[0].w, (*ihdrbox)->width);
    
    if( (*ihdrbox)->height != image->comps[0].h)
      fprintf( stderr, "Exception: heigth value not identical, codestream: %d, ihdrbox: %d\n", image->comps[0].h, (*ihdrbox)->height);
    
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
    
    //    if( adjustR > 0)
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
