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
 * This fuzzer exercises component selection decoding:
 * - opj_set_decoded_components() to decode only specific components
 * - Various combinations of component indices
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
    if (len < 10) {
        return 0;
    }

    // Use first bytes for component selection control
    uint8_t ctrl_num_comps = buf[0];
    uint8_t ctrl_comp0 = buf[1];
    uint8_t ctrl_comp1 = buf[2];
    uint8_t ctrl_comp2 = buf[3];
    uint8_t ctrl_comp3 = buf[4];

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

    if (!opj_setup_decoder(pCodec, &parameters)) {
        opj_destroy_codec(pCodec);
        return 0;
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

    // Build component indices array based on fuzz input
    OPJ_UINT32 imageNumComps = psImage->numcomps;
    if (imageNumComps == 0) {
        opj_end_decompress(pCodec, pStream);
        opj_stream_destroy(pStream);
        opj_destroy_codec(pCodec);
        opj_image_destroy(psImage);
        return 0;
    }

    // Determine how many components to select (1-4)
    OPJ_UINT32 numCompsToSelect = (ctrl_num_comps % 4) + 1;
    if (numCompsToSelect > imageNumComps) {
        numCompsToSelect = imageNumComps;
    }

    OPJ_UINT32 compIndices[4];
    compIndices[0] = ctrl_comp0 % imageNumComps;
    compIndices[1] = ctrl_comp1 % imageNumComps;
    compIndices[2] = ctrl_comp2 % imageNumComps;
    compIndices[3] = ctrl_comp3 % imageNumComps;

    // Try to set decoded components (may fail, which is OK)
    // Note: apply_color_transforms must be OPJ_FALSE per API docs
    opj_set_decoded_components(pCodec, numCompsToSelect, compIndices, OPJ_FALSE);

    // Limit decode area to prevent OOM
    OPJ_UINT32 width = psImage->x1 - psImage->x0;
    OPJ_UINT32 height = psImage->y1 - psImage->y0;

    OPJ_UINT32 width_to_read = width;
    if (width_to_read > 1024) {
        width_to_read = 1024;
    }
    OPJ_UINT32 height_to_read = height;
    if (height_to_read > 1024) {
        height_to_read = 1024;
    }

    if (opj_set_decode_area(pCodec, psImage,
                            psImage->x0, psImage->y0,
                            psImage->x0 + width_to_read,
                            psImage->y0 + height_to_read)) {
        opj_decode(pCodec, pStream, psImage);
    }

    opj_end_decompress(pCodec, pStream);
    opj_stream_destroy(pStream);
    opj_destroy_codec(pCodec);
    opj_image_destroy(psImage);

    return 0;
}
