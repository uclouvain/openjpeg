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

/*
 * Round-trip fuzzer: Encode image data, then decode the result.
 * This catches encoder bugs that produce invalid output, and exercises
 * both encode and decode paths together.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include "openjpeg.h"

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len);

static const size_t kMaxInputSize = 32 * 1024;
static const OPJ_UINT32 kMaxWidth = 128;
static const OPJ_UINT32 kMaxHeight = 128;
static const OPJ_UINT32 kMaxComponents = 4;

// Memory buffer for encoding output
static std::vector<uint8_t> g_encodedData;
static size_t g_encodedSize;
static size_t g_encodedPos;

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

// Write callback for encoder
static OPJ_SIZE_T WriteCallback(void* pBuffer, OPJ_SIZE_T nBytes, void* pUserData)
{
    (void)pUserData;
    size_t newSize = g_encodedPos + nBytes;
    if (newSize > g_encodedData.size()) {
        // Limit maximum encoded size
        if (newSize > 16 * 1024 * 1024) {
            return (OPJ_SIZE_T)-1;
        }
        g_encodedData.resize(newSize);
    }
    memcpy(g_encodedData.data() + g_encodedPos, pBuffer, nBytes);
    g_encodedPos += nBytes;
    if (g_encodedPos > g_encodedSize) {
        g_encodedSize = g_encodedPos;
    }
    return nBytes;
}

static OPJ_BOOL WriteSeekCallback(OPJ_OFF_T nBytes, void* pUserData)
{
    (void)pUserData;
    if (nBytes < 0) {
        return OPJ_FALSE;
    }
    g_encodedPos = (size_t)nBytes;
    return OPJ_TRUE;
}

static OPJ_OFF_T WriteSkipCallback(OPJ_OFF_T nBytes, void* pUserData)
{
    (void)pUserData;
    g_encodedPos += (size_t)nBytes;
    return nBytes;
}

// Read callback for decoder
typedef struct {
    const uint8_t* data;
    size_t size;
    size_t pos;
} ReadState;

static OPJ_SIZE_T ReadCallback(void* pBuffer, OPJ_SIZE_T nBytes, void* pUserData)
{
    ReadState* state = (ReadState*)pUserData;
    if (state->pos >= state->size) {
        return (OPJ_SIZE_T)-1;
    }
    size_t toRead = nBytes;
    if (state->pos + toRead > state->size) {
        toRead = state->size - state->pos;
    }
    if (toRead == 0) {
        return (OPJ_SIZE_T)-1;
    }
    memcpy(pBuffer, state->data + state->pos, toRead);
    state->pos += toRead;
    return toRead;
}

static OPJ_BOOL ReadSeekCallback(OPJ_OFF_T nBytes, void* pUserData)
{
    ReadState* state = (ReadState*)pUserData;
    state->pos = (size_t)nBytes;
    return OPJ_TRUE;
}

static OPJ_OFF_T ReadSkipCallback(OPJ_OFF_T nBytes, void* pUserData)
{
    ReadState* state = (ReadState*)pUserData;
    state->pos += (size_t)nBytes;
    return nBytes;
}

static uint8_t ReadByte(const uint8_t* buf, size_t len, size_t* pos)
{
    if (*pos >= len) return 0;
    return buf[(*pos)++];
}

int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    (void)argc;
    (void)argv;
    g_encodedData.reserve(1024 * 1024);  // Pre-allocate 1MB
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len)
{
    if (len < 16 || len > kMaxInputSize) {
        return 0;
    }

    size_t pos = 0;

    // Read image parameters
    OPJ_UINT32 width = (ReadByte(buf, len, &pos) % kMaxWidth) + 1;
    OPJ_UINT32 height = (ReadByte(buf, len, &pos) % kMaxHeight) + 1;
    OPJ_UINT32 numcomps = (ReadByte(buf, len, &pos) % kMaxComponents) + 1;
    OPJ_UINT32 prec = (ReadByte(buf, len, &pos) % 12) + 1;  // 1-12 bits
    OPJ_BOOL sgnd = ReadByte(buf, len, &pos) & 1;

    // Read encoding parameters
    uint8_t codec_type = ReadByte(buf, len, &pos) & 1;
    uint8_t irreversible = ReadByte(buf, len, &pos) & 1;
    uint8_t num_resolutions = (ReadByte(buf, len, &pos) % 5) + 1;
    uint8_t prog_order = ReadByte(buf, len, &pos) % 5;

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

    // Fill image data
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
    opj_codec_t* encoder = opj_create_compress(format);
    if (!encoder) {
        opj_image_destroy(image);
        return 0;
    }

    opj_set_info_handler(encoder, InfoCallback, NULL);
    opj_set_warning_handler(encoder, WarningCallback, NULL);
    opj_set_error_handler(encoder, ErrorCallback, NULL);

    opj_cparameters_t cparams;
    opj_set_default_encoder_parameters(&cparams);
    cparams.prog_order = (OPJ_PROG_ORDER)prog_order;
    cparams.numresolution = num_resolutions;
    cparams.irreversible = irreversible;
    cparams.tcp_numlayers = 1;
    cparams.tcp_rates[0] = 0;  // Lossless
    cparams.cp_disto_alloc = 1;

    if (!opj_setup_encoder(encoder, &cparams, image)) {
        opj_destroy_codec(encoder);
        opj_image_destroy(image);
        return 0;
    }

    // Create output stream
    opj_stream_t* outStream = opj_stream_create(1024, OPJ_FALSE);
    if (!outStream) {
        opj_destroy_codec(encoder);
        opj_image_destroy(image);
        return 0;
    }

    g_encodedData.clear();
    g_encodedSize = 0;
    g_encodedPos = 0;

    opj_stream_set_write_function(outStream, WriteCallback);
    opj_stream_set_seek_function(outStream, WriteSeekCallback);
    opj_stream_set_skip_function(outStream, WriteSkipCallback);
    opj_stream_set_user_data(outStream, NULL, NULL);

    // Encode
    bool encodeSuccess = false;
    if (opj_start_compress(encoder, image, outStream)) {
        if (opj_encode(encoder, outStream)) {
            if (opj_end_compress(encoder, outStream)) {
                encodeSuccess = true;
            }
        }
    }

    opj_stream_destroy(outStream);
    opj_destroy_codec(encoder);
    opj_image_destroy(image);

    if (!encodeSuccess || g_encodedSize == 0) {
        return 0;
    }

    // Now decode the encoded data
    opj_codec_t* decoder = opj_create_decompress(format);
    if (!decoder) {
        return 0;
    }

    opj_set_info_handler(decoder, InfoCallback, NULL);
    opj_set_warning_handler(decoder, WarningCallback, NULL);
    opj_set_error_handler(decoder, ErrorCallback, NULL);

    opj_dparameters_t dparams;
    opj_set_default_decoder_parameters(&dparams);

    if (!opj_setup_decoder(decoder, &dparams)) {
        opj_destroy_codec(decoder);
        return 0;
    }

    opj_stream_t* inStream = opj_stream_create(1024, OPJ_TRUE);
    if (!inStream) {
        opj_destroy_codec(decoder);
        return 0;
    }

    ReadState readState;
    readState.data = g_encodedData.data();
    readState.size = g_encodedSize;
    readState.pos = 0;

    opj_stream_set_user_data_length(inStream, g_encodedSize);
    opj_stream_set_read_function(inStream, ReadCallback);
    opj_stream_set_seek_function(inStream, ReadSeekCallback);
    opj_stream_set_skip_function(inStream, ReadSkipCallback);
    opj_stream_set_user_data(inStream, &readState, NULL);

    opj_image_t* decodedImage = NULL;
    if (opj_read_header(inStream, decoder, &decodedImage)) {
        opj_decode(decoder, inStream, decodedImage);
        opj_end_decompress(decoder, inStream);
    }

    opj_stream_destroy(inStream);
    opj_destroy_codec(decoder);
    opj_image_destroy(decodedImage);

    return 0;
}
