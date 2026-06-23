/*
 * opj_compress_fuzzer.cpp
 *
 * Fuzz target for the openjpeg COMPRESS (encode) path.
 *
 * The existing OSS-Fuzz harnesses cover only DECOMPRESS:
 *   opj_decompress_fuzzer_J2K  - .j2k / .jph codestream decode
 *   opj_decompress_fuzzer_JP2  - .jp2 container decode
 *
 * This harness exercises the encode path in src/lib/openjp2/:
 *   j2k.c  - J2K codestream encoder, tier-1/tier-2 coding, DWT, RD-opt
 *   jp2.c  - JP2 container writer (box serialization)
 *   tcd.c  - tile-component-data encoder
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "openjpeg.h"

/* Silence callbacks */
static void noop(const char *, void *) {}

/* Write-side memory stream helpers */
typedef struct {
    uint8_t *buf;
    size_t   pos;
    size_t   cap;
} WriteBuf;

static OPJ_SIZE_T write_cb(void *buf, OPJ_SIZE_T n, void *data)
{
    WriteBuf *wb = (WriteBuf *)data;
    if (wb->pos + n > wb->cap) {
        size_t newcap = wb->pos + n + 65536;
        uint8_t *tmp = (uint8_t *)realloc(wb->buf, newcap);
        if (!tmp) return (OPJ_SIZE_T)-1;
        wb->buf = tmp; wb->cap = newcap;
    }
    memcpy(wb->buf + wb->pos, buf, n);
    wb->pos += n;
    return n;
}
static OPJ_OFF_T skip_cb(OPJ_OFF_T n, void *data)
{
    WriteBuf *wb = (WriteBuf *)data;
    wb->pos += (size_t)n;
    return n;
}
static OPJ_BOOL seek_cb(OPJ_OFF_T pos, void *data)
{
    WriteBuf *wb = (WriteBuf *)data;
    wb->pos = (size_t)pos;
    return OPJ_TRUE;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 8) return 0;

    OPJ_UINT32 width   = (OPJ_UINT32)(data[0] % 32) + 1;   /* 1–32 */
    OPJ_UINT32 height  = (OPJ_UINT32)(data[1] % 32) + 1;   /* 1–32 */
    OPJ_UINT32 ncomps  = (OPJ_UINT32)(data[2] % 3)  + 1;   /* 1–3 */
    int        lossy   =  data[3] & 1;
    OPJ_CODEC_FORMAT fmt = (data[4] & 1) ? OPJ_CODEC_JP2 : OPJ_CODEC_J2K;

    size_t need = 5 + (size_t)width * height * ncomps;
    if (size < need) return 0;

    opj_image_cmptparm_t cmp[3];
    memset(cmp, 0, sizeof(cmp));
    for (OPJ_UINT32 i = 0; i < ncomps; i++) {
        cmp[i].prec = 8; cmp[i].bpp = 8; cmp[i].sgnd = 0;
        cmp[i].dx = cmp[i].dy = 1;
        cmp[i].w = width; cmp[i].h = height;
    }
    OPJ_COLOR_SPACE cs = (ncomps >= 3) ? OPJ_CLRSPC_SRGB : OPJ_CLRSPC_GRAY;
    opj_image_t *img = opj_image_create(ncomps, cmp, cs);
    if (!img) return 0;
    img->x0 = img->y0 = 0;
    img->x1 = width; img->y1 = height;

    const uint8_t *px = data + 5;
    for (OPJ_UINT32 c = 0; c < ncomps; c++)
        for (OPJ_UINT32 i = 0; i < width * height; i++)
            img->comps[c].data[i] = (OPJ_INT32)px[i];

    opj_cparameters_t p;
    opj_set_default_encoder_parameters(&p);
    p.cod_format     = (int)fmt;
    p.tcp_numlayers  = 1;
    p.tcp_rates[0]   = lossy ? 10.0f : 0.0f;
    p.cp_disto_alloc = 1;

    opj_codec_t *codec = opj_create_compress(fmt);
    if (!codec) { opj_image_destroy(img); return 0; }
    opj_set_error_handler  (codec, noop, NULL);
    opj_set_warning_handler(codec, noop, NULL);
    opj_set_info_handler   (codec, noop, NULL);

    if (!opj_setup_encoder(codec, &p, img)) {
        opj_destroy_codec(codec); opj_image_destroy(img); return 0;
    }

    /* Create write stream via callbacks */
    WriteBuf wb = {NULL, 0, 0};
    opj_stream_t *stream = opj_stream_create(65536, OPJ_FALSE);
    if (!stream) {
        opj_destroy_codec(codec); opj_image_destroy(img); return 0;
    }
    opj_stream_set_write_function(stream, write_cb);
    opj_stream_set_skip_function (stream, skip_cb);
    opj_stream_set_seek_function (stream, seek_cb);
    opj_stream_set_user_data(stream, &wb, NULL);

    if (opj_start_compress(codec, img, stream)) {
        opj_encode(codec, stream);
        opj_end_compress(codec, stream);
    }

    opj_stream_destroy(stream);
    if (wb.buf) free(wb.buf);
    opj_destroy_codec(codec);
    opj_image_destroy(img);
    return 0;
}
