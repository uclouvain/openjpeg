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
 * This fuzzer exercises the tile-based decoding API:
 * - opj_get_decoded_tile()
 * - opj_read_tile_header()
 * - opj_decode_tile_data()
 *
 * These functions have 0% coverage in the existing fuzzers.
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
    if (len < 8) {
        return 0;
    }

    // Determine codec format
    OPJ_CODEC_FORMAT eCodecFormat;
    if (len >= sizeof(jpc_header) &&
            memcmp(buf, jpc_header, sizeof(jpc_header)) == 0) {
        eCodecFormat = OPJ_CODEC_J2K;
    } else if (len >= 12 &&
               memcmp(buf + 4, jp2_box_jp, sizeof(jp2_box_jp)) == 0) {
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
    memFile.pabyData = buf;
    memFile.nLength = len;
    memFile.nCurPos = 0;

    opj_stream_set_user_data_length(pStream, len);
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

    // Get codestream info to determine number of tiles
    opj_codestream_info_v2_t* pCodeStreamInfo = opj_get_cstr_info(pCodec);
    if (!pCodeStreamInfo) {
        opj_end_decompress(pCodec, pStream);
        opj_stream_destroy(pStream);
        opj_destroy_codec(pCodec);
        opj_image_destroy(psImage);
        return 0;
    }

    OPJ_UINT32 numTilesX = pCodeStreamInfo->tw;
    OPJ_UINT32 numTilesY = pCodeStreamInfo->th;
    OPJ_UINT32 numTiles = numTilesX * numTilesY;

    opj_destroy_cstr_info(&pCodeStreamInfo);

    // Limit number of tiles to process
    if (numTiles > 16) {
        numTiles = 16;
    }

    // Use last byte of input to select decoding method
    uint8_t decodeMethod = (len > 0) ? buf[len - 1] : 0;

    if (decodeMethod & 0x01) {
        // Method 1: Use opj_get_decoded_tile() to decode individual tiles
        for (OPJ_UINT32 tileIndex = 0; tileIndex < numTiles; tileIndex++) {
            // Reset stream position for each tile
            memFile.nCurPos = 0;

            if (!opj_get_decoded_tile(pCodec, pStream, psImage, tileIndex)) {
                // Tile decode failed, which is OK for malformed input
                break;
            }
        }
    } else {
        // Method 2: Use opj_read_tile_header() + opj_decode_tile_data()
        OPJ_UINT32 tileIndex = 0;
        OPJ_UINT32 dataSize = 0;
        OPJ_INT32 tileX0, tileY0, tileX1, tileY1;
        OPJ_UINT32 nbComps;
        OPJ_BOOL shouldGoOn = OPJ_TRUE;

        while (shouldGoOn) {
            if (!opj_read_tile_header(pCodec, pStream, &tileIndex, &dataSize,
                                       &tileX0, &tileY0, &tileX1, &tileY1,
                                       &nbComps, &shouldGoOn)) {
                break;
            }

            if (!shouldGoOn) {
                break;
            }

            // Limit data size to prevent OOM
            if (dataSize > 16 * 1024 * 1024) {
                break;
            }

            if (dataSize > 0) {
                OPJ_BYTE* tileData = (OPJ_BYTE*)malloc(dataSize);
                if (!tileData) {
                    break;
                }

                if (!opj_decode_tile_data(pCodec, tileIndex, tileData, dataSize, pStream)) {
                    free(tileData);
                    break;
                }

                free(tileData);
            }

            // Limit iterations
            if (tileIndex >= numTiles) {
                break;
            }
        }
    }

    opj_end_decompress(pCodec, pStream);
    opj_stream_destroy(pStream);
    opj_destroy_codec(pCodec);
    opj_image_destroy(psImage);

    return 0;
}
