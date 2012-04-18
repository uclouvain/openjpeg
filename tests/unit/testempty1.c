/*
 * Copyright (c) 2012, Mathieu Malaterre
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
#include <openjpeg.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define J2K_CFMT 0

void error_callback(const char *msg, void *v);
void warning_callback(const char *msg, void *v);
void info_callback(const char *msg, void *v);

void error_callback(const char *msg, void *v) {
(void)msg;
(void)v;
assert(0);
}
void warning_callback(const char *msg, void *v) {
(void)msg;
(void)v;
puts(msg);
}
void info_callback(const char *msg, void *v) {
(void)msg;
(void)v;
puts(msg);
}

int main(int argc, char *argv[])
{
  const char * v = opj_version();

  const OPJ_COLOR_SPACE color_space = CLRSPC_GRAY;
  int numcomps = 1;
  int i;
  int image_width = 256;
  int image_height = 256;

  opj_cparameters_t parameters;

  int subsampling_dx = 0;
  int subsampling_dy = 0;

  opj_image_cmptparm_t cmptparm;
  opj_image_t *image;
  opj_event_mgr_t event_mgr;
  opj_cinfo_t* cinfo;
  opj_cio_t *cio;
  opj_bool bSuccess;
  size_t codestream_length;
  FILE *f;
  (void)argc;
  (void)argv;

  opj_set_default_encoder_parameters(&parameters);
  parameters.cod_format = J2K_CFMT;
  puts(v);
  cmptparm.prec = 8;
  cmptparm.bpp = 8;
  cmptparm.sgnd = 0;
  cmptparm.dx = subsampling_dx;
  cmptparm.dy = subsampling_dy;
  cmptparm.w = image_width;
  cmptparm.h = image_height;

  image = opj_image_create(numcomps, &cmptparm, color_space);
  assert( image );

  for (i = 0; i < image_width * image_height; i++)
    {
    int compno;
    for(compno = 0; compno < numcomps; compno++)
      {
      image->comps[compno].data[i] = 0;
      }
    }

  event_mgr.error_handler = error_callback;
  event_mgr.warning_handler = warning_callback;
  event_mgr.info_handler = info_callback;

  cinfo = opj_create_compress(CODEC_J2K);
  opj_set_event_mgr((opj_common_ptr)cinfo, &event_mgr, stderr);

  opj_setup_encoder(cinfo, &parameters, image);

  cio = opj_cio_open((opj_common_ptr)cinfo, NULL, 0);
  assert( cio );
  bSuccess = opj_encode(cinfo, cio, image, NULL);
  assert( bSuccess );

  codestream_length = (size_t)cio_tell(cio);
  assert( codestream_length );

  strcpy(parameters.outfile, "testempty1.j2k");
  f = fopen(parameters.outfile, "wb");
  assert( f );
  fwrite(cio->buffer, 1, codestream_length, f);
  fclose(f);

  opj_cio_close(cio);
  opj_destroy_compress(cinfo);
  opj_image_destroy(image);

  puts( "end" );
  return 0;
}
