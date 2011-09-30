/*
 * $Id$
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
#include "ext_libopenjpeg/ext_openjpeg.h"
#include "j2k_to_idxjp2.h"
#include "event_mgr_handler.h"

opj_image_t * decode_j2k( unsigned char *j2kstream, int j2klen, opj_codestream_info_t *cstr_info)
{
  opj_image_t *image;
  opj_dparameters_t parameters;	/* decompression parameters */
  opj_dinfo_t *dinfo;	/* handle to a decompressor */
  opj_cio_t *cio;
  opj_event_mgr_t event_mgr;		/* event manager */

  /* set decoding parameters to default values */
  opj_set_default_decoder_parameters(&parameters);

  /* decode the code-stream */
  /* ---------------------- */

  /* JPEG-2000 codestream */
  /* get a decoder handle */
  dinfo = opj_create_decompress( CODEC_J2K);

  event_mgr = set_default_event_mgr();

  /* catch events using our callbacks and give a local context */
  opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);

  /* setup the decoder decoding parameters using user parameters */
  opj_setup_decoder(dinfo, &parameters);

  /* open a byte stream */
  cio = opj_cio_open((opj_common_ptr)dinfo, j2kstream, j2klen);
  
  /* decode the stream and fill the image structure */
  image = opj_decode_with_info(dinfo, cio, cstr_info);
  if(!image) {
    fprintf(stderr, "ERROR -> jp2_to_image: failed to decode image!\n");
    opj_destroy_decompress(dinfo);
    opj_cio_close(cio);
    return NULL;
  }

  if( cstr_info->marknum == 0)
    add_mainheader_marker_info( cio, cstr_info);

  /* close the byte stream */
  opj_cio_close(cio);
 
  /* free remaining structures */
  if(dinfo) {
    opj_destroy_decompress(dinfo);
  }

  return image;
}
