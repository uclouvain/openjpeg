/*
 * Extended encoder options fuzzer
 * Covers: opj_encoder_set_extra_options with PLT, TLM, GUARD_BITS
 * Also tests various codec modes, code block sizes, and precinct sizes
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

    // Extended options
    uint8_t use_plt = ReadByte(buf, len, &pos) & 1;
    uint8_t use_tlm = ReadByte(buf, len, &pos) & 1;
    uint8_t guard_bits = ReadByte(buf, len, &pos) % 8;  // 0-7
    uint8_t codec_type = ReadByte(buf, len, &pos) & 1;
    uint8_t irreversible = ReadByte(buf, len, &pos) & 1;
    uint8_t prog_order = ReadByte(buf, len, &pos) % 5;
    uint8_t num_resolutions = (ReadByte(buf, len, &pos) % 5) + 1;

    // Code block size (power of 2: 4, 8, 16, 32, 64)
    uint8_t cblk_exp_w = (ReadByte(buf, len, &pos) % 5) + 2;
    uint8_t cblk_exp_h = (ReadByte(buf, len, &pos) % 5) + 2;

    // Precinct sizes
    uint8_t use_precincts = ReadByte(buf, len, &pos) & 1;
    uint8_t prec_exp = (ReadByte(buf, len, &pos) % 4) + 6;  // 64-512

    // SOP/EPH markers
    uint8_t use_sop = ReadByte(buf, len, &pos) & 1;
    uint8_t use_eph = ReadByte(buf, len, &pos) & 1;

    // Create image
    opj_image_cmptparm_t* cmptparms = (opj_image_cmptparm_t*)calloc(numComps, sizeof(opj_image_cmptparm_t));
    if (!cmptparms) return 0;

    for (OPJ_UINT32 i = 0; i < numComps; i++) {
        cmptparms[i].dx = 1;
        cmptparms[i].dy = 1;
        cmptparms[i].w = width;
        cmptparms[i].h = height;
        cmptparms[i].prec = prec;
        cmptparms[i].sgnd = sgnd;
    }

    OPJ_COLOR_SPACE colorSpace = (numComps >= 3) ? OPJ_CLRSPC_SRGB : OPJ_CLRSPC_GRAY;
    opj_image_t* image = opj_image_create(numComps, cmptparms, colorSpace);
    free(cmptparms);
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
    params.tcp_numlayers = 1;
    params.tcp_rates[0] = 0;
    params.cp_disto_alloc = 1;

    // Code block size
    params.cblockw_init = 1 << cblk_exp_w;
    params.cblockh_init = 1 << cblk_exp_h;

    // Precinct sizes
    if (use_precincts) {
        params.csty |= 0x01;  // Use precincts
        params.res_spec = num_resolutions;
        for (int i = 0; i < num_resolutions; i++) {
            params.prcw_init[i] = 1 << prec_exp;
            params.prch_init[i] = 1 << prec_exp;
        }
    }

    // SOP/EPH markers
    if (use_sop) params.csty |= 0x02;
    if (use_eph) params.csty |= 0x04;

    if (!opj_setup_encoder(codec, &params, image)) {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return 0;
    }

    // Set extended options using opj_encoder_set_extra_options
    char pltOpt[16], tlmOpt[16], guardOpt[16];
    snprintf(pltOpt, sizeof(pltOpt), "PLT=%s", use_plt ? "YES" : "NO");
    snprintf(tlmOpt, sizeof(tlmOpt), "TLM=%s", use_tlm ? "YES" : "NO");
    snprintf(guardOpt, sizeof(guardOpt), "GUARD_BITS=%d", guard_bits);

    const char* options[] = {pltOpt, tlmOpt, guardOpt, NULL};
    opj_encoder_set_extra_options(codec, options);

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
