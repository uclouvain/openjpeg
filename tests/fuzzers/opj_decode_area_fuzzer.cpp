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
 * This fuzzer exercises partial/area decoding with various options:
 * - opj_set_decode_area() with fuzz-controlled regions
 * - opj_set_decoded_resolution_factor() for multi-resolution decoding
 * - opj_decoder_set_strict_mode() for strict vs lenient parsing
 * - opj_codec_set_threads() for multi-threaded decoding
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "openjpeg.h"

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len);

typedef struct {
    const uint8_t* pabyData;
    size_t nCurPos;
    size_t nLength;
} MemFile;

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

static OPJ_SIZE_T ReadCallback(void* pBuffer, OPJ_SIZE_T nBytes, void* pUserData)
{
    MemFile* memFile = (MemFile*)pUserData;
    if (memFile->nCurPos >= memFile->nLength) {
        return (OPJ_SIZE_T)-1;
    }
    if (memFile->nCurPos + nBytes >= memFile->nLength) {
        size_t nToRead = memFile->nLength - memFile->nCurPos;
        memcpy(pBuffer, memFile->pabyData + memFile->nCurPos, nToRead);
        memFile->nCurPos = memFile->nLength;
        return nToRead;
    }
    if (nBytes == 0) {
        return (OPJ_SIZE_T)-1;
    }
    memcpy(pBuffer, memFile->pabyData + memFile->nCurPos, nBytes);
    memFile->nCurPos += nBytes;
    return nBytes;
}

static OPJ_BOOL SeekCallback(OPJ_OFF_T nBytes, void* pUserData)
{
    MemFile* memFile = (MemFile*)pUserData;
    memFile->nCurPos = (size_t)nBytes;
    return OPJ_TRUE;
}

static OPJ_OFF_T SkipCallback(OPJ_OFF_T nBytes, void* pUserData)
{
    MemFile* memFile = (MemFile*)pUserData;
    memFile->nCurPos += (size_t)nBytes;
    return nBytes;
}

static const unsigned char jpc_header[] = {0xff, 0x4f};
static const unsigned char jp2_box_jp[] = {0x6a, 0x50, 0x20, 0x20};

int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    (void)argc;
    (void)argv;
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len)
{
    if (len < 16) {
        return 0;
    }

    // Use first bytes for fuzzer control parameters
    uint8_t ctrl_strict = buf[0] & 0x01;
    uint8_t ctrl_threads = (buf[0] >> 1) & 0x03;  // 0-3 threads
    uint8_t ctrl_reduce = (buf[0] >> 3) & 0x07;   // 0-7 resolution reduction
    uint8_t ctrl_x0_pct = buf[1];
    uint8_t ctrl_y0_pct = buf[2];
    uint8_t ctrl_x1_pct = buf[3];
    uint8_t ctrl_y1_pct = buf[4];

    // Skip control bytes for codec detection
    const uint8_t* data = buf + 5;
    size_t data_len = len - 5;

    // Determine codec format
    OPJ_CODEC_FORMAT eCodecFormat;
    if (data_len >= sizeof(jpc_header) &&
            memcmp(data, jpc_header, sizeof(jpc_header)) == 0) {
        eCodecFormat = OPJ_CODEC_J2K;
    } else if (data_len >= 12 &&
               memcmp(data + 4, jp2_box_jp, sizeof(jp2_box_jp)) == 0) {
        eCodecFormat = OPJ_CODEC_JP2;
    } else {
        return 0;
    }

    opj_codec_t* pCodec = opj_create_decompress(eCodecFormat);
    if (!pCodec) {
        return 0;
    }

    opj_set_info_handler(pCodec, InfoCallback, NULL);
    opj_set_warning_handler(pCodec, WarningCallback, NULL);
    opj_set_error_handler(pCodec, ErrorCallback, NULL);

    opj_dparameters_t parameters;
    opj_set_default_decoder_parameters(&parameters);
    parameters.cp_reduce = ctrl_reduce;

    if (!opj_setup_decoder(pCodec, &parameters)) {
        opj_destroy_codec(pCodec);
        return 0;
    }

    // Set strict mode based on fuzz input
    opj_decoder_set_strict_mode(pCodec, ctrl_strict ? OPJ_TRUE : OPJ_FALSE);

    // Set thread count based on fuzz input
    if (ctrl_threads > 0) {
        opj_codec_set_threads(pCodec, ctrl_threads);
    }

    opj_stream_t* pStream = opj_stream_create(1024, OPJ_TRUE);
    if (!pStream) {
        opj_destroy_codec(pCodec);
        return 0;
    }

    MemFile memFile;
    memFile.pabyData = data;
    memFile.nLength = data_len;
    memFile.nCurPos = 0;

    opj_stream_set_user_data_length(pStream, data_len);
    opj_stream_set_read_function(pStream, ReadCallback);
    opj_stream_set_seek_function(pStream, SeekCallback);
    opj_stream_set_skip_function(pStream, SkipCallback);
    opj_stream_set_user_data(pStream, &memFile, NULL);

    opj_image_t* psImage = NULL;
    if (!opj_read_header(pStream, pCodec, &psImage)) {
        opj_destroy_codec(pCodec);
        opj_stream_destroy(pStream);
        opj_image_destroy(psImage);
        return 0;
    }

    // Set resolution factor (different from cp_reduce in parameters)
    opj_set_decoded_resolution_factor(pCodec, ctrl_reduce % 4);

    // Calculate decode area based on fuzz-controlled percentages
    OPJ_INT32 img_width = (OPJ_INT32)(psImage->x1 - psImage->x0);
    OPJ_INT32 img_height = (OPJ_INT32)(psImage->y1 - psImage->y0);

    if (img_width <= 0 || img_height <= 0) {
        opj_end_decompress(pCodec, pStream);
        opj_stream_destroy(pStream);
        opj_destroy_codec(pCodec);
        opj_image_destroy(psImage);
        return 0;
    }

    // Limit dimensions to prevent OOM
    if (img_width > 4096) img_width = 4096;
    if (img_height > 4096) img_height = 4096;

    OPJ_INT32 x0 = psImage->x0 + (img_width * ctrl_x0_pct) / 256;
    OPJ_INT32 y0 = psImage->y0 + (img_height * ctrl_y0_pct) / 256;
    OPJ_INT32 x1 = psImage->x0 + (img_width * ctrl_x1_pct) / 256;
    OPJ_INT32 y1 = psImage->y0 + (img_height * ctrl_y1_pct) / 256;

    // Ensure valid area (swap if needed, ensure non-zero size)
    if (x0 > x1) { OPJ_INT32 tmp = x0; x0 = x1; x1 = tmp; }
    if (y0 > y1) { OPJ_INT32 tmp = y0; y0 = y1; y1 = tmp; }
    if (x1 <= x0) x1 = x0 + 1;
    if (y1 <= y0) y1 = y0 + 1;

    // Limit area size
    if (x1 - x0 > 1024) x1 = x0 + 1024;
    if (y1 - y0 > 1024) y1 = y0 + 1024;

    if (opj_set_decode_area(pCodec, psImage, x0, y0, x1, y1)) {
        opj_decode(pCodec, pStream, psImage);
    }

    opj_end_decompress(pCodec, pStream);
    opj_stream_destroy(pStream);
    opj_destroy_codec(pCodec);
    opj_image_destroy(psImage);

    return 0;
}
