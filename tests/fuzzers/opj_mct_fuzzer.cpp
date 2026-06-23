/*
 * MCT (Multi-Component Transform) encoder fuzzer
 * Covers: opj_set_MCT, custom MCT matrices, Part-2 extensions
 * Also tests various encoder profiles and progression orders
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

static OPJ_SIZE_T WriteCallback(void*, OPJ_SIZE_T nBytes, void*)
{
    return nBytes;  // Discard output
}

static OPJ_BOOL SeekCallback(OPJ_OFF_T, void*) { return OPJ_TRUE; }
static OPJ_OFF_T SkipCallback(OPJ_OFF_T nBytes, void*) { return nBytes; }

static uint8_t ReadByte(const uint8_t* buf, size_t len, size_t* pos)
{
    if (*pos >= len) return 0;
    return buf[(*pos)++];
}

static float ReadFloat(const uint8_t* buf, size_t len, size_t* pos)
{
    uint8_t b0 = ReadByte(buf, len, pos);
    uint8_t b1 = ReadByte(buf, len, pos);
    // Generate a float between -2.0 and 2.0
    int16_t val = (int16_t)((b1 << 8) | b0);
    return (float)val / 16384.0f;
}

int LLVMFuzzerInitialize(int*, char***) { return 0; }

int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len)
{
    if (len < 50 || len > kMaxInputSize) return 0;

    size_t pos = 0;

    // Image parameters - always 3 components for MCT
    OPJ_UINT32 width = (ReadByte(buf, len, &pos) % 32) + 8;   // 8-39
    OPJ_UINT32 height = (ReadByte(buf, len, &pos) % 32) + 8;  // 8-39
    OPJ_UINT32 numComps = 3;  // MCT requires at least 3 components
    OPJ_UINT32 prec = (ReadByte(buf, len, &pos) % 8) + 1;     // 1-8 bits
    OPJ_BOOL sgnd = ReadByte(buf, len, &pos) & 1;

    // Encoder parameters
    uint8_t codec_type = ReadByte(buf, len, &pos) & 1;
    uint8_t use_mct = ReadByte(buf, len, &pos);
    uint8_t prog_order = ReadByte(buf, len, &pos) % 5;
    uint8_t num_resolutions = (ReadByte(buf, len, &pos) % 4) + 1;
    uint8_t irreversible = ReadByte(buf, len, &pos) & 1;
    uint8_t num_layers = (ReadByte(buf, len, &pos) % 5) + 1;
    uint8_t cblk_style = ReadByte(buf, len, &pos);
    uint8_t profile = ReadByte(buf, len, &pos);

    // Create image
    opj_image_cmptparm_t cmptparms[3];
    memset(cmptparms, 0, sizeof(cmptparms));
    for (OPJ_UINT32 i = 0; i < numComps; i++) {
        cmptparms[i].dx = 1;
        cmptparms[i].dy = 1;
        cmptparms[i].w = width;
        cmptparms[i].h = height;
        cmptparms[i].prec = prec;
        cmptparms[i].sgnd = sgnd;
    }

    opj_image_t* image = opj_image_create(numComps, cmptparms, OPJ_CLRSPC_SRGB);
    if (!image) return 0;

    image->x0 = 0;
    image->y0 = 0;
    image->x1 = width;
    image->y1 = height;

    // Fill image data
    size_t pixels = width * height;
    for (OPJ_UINT32 c = 0; c < numComps; c++) {
        if (!image->comps[c].data) {
            opj_image_destroy(image);
            return 0;
        }
        for (size_t p = 0; p < pixels; p++) {
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

    params.prog_order = (OPJ_PROG_ORDER)prog_order;
    params.numresolution = num_resolutions;
    params.irreversible = irreversible;
    params.tcp_numlayers = num_layers;
    params.cp_disto_alloc = 1;
    params.mode = cblk_style & 0x3F;  // Valid cblk_style bits

    // Set layer rates
    for (int i = 0; i < num_layers; i++) {
        params.tcp_rates[i] = (float)(num_layers - i + 1);
    }
    params.tcp_rates[num_layers - 1] = 0;  // Last layer lossless

    // Set profile based on fuzz input
    switch (profile % 6) {
        case 0: params.rsiz = OPJ_PROFILE_NONE; break;
        case 1: params.rsiz = OPJ_PROFILE_0; break;
        case 2: params.rsiz = OPJ_PROFILE_1; break;
        case 3: params.rsiz = OPJ_PROFILE_CINEMA_2K; break;
        case 4: params.rsiz = OPJ_PROFILE_CINEMA_4K; break;
        case 5: params.rsiz = OPJ_PROFILE_PART2 | OPJ_EXTENSION_MCT; break;
    }

    // Setup MCT if requested
    if (use_mct & 0x01) {
        params.tcp_mct = 1;  // Enable standard MCT
    }

    // Try custom MCT if profile supports Part-2
    if ((use_mct & 0x02) && (params.rsiz & OPJ_PROFILE_PART2)) {
        // Read custom MCT matrix (3x3 for 3 components)
        OPJ_FLOAT32 mctMatrix[9];
        for (int i = 0; i < 9; i++) {
            mctMatrix[i] = ReadFloat(buf, len, &pos);
        }

        // Read DC shift values
        OPJ_INT32 dcShift[3];
        for (int i = 0; i < 3; i++) {
            dcShift[i] = (OPJ_INT32)(ReadByte(buf, len, &pos) - 128);
        }

        // Set custom MCT
        opj_set_MCT(&params, mctMatrix, dcShift, numComps);
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
