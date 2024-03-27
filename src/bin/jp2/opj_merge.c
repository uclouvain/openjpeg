/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2010, Mathieu Malaterre, GDCM
 * Copyright (c) 2011-2012, Centre National d'Etudes Spatiales (CNES), France
 * Copyright (c) 2012, CS Systemes d'Information, France
 * Copyright (c) 2024, Daniel Garcia Briseno, ADNET Systems Inc, NASA
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

int main(int argc, char **argv) {
    /** Create jpx encoder */
    opj_codec_t* codec = opj_create_compress(OPJ_CODEC_JPX);
    /* set encoding parameters to default values */
    opj_cparameters_t parameters;
    /** Output file */
    opj_stream_t *outfile = NULL;
    /** Creation status */
    OPJ_BOOL bSuccess = OPJ_FALSE;
    /** Program return code */
    int ret = 1;

    if (!codec) {
        fprintf(stderr, "Failed to initialize the jpx codec.\n");
        return ret;
    }

    opj_set_default_encoder_parameters(&parameters);

    // Creating references to other jpx files doesn't require image data.
    // so pass NULL for the image parameter and hope for the best.
    if (! opj_setup_encoder(codec, &parameters, 1)) {
        fprintf(stderr, "Failed to setup encoder: opj_setup_encoder\n");
        goto fin;
    }

    // Use extra options to specify the list of files to be merged into the jpx file.
    {
        const char* options[2] = { "img/2024_03_27__13_21_29_129__SDO_AIA_AIA_304.jp2", NULL };
        if (!opj_encoder_set_extra_options(codec, options)) {
            fprintf(stderr, "Failed to set list of jp2 files to include: opj_encoder_set_extra_options\n");
            goto fin;
        }
    }

    // /* open a byte stream for writing */
    outfile = opj_stream_create_default_file_stream("sample.jpx", OPJ_FALSE);
    if (!outfile) {
        fprintf(stderr, "Failed to allocate memory for the output file");
        goto fin;
    }

    bSuccess = opj_start_compress(codec, 1, outfile);
    if (!bSuccess) {
        fprintf(stderr, "Failed to create the jpx image: opj_start_compress\n");
        goto fin;
    }

    bSuccess = bSuccess && opj_encode(codec, outfile);
    if (!bSuccess) {
        fprintf(stderr, "Failed to encode the image: opj_encode\n");
    }

    bSuccess = bSuccess && opj_end_compress(codec, outfile);
    if (!bSuccess) {
        fprintf(stderr, "Failed to encode the image: opj_end_compress\n");
        goto fin;
    }

    ret = 0;

fin:
    if (codec) {
        opj_destroy_codec(codec);
    }
    if (outfile) {
        opj_stream_destroy(outfile);
    }
    return ret;
}