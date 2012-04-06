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
#include <stdlib.h>

#define J2K_CFMT 0

void error_callback(const char *msg, void *v) {
assert(0);
}
void warning_callback(const char *msg, void *v) {
puts(msg);
}
void info_callback(const char *msg, void *v) {
puts(msg);
}

int main(int argc, char *argv[])
{
  const char * v = opj_version();
  puts(v);

  const OPJ_COLOR_SPACE color_space = CLRSPC_GRAY;
  int numcomps = 1;
  int i;
  int image_width = 256;
  int image_height = 256;

  opj_cparameters_t parameters;
  opj_set_default_encoder_parameters(&parameters);
  parameters.cod_format = J2K_CFMT;

  int subsampling_dx = parameters.subsampling_dx;
  int subsampling_dy = parameters.subsampling_dy;

  opj_image_cmptparm_t cmptparm;
  cmptparm.prec = 8;
  cmptparm.bpp = 8;
  cmptparm.sgnd = 0;
  cmptparm.dx = subsampling_dx;
  cmptparm.dy = subsampling_dy;
  cmptparm.w = image_width;
  cmptparm.h = image_height;

  opj_image_t *image = opj_image_create(numcomps, &cmptparm, color_space);
  assert( image );

  for (i = 0; i < image_width * image_height; i++)
    {
    int compno;
    for(compno = 0; compno < numcomps; compno++)
      {
      image->comps[compno].data[i] = 0;
      }
    }

  opj_event_mgr_t event_mgr;
  event_mgr.error_handler = error_callback;
  event_mgr.warning_handler = warning_callback;
  event_mgr.info_handler = info_callback;

  opj_cinfo_t* cinfo = opj_create_compress(CODEC_J2K);
  opj_set_event_mgr((opj_common_ptr)cinfo, &event_mgr, stderr);

  opj_setup_encoder(cinfo, &parameters, image);

  opj_cio_t *cio = opj_cio_open((opj_common_ptr)cinfo, NULL, 0);
  assert( cio );
  opj_bool bSuccess = opj_encode(cinfo, cio, image, NULL);
  assert( bSuccess );

  size_t codestream_length = cio_tell(cio);
  assert( codestream_length );

  const char outputfile[] = "testempty2.j2k";
  strcpy(parameters.outfile, outputfile);
  FILE *f = fopen(parameters.outfile, "wb");
  assert( f );
  fwrite(cio->buffer, 1, codestream_length, f);
  fclose(f);

  opj_cio_close(cio);
  opj_destroy_compress(cinfo);
  opj_image_destroy(image);

  /* read back the generated file */
  size_t file_length;
  FILE *fsrc = fopen(outputfile, "rb");
  assert( fsrc );
  fseek(fsrc, 0, SEEK_END);
  file_length = ftell(fsrc);
  fseek(fsrc, 0, SEEK_SET);
  unsigned char *src = (unsigned char *) malloc(file_length);
  if (fread(src, 1, file_length, fsrc) != file_length)
    {
    free(src);
    fclose(fsrc);
    return 1;
    }
  fclose(fsrc);

	opj_dinfo_t* dinfo = NULL;	/* handle to a decompressor */
  dinfo = opj_create_decompress(CODEC_J2K);

  opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);

	opj_dparameters_t dparameters;
	opj_set_default_decoder_parameters(&dparameters);
  opj_setup_decoder(dinfo, &dparameters);

  cio = opj_cio_open((opj_common_ptr)dinfo, src, file_length);
  image = opj_decode(dinfo, cio);
  if(!image) {
    opj_destroy_decompress(dinfo);
    opj_cio_close(cio);
    return 1;
  }
    opj_destroy_decompress(dinfo);
    opj_cio_close(cio);


  puts( "end" );
  return 0;
}
