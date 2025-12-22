/*
 * Subsampled image and ROI encoder fuzzer
 * Covers: subsampling (dx/dy != 1), ROI encoding, different color spaces
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "openjpeg.h"

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len);

static const size_t kMaxInputSize = 32 * 1024;

static void DummyCallback(const char*, void*) {}

static OPJ_SIZE_T WriteCallback(void*, OPJ_SIZE_T nBytes, void*)
{
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
    if (len < 30 || len > kMaxInputSize) return 0;

    size_t pos = 0;

    // Image parameters
    OPJ_UINT32 width = (ReadByte(buf, len, &pos) % 48) + 16;
    OPJ_UINT32 height = (ReadByte(buf, len, &pos) % 48) + 16;
    OPJ_UINT32 numComps = (ReadByte(buf, len, &pos) % 4) + 1;
    OPJ_UINT32 prec = (ReadByte(buf, len, &pos) % 12) + 1;
    OPJ_BOOL sgnd = ReadByte(buf, len, &pos) & 1;

    // Subsampling factors
    uint8_t subsamp_type = ReadByte(buf, len, &pos) % 4;

    // ROI parameters
    uint8_t use_roi = ReadByte(buf, len, &pos) & 1;
    uint8_t roi_comp = ReadByte(buf, len, &pos);
    uint8_t roi_shift = ReadByte(buf, len, &pos);

    // Color space
    uint8_t color_space_idx = ReadByte(buf, len, &pos) % 5;

    // Other encoder parameters
    uint8_t codec_type = ReadByte(buf, len, &pos) & 1;
    uint8_t irreversible = ReadByte(buf, len, &pos) & 1;
    uint8_t num_resolutions = (ReadByte(buf, len, &pos) % 4) + 1;

    // Create image with subsampling
    opj_image_cmptparm_t* cmptparms = (opj_image_cmptparm_t*)calloc(numComps, sizeof(opj_image_cmptparm_t));
    if (!cmptparms) return 0;

    for (OPJ_UINT32 i = 0; i < numComps; i++) {
        // Apply different subsampling patterns
        switch (subsamp_type) {
            case 0:  // 4:4:4 (no subsampling)
                cmptparms[i].dx = 1;
                cmptparms[i].dy = 1;
                break;
            case 1:  // 4:2:2 (horizontal subsampling for chroma)
                cmptparms[i].dx = (i > 0) ? 2 : 1;
                cmptparms[i].dy = 1;
                break;
            case 2:  // 4:2:0 (both horizontal and vertical for chroma)
                cmptparms[i].dx = (i > 0) ? 2 : 1;
                cmptparms[i].dy = (i > 0) ? 2 : 1;
                break;
            case 3:  // 4:1:1
                cmptparms[i].dx = (i > 0) ? 4 : 1;
                cmptparms[i].dy = 1;
                break;
        }

        // Calculate actual component dimensions
        cmptparms[i].w = (width + cmptparms[i].dx - 1) / cmptparms[i].dx;
        cmptparms[i].h = (height + cmptparms[i].dy - 1) / cmptparms[i].dy;
        cmptparms[i].prec = prec;
        cmptparms[i].sgnd = sgnd;
    }

    // Select color space
    OPJ_COLOR_SPACE colorSpace;
    switch (color_space_idx) {
        case 0: colorSpace = OPJ_CLRSPC_UNKNOWN; break;
        case 1: colorSpace = OPJ_CLRSPC_SRGB; break;
        case 2: colorSpace = OPJ_CLRSPC_GRAY; break;
        case 3: colorSpace = OPJ_CLRSPC_SYCC; break;
        case 4: colorSpace = OPJ_CLRSPC_EYCC; break;
        default: colorSpace = OPJ_CLRSPC_UNSPECIFIED; break;
    }

    opj_image_t* image = opj_image_create(numComps, cmptparms, colorSpace);
    free(cmptparms);
    if (!image) return 0;

    image->x0 = 0;
    image->y0 = 0;
    image->x1 = width;
    image->y1 = height;

    // Fill image data
    for (OPJ_UINT32 c = 0; c < numComps; c++) {
        if (!image->comps[c].data) {
            opj_image_destroy(image);
            return 0;
        }
        size_t compPixels = (size_t)image->comps[c].w * image->comps[c].h;
        for (size_t p = 0; p < compPixels; p++) {
            image->comps[c].data[p] = (pos < len) ? buf[pos++] : 0;
        }
    }

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

    // Setup encoding parameters
    opj_cparameters_t params;
    opj_set_default_encoder_parameters(&params);

    params.numresolution = num_resolutions;
    params.irreversible = irreversible;
    params.tcp_numlayers = 1;
    params.tcp_rates[0] = 0;
    params.cp_disto_alloc = 1;

    // Setup ROI if requested
    if (use_roi && numComps > 0) {
        params.roi_compno = roi_comp % numComps;
        params.roi_shift = roi_shift % 38;  // Valid range 0-37
    }

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

    opj_stream_set_write_function(stream, WriteCallback);
    opj_stream_set_seek_function(stream, SeekCallback);
    opj_stream_set_skip_function(stream, SkipCallback);
    opj_stream_set_user_data(stream, NULL, NULL);

    // Encode
    if (opj_start_compress(codec, image, stream)) {
        opj_encode(codec, stream);
        opj_end_compress(codec, stream);
    }

    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    opj_image_destroy(image);

    return 0;
}
