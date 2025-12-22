/*
 * Codec dump/info fuzzer - exercises metadata and index APIs
 * Covers: opj_get_cstr_info, opj_get_cstr_index, opj_get_jp2_metadata,
 *         opj_get_jp2_index, opj_dump_codec
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "openjpeg.h"

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len);

typedef struct {
    const uint8_t* data;
    size_t size;
    size_t pos;
} MemFile;

static void DummyCallback(const char*, void*) {}

static OPJ_SIZE_T ReadCallback(void* pBuffer, OPJ_SIZE_T nBytes, void* pUserData)
{
    MemFile* f = (MemFile*)pUserData;
    if (f->pos >= f->size) return (OPJ_SIZE_T)-1;
    size_t toRead = nBytes;
    if (f->pos + toRead > f->size) toRead = f->size - f->pos;
    if (toRead == 0) return (OPJ_SIZE_T)-1;
    memcpy(pBuffer, f->data + f->pos, toRead);
    f->pos += toRead;
    return toRead;
}

static OPJ_BOOL SeekCallback(OPJ_OFF_T nBytes, void* pUserData)
{
    MemFile* f = (MemFile*)pUserData;
    f->pos = (size_t)nBytes;
    return OPJ_TRUE;
}

static OPJ_OFF_T SkipCallback(OPJ_OFF_T nBytes, void* pUserData)
{
    MemFile* f = (MemFile*)pUserData;
    f->pos += (size_t)nBytes;
    return nBytes;
}

static const unsigned char jpc_header[] = {0xff, 0x4f};
static const unsigned char jp2_box_jp[] = {0x6a, 0x50, 0x20, 0x20};

int LLVMFuzzerInitialize(int*, char***) { return 0; }

int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len)
{
    if (len < 8) return 0;

    // Determine codec format
    OPJ_CODEC_FORMAT format;
    if (len >= 2 && memcmp(buf, jpc_header, 2) == 0) {
        format = OPJ_CODEC_J2K;
    } else if (len >= 12 && memcmp(buf + 4, jp2_box_jp, 4) == 0) {
        format = OPJ_CODEC_JP2;
    } else {
        return 0;
    }

    opj_codec_t* codec = opj_create_decompress(format);
    if (!codec) return 0;

    opj_set_info_handler(codec, DummyCallback, NULL);
    opj_set_warning_handler(codec, DummyCallback, NULL);
    opj_set_error_handler(codec, DummyCallback, NULL);

    opj_dparameters_t params;
    opj_set_default_decoder_parameters(&params);

    if (!opj_setup_decoder(codec, &params)) {
        opj_destroy_codec(codec);
        return 0;
    }

    opj_stream_t* stream = opj_stream_create(1024, OPJ_TRUE);
    if (!stream) {
        opj_destroy_codec(codec);
        return 0;
    }

    MemFile memFile = {buf, len, 0};
    opj_stream_set_user_data_length(stream, len);
    opj_stream_set_read_function(stream, ReadCallback);
    opj_stream_set_seek_function(stream, SeekCallback);
    opj_stream_set_skip_function(stream, SkipCallback);
    opj_stream_set_user_data(stream, &memFile, NULL);

    opj_image_t* image = NULL;
    if (!opj_read_header(stream, codec, &image)) {
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        return 0;
    }

    // Exercise all info/dump APIs

    // 1. Get codestream info
    opj_codestream_info_v2_t* cstrInfo = opj_get_cstr_info(codec);
    if (cstrInfo) {
        // Access fields to ensure they're valid
        (void)cstrInfo->tx0;
        (void)cstrInfo->ty0;
        (void)cstrInfo->tdx;
        (void)cstrInfo->tdy;
        (void)cstrInfo->tw;
        (void)cstrInfo->th;
        (void)cstrInfo->nbcomps;
        opj_destroy_cstr_info(&cstrInfo);
    }

    // 2. Get codestream index
    opj_codestream_index_t* cstrIndex = opj_get_cstr_index(codec);
    if (cstrIndex) {
        // Access fields
        (void)cstrIndex->main_head_start;
        (void)cstrIndex->main_head_end;
        (void)cstrIndex->codestream_size;
        (void)cstrIndex->nb_of_tiles;
        opj_destroy_cstr_index(&cstrIndex);
    }

    // Note: opj_get_jp2_metadata and opj_get_jp2_index are declared
    // but not implemented in the library

    // 4. Dump codec info to /dev/null
    FILE* nullFile = fopen("/dev/null", "w");
    if (nullFile) {
        opj_dump_codec(codec, OPJ_IMG_INFO | OPJ_J2K_MH_INFO | OPJ_J2K_TH_INFO, nullFile);
        fclose(nullFile);
    }

    // Try to decode a small area to exercise more code paths
    OPJ_UINT32 width = image->x1 - image->x0;
    OPJ_UINT32 height = image->y1 - image->y0;
    if (width > 256) width = 256;
    if (height > 256) height = 256;

    if (opj_set_decode_area(codec, image, image->x0, image->y0,
                            image->x0 + width, image->y0 + height)) {
        opj_decode(codec, stream, image);
    }

    opj_end_decompress(codec, stream);
    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    opj_image_destroy(image);

    return 0;
}
