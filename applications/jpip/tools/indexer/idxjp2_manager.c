/*
 * $Id$
 *
 * Copyright (c) 2002-2011, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2011, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux and Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team

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

/*! \file
 *  \brief Copy of some jp2.c functions, and modification of jpip.h from 2KAN indexer
 */

#include <stdio.h>
#include "ext_libopenjpeg/ext_openjpeg.h"
#include "j2k_to_idxjp2.h"
#include "event_mgr_handler.h"


void fwrite_idxjp2( char filename[], unsigned char *j2kstream, int j2klen, opj_image_t *image, opj_codestream_info_t cstr_info)
{
  int codestream_length;
  opj_cio_t *cio = NULL;
  FILE *fp = NULL;
  opj_cinfo_t *cinfo;
  opj_cparameters_t parameters;	/* compression parameters */
  opj_event_mgr_t event_mgr;
  opj_bool encSuccess;

  /* set encoding parameters to default values */
  opj_set_default_encoder_parameters(&parameters);
  
  /* get a JP2 compressor handle */
  cinfo = opj_create_compress(CODEC_JP2);

  event_mgr = set_default_event_mgr();

  /* catch events using our callbacks and give a local context */
  opj_set_event_mgr((opj_common_ptr)cinfo, &event_mgr, stderr);

  /* setup the encoder parameters using the current image and using user parameters */
  opj_setup_encoder(cinfo, &parameters, image);

  /* open a byte stream for writing */
  /* allocate memory for all tiles */
  cio = opj_cio_open((opj_common_ptr)cinfo, NULL, 0);

  encSuccess = idxjp2_encode( cinfo, cio, j2kstream, j2klen, cstr_info, image);

  if (!encSuccess){
    opj_cio_close(cio);
    fprintf(stderr, "failed to encode image\n");
    return;
  }
  codestream_length = cio_tell(cio);

  /* write the buffer to disk */
  fp = fopen( filename, "wb");
  if (!fp) {
    fprintf(stderr, "failed to open %s for writing\n", filename);
    return;
  }

  fwrite(cio->buffer, 1, codestream_length, fp);
  fclose(fp);
  fprintf(stderr,"Generated outfile %s\n", filename);
  /* close and free the byte stream */
  opj_cio_close(cio);

  /* free remaining compression structures */
  opj_destroy_compress(cinfo);

  opj_destroy_cstr_info(&cstr_info);
}
