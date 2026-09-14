/*
 * Tile-based encoder fuzzer - uses opj_write_tile() API
 * This covers: opj_j2k_write_tile, opj_jp2_write_tile, opj_j2k_post_write_tile
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

static void DummyCallback(const char*, void*) {}

static OPJ_SIZE_T WriteCallback(void*, OPJ_SIZE_T nBytes, void* pUserData)
{
    size_t* written = (size_t*)pUserData;
    *written += nBytes;
    return nBytes;
}

static OPJ_BOOL SeekCallback(OPJ_OFF_T, void*) { return OPJ_TRUE; }
static OPJ_OFF_T SkipCallback(OPJ_OFF_T nBytes, void*) { return nBytes; }

static uint8_t ReadByte(const uint8_t* buf, size_t len, size_t* pos)
{
    if (*pos >= len) return 0;
    return buf[(*pos)++];
}

int LLVMFuzzerInitialize(int*, char***) { return 0; }

int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len)
{
    if (len < 20 || len > kMaxInputSize) return 0;

    size_t pos = 0;

    // Image parameters
    OPJ_UINT32 imgWidth = (ReadByte(buf, len, &pos) % 64) + 16;   // 16-79
    OPJ_UINT32 imgHeight = (ReadByte(buf, len, &pos) % 64) + 16;  // 16-79
    OPJ_UINT32 numComps = (ReadByte(buf, len, &pos) % 3) + 1;     // 1-3
    OPJ_UINT32 prec = (ReadByte(buf, len, &pos) % 8) + 1;         // 1-8 bits
    OPJ_BOOL sgnd = ReadByte(buf, len, &pos) & 1;

    // Tile parameters
    OPJ_UINT32 tileWidth = (ReadByte(buf, len, &pos) % 32) + 8;   // 8-39
    OPJ_UINT32 tileHeight = (ReadByte(buf, len, &pos) % 32) + 8;  // 8-39
    uint8_t codec_type = ReadByte(buf, len, &pos) & 1;
    uint8_t irreversible = ReadByte(buf, len, &pos) & 1;
    uint8_t num_resolutions = (ReadByte(buf, len, &pos) % 4) + 1;

    // Create tile-based image using opj_image_tile_create
    opj_image_cmptparm_t* cmptparms = (opj_image_cmptparm_t*)calloc(numComps, sizeof(opj_image_cmptparm_t));
    if (!cmptparms) return 0;

    for (OPJ_UINT32 i = 0; i < numComps; i++) {
        cmptparms[i].dx = 1;
        cmptparms[i].dy = 1;
        cmptparms[i].w = imgWidth;
        cmptparms[i].h = imgHeight;
        cmptparms[i].prec = prec;
        cmptparms[i].sgnd = sgnd;
    }

    OPJ_COLOR_SPACE colorSpace = (numComps >= 3) ? OPJ_CLRSPC_SRGB : OPJ_CLRSPC_GRAY;
    opj_image_t* image = opj_image_tile_create(numComps, cmptparms, colorSpace);
    free(cmptparms);
    if (!image) return 0;

    image->x0 = 0;
    image->y0 = 0;
    image->x1 = imgWidth;
    image->y1 = imgHeight;

    // Create encoder
    OPJ_CODEC_FORMAT format = codec_type ? OPJ_CODEC_JP2 : OPJ_CODEC_J2K;
    opj_codec_t* codec = opj_create_compress(format);
    if (!codec) {
        opj_image_destroy(image);
        return 0;
    }

    opj_set_info_handler(codec, DummyCallback, NULL);
    opj_set_warning_handler(codec, DummyCallback, NULL);
    opj_set_error_handler(codec, DummyCallback, NULL);

    // Setup encoding parameters with tiles
    opj_cparameters_t params;
    opj_set_default_encoder_parameters(&params);
    params.tile_size_on = OPJ_TRUE;
    params.cp_tx0 = 0;
    params.cp_ty0 = 0;
    params.cp_tdx = tileWidth;
    params.cp_tdy = tileHeight;
    params.numresolution = num_resolutions;
    params.irreversible = irreversible;
    params.tcp_numlayers = 1;
    params.tcp_rates[0] = 0;
    params.cp_disto_alloc = 1;

    if (!opj_setup_encoder(codec, &params, image)) {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return 0;
    }

    // Create output stream
    opj_stream_t* stream = opj_stream_create(1024, OPJ_FALSE);
    if (!stream) {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return 0;
    }

    size_t bytesWritten = 0;
    opj_stream_set_write_function(stream, WriteCallback);
    opj_stream_set_seek_function(stream, SeekCallback);
    opj_stream_set_skip_function(stream, SkipCallback);
    opj_stream_set_user_data(stream, &bytesWritten, NULL);

    if (!opj_start_compress(codec, image, stream)) {
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return 0;
    }

    // Calculate number of tiles
    OPJ_UINT32 numTilesX = (imgWidth + tileWidth - 1) / tileWidth;
    OPJ_UINT32 numTilesY = (imgHeight + tileHeight - 1) / tileHeight;
    OPJ_UINT32 numTiles = numTilesX * numTilesY;

    // Limit tiles to prevent excessive processing
    if (numTiles > 16) numTiles = 16;

    // Write each tile using opj_write_tile
    for (OPJ_UINT32 tileIdx = 0; tileIdx < numTiles; tileIdx++) {
        // Calculate tile dimensions
        OPJ_UINT32 tileX = tileIdx % numTilesX;
        OPJ_UINT32 tileY = tileIdx / numTilesX;
        OPJ_UINT32 tileX0 = tileX * tileWidth;
        OPJ_UINT32 tileY0 = tileY * tileHeight;
        OPJ_UINT32 tileX1 = (tileX0 + tileWidth > imgWidth) ? imgWidth : tileX0 + tileWidth;
        OPJ_UINT32 tileY1 = (tileY0 + tileHeight > imgHeight) ? imgHeight : tileY0 + tileHeight;
        OPJ_UINT32 tileSizeX = tileX1 - tileX0;
        OPJ_UINT32 tileSizeY = tileY1 - tileY0;

        // Calculate data size for this tile
        size_t bytesPerSample = (prec <= 8) ? 1 : ((prec <= 16) ? 2 : 4);
        size_t tileDataSize = tileSizeX * tileSizeY * numComps * bytesPerSample;

        // Allocate and fill tile data from fuzz input
        OPJ_BYTE* tileData = (OPJ_BYTE*)malloc(tileDataSize);
        if (!tileData) break;

        for (size_t i = 0; i < tileDataSize; i++) {
            tileData[i] = (pos < len) ? buf[pos++] : 0;
        }

        // Write tile
        opj_write_tile(codec, tileIdx, tileData, (OPJ_UINT32)tileDataSize, stream);
        free(tileData);
    }

    opj_end_compress(codec, stream);
    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    opj_image_destroy(image);

    return 0;
}
