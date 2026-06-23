/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2024, OpenJPEG contributors
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

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "openjpeg.h"

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len);

// Maximum input size to prevent OOM
static const size_t kMaxInputSize = 64 * 1024;

// Maximum image dimensions to prevent excessive memory usage
static const OPJ_UINT32 kMaxWidth = 256;
static const OPJ_UINT32 kMaxHeight = 256;
static const OPJ_UINT32 kMaxComponents = 4;

typedef struct {
    const uint8_t* data;
    size_t size;
    size_t offset;
} MemWriteStream;

static void ErrorCallback(const char* msg, void* client_data)
{
    (void)msg;
    (void)client_data;
}

static void WarningCallback(const char* msg, void* client_data)
{
    (void)msg;
    (void)client_data;
}

static void InfoCallback(const char* msg, void* client_data)
{
    (void)msg;
    (void)client_data;
}

static OPJ_SIZE_T WriteCallback(void* pBuffer, OPJ_SIZE_T nBytes, void* pUserData)
{
    MemWriteStream* stream = (MemWriteStream*)pUserData;
    // Discard output - we're only testing the encoder, not the output
    (void)pBuffer;
    stream->offset += nBytes;
    return nBytes;
}

static OPJ_BOOL SeekCallback(OPJ_OFF_T nBytes, void* pUserData)
{
    MemWriteStream* stream = (MemWriteStream*)pUserData;
    stream->offset = (size_t)nBytes;
    return OPJ_TRUE;
}

static OPJ_OFF_T SkipCallback(OPJ_OFF_T nBytes, void* pUserData)
{
    MemWriteStream* stream = (MemWriteStream*)pUserData;
    stream->offset += (size_t)nBytes;
    return nBytes;
}

// Helper to read a value from fuzz input
static uint8_t ReadByte(const uint8_t* buf, size_t len, size_t* pos)
{
    if (*pos >= len) {
        return 0;
    }
    return buf[(*pos)++];
}

static uint16_t ReadUInt16(const uint8_t* buf, size_t len, size_t* pos)
{
    uint16_t val = ReadByte(buf, len, pos);
    val |= (uint16_t)ReadByte(buf, len, pos) << 8;
    return val;
}

int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    (void)argc;
    (void)argv;
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len)
{
    if (len < 16 || len > kMaxInputSize) {
        return 0;
    }

    size_t pos = 0;

    // Read image parameters from fuzz input
    OPJ_UINT32 width = (ReadByte(buf, len, &pos) % kMaxWidth) + 1;
    OPJ_UINT32 height = (ReadByte(buf, len, &pos) % kMaxHeight) + 1;
    OPJ_UINT32 numcomps = (ReadByte(buf, len, &pos) % kMaxComponents) + 1;
    OPJ_UINT32 prec = (ReadByte(buf, len, &pos) % 16) + 1;  // 1-16 bits
    OPJ_BOOL sgnd = ReadByte(buf, len, &pos) & 1;

    // Read encoding parameters
    uint8_t codec_type = ReadByte(buf, len, &pos) & 1;  // 0=J2K, 1=JP2
    uint8_t prog_order = ReadByte(buf, len, &pos) % 5;
    uint8_t num_resolutions = (ReadByte(buf, len, &pos) % 6) + 1;
    uint8_t irreversible = ReadByte(buf, len, &pos) & 1;
    uint8_t tcp_numlayers = (ReadByte(buf, len, &pos) % 10) + 1;
    uint8_t cblockw_exp = (ReadByte(buf, len, &pos) % 4) + 2;  // 4-64
    uint8_t cblockh_exp = (ReadByte(buf, len, &pos) % 4) + 2;  // 4-64
    uint8_t use_tiles = ReadByte(buf, len, &pos) & 1;
    uint8_t tile_size = ReadByte(buf, len, &pos);

    // Create image
    opj_image_cmptparm_t* cmptparms = (opj_image_cmptparm_t*)calloc(numcomps, sizeof(opj_image_cmptparm_t));
    if (!cmptparms) {
        return 0;
    }

    for (OPJ_UINT32 i = 0; i < numcomps; i++) {
        cmptparms[i].dx = 1;
        cmptparms[i].dy = 1;
        cmptparms[i].w = width;
        cmptparms[i].h = height;
        cmptparms[i].x0 = 0;
        cmptparms[i].y0 = 0;
        cmptparms[i].prec = prec;
        cmptparms[i].sgnd = sgnd;
    }

    OPJ_COLOR_SPACE color_space = (numcomps >= 3) ? OPJ_CLRSPC_SRGB : OPJ_CLRSPC_GRAY;
    opj_image_t* image = opj_image_create(numcomps, cmptparms, color_space);
    free(cmptparms);

    if (!image) {
        return 0;
    }

    image->x0 = 0;
    image->y0 = 0;
    image->x1 = width;
    image->y1 = height;

    // Fill image data from remaining fuzz input
    size_t pixels = (size_t)width * height;
    for (OPJ_UINT32 c = 0; c < numcomps; c++) {
        if (!image->comps[c].data) {
            opj_image_destroy(image);
            return 0;
        }
        for (size_t p = 0; p < pixels; p++) {
            if (pos < len) {
                image->comps[c].data[p] = (OPJ_INT32)buf[pos++];
            } else {
                image->comps[c].data[p] = 0;
            }
        }
    }

    // Setup encoder
    OPJ_CODEC_FORMAT format = codec_type ? OPJ_CODEC_JP2 : OPJ_CODEC_J2K;
    opj_codec_t* codec = opj_create_compress(format);
    if (!codec) {
        opj_image_destroy(image);
        return 0;
    }

    opj_set_info_handler(codec, InfoCallback, NULL);
    opj_set_warning_handler(codec, WarningCallback, NULL);
    opj_set_error_handler(codec, ErrorCallback, NULL);

    opj_cparameters_t parameters;
    opj_set_default_encoder_parameters(&parameters);

    parameters.prog_order = (OPJ_PROG_ORDER)prog_order;
    parameters.numresolution = num_resolutions;
    parameters.irreversible = irreversible;
    parameters.tcp_numlayers = tcp_numlayers;
    parameters.cblockw_init = 1 << cblockw_exp;
    parameters.cblockh_init = 1 << cblockh_exp;
    parameters.cp_disto_alloc = 1;

    // Set layer rates
    for (int i = 0; i < tcp_numlayers; i++) {
        parameters.tcp_rates[i] = (float)(tcp_numlayers - i);
    }
    parameters.tcp_rates[tcp_numlayers - 1] = 1.0f;  // Last layer lossless

    // Optionally use tiles
    if (use_tiles && tile_size > 0) {
        OPJ_UINT32 ts = ((tile_size % 64) + 16);
        parameters.tile_size_on = OPJ_TRUE;
        parameters.cp_tx0 = 0;
        parameters.cp_ty0 = 0;
        parameters.cp_tdx = (int)ts;
        parameters.cp_tdy = (int)ts;
    }

    if (!opj_setup_encoder(codec, &parameters, image)) {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return 0;
    }

    // Create output stream (discard output)
    opj_stream_t* stream = opj_stream_create(1024, OPJ_FALSE);
    if (!stream) {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return 0;
    }

    MemWriteStream memStream;
    memStream.data = NULL;
    memStream.size = 0;
    memStream.offset = 0;

    opj_stream_set_write_function(stream, WriteCallback);
    opj_stream_set_seek_function(stream, SeekCallback);
    opj_stream_set_skip_function(stream, SkipCallback);
    opj_stream_set_user_data(stream, &memStream, NULL);

    // Encode
    if (opj_start_compress(codec, image, stream)) {
        opj_encode(codec, stream);
        opj_end_compress(codec, stream);
    }

    // Cleanup
    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    opj_image_destroy(image);

    return 0;
}
