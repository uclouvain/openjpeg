/*
 * Copyright (c) 2023, Even Rouault
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

#include "openjpeg.h"

static void test_colorspace(const char* pszDirectory)
{
    char szFile[2048];
    opj_image_t* image = NULL;
    opj_stream_t *l_stream = NULL;              /* Stream */
    opj_codec_t* l_codec = NULL;                /* Handle to a decompressor */
    opj_dparameters_t parameters;               /* decompression parameters */

    snprintf(szFile, sizeof(szFile), "%s/input/conformance/file1.jp2",
             pszDirectory);
    l_stream = opj_stream_create_default_file_stream(szFile, 1);
    if (!l_stream) {
        fprintf(stderr, "ERROR -> failed to create the stream from the file %s\n",
                szFile);
        exit(1);
    }
    l_codec = opj_create_decompress(OPJ_CODEC_JP2);

    /* Setup the decoder */
    opj_set_default_decoder_parameters(&parameters);
    if (!opj_setup_decoder(l_codec, &parameters)) {
        fprintf(stderr, "ERROR -> opj_decompress: failed to setup the decoder\n");
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        exit(1);
    }

    /* Read the main header of the codestream and if necessary the JP2 boxes*/
    if (! opj_read_header(l_stream, l_codec, &image)) {
        fprintf(stderr, "ERROR -> opj_decompress: failed to read the header\n");
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        opj_image_destroy(image);
        exit(1);
    }

    /* Check that color_space is set after opj_read_header() */
    if (image->color_space != OPJ_CLRSPC_SRGB) {
        fprintf(stderr, "ERROR -> image->color_space (=%d) != OPJ_CLRSPC_SRGB\n",
                image->color_space);
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        opj_image_destroy(image);
        exit(1);
    }

    opj_destroy_codec(l_codec);
    opj_stream_destroy(l_stream);
    opj_image_destroy(image);
}

static void test_iccprofile(const char* pszDirectory)
{
    char szFile[2048];
    opj_image_t* image = NULL;
    opj_stream_t *l_stream = NULL;              /* Stream */
    opj_codec_t* l_codec = NULL;                /* Handle to a decompressor */
    opj_dparameters_t parameters;               /* decompression parameters */

    snprintf(szFile, sizeof(szFile), "%s/input/nonregression/relax.jp2",
             pszDirectory);
    l_stream = opj_stream_create_default_file_stream(szFile, 1);
    if (!l_stream) {
        fprintf(stderr, "ERROR -> failed to create the stream from the file %s\n",
                szFile);
        exit(1);
    }
    l_codec = opj_create_decompress(OPJ_CODEC_JP2);

    /* Setup the decoder */
    opj_set_default_decoder_parameters(&parameters);
    if (!opj_setup_decoder(l_codec, &parameters)) {
        fprintf(stderr, "ERROR -> opj_decompress: failed to setup the decoder\n");
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        exit(1);
    }

    /* Read the main header of the codestream and if necessary the JP2 boxes*/
    if (! opj_read_header(l_stream, l_codec, &image)) {
        fprintf(stderr, "ERROR -> opj_decompress: failed to read the header\n");
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        opj_image_destroy(image);
        exit(1);
    }

    /* Check that icc_profile_len is set after opj_read_header() */
    if (image->icc_profile_len != 278) {
        fprintf(stderr, "ERROR -> image->icc_profile_len (=%d) != 278\n",
                image->icc_profile_len);
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        opj_image_destroy(image);
        exit(1);
    }

    opj_destroy_codec(l_codec);
    opj_stream_destroy(l_stream);
    opj_image_destroy(image);
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: testjp2 /path/to/opj_data_root\n");
        exit(1);
    }

    test_colorspace(argv[1]);
    test_iccprofile(argv[1]);

    return 0;
}
