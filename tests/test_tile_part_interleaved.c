/*
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
 * Non-regression test for https://github.com/uclouvain/openjpeg/issues/1558
 *
 * Some conformant JPEG 2000 codestreams store the tile-parts of the different
 * tiles interleaved (all tiles' first tile-part, then all tiles' second
 * tile-part, ...) and only declare the total number of tile-parts (TNsot) in
 * the last tile-part of each tile. When such a file was decoded tile by tile
 * with opj_get_decoded_tile(), the second and following tiles failed with
 * "Invalid tile part index for tile number N. Got X, expected 0", because the
 * decoder could not build a reliable tile-part index for those tiles and ended
 * up seeking to the wrong SOT.
 *
 * This test builds such an interleaved codestream from scratch (no external
 * data dependency), then decodes every tile individually and checks that the
 * decoded samples match a full-image decode.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "openjpeg.h"

#define IMAGE_W      64
#define IMAGE_H      64
#define TILE_W       32
#define TILE_H       32
#define NUM_RES       6 /* one tile-part per resolution => several tile-parts/tile */

#define J2K_MS_SOT 0xff90
#define J2K_MS_SOD 0xff93
#define J2K_MS_EOC 0xffd9

static void quiet_callback(const char *msg, void *client_data)
{
    (void)msg;
    (void)client_data;
}

/* -------------------------------------------------------------------------- */
/* big-endian helpers                                                         */
/* -------------------------------------------------------------------------- */
static OPJ_UINT32 read_be16(const unsigned char *p)
{
    return ((OPJ_UINT32)p[0] << 8) | p[1];
}
static OPJ_UINT32 read_be32(const unsigned char *p)
{
    return ((OPJ_UINT32)p[0] << 24) | ((OPJ_UINT32)p[1] << 16) |
           ((OPJ_UINT32)p[2] << 8) | p[3];
}

/* -------------------------------------------------------------------------- */
/* Build a sample multi-tile image                                            */
/* -------------------------------------------------------------------------- */
static opj_image_t *create_test_image(void)
{
    opj_image_cmptparm_t cmptparm;
    opj_image_t *image;
    OPJ_UINT32 x, y;

    memset(&cmptparm, 0, sizeof(cmptparm));
    cmptparm.dx = 1;
    cmptparm.dy = 1;
    cmptparm.w = IMAGE_W;
    cmptparm.h = IMAGE_H;
    cmptparm.x0 = 0;
    cmptparm.y0 = 0;
    cmptparm.prec = 8;
    cmptparm.sgnd = 0;

    image = opj_image_create(1, &cmptparm, OPJ_CLRSPC_GRAY);
    if (!image) {
        return NULL;
    }
    image->x0 = 0;
    image->y0 = 0;
    image->x1 = IMAGE_W;
    image->y1 = IMAGE_H;

    for (y = 0; y < IMAGE_H; ++y) {
        for (x = 0; x < IMAGE_W; ++x) {
            image->comps[0].data[y * IMAGE_W + x] = (OPJ_INT32)((x * 3 + y * 5) & 0xff);
        }
    }
    return image;
}

/* -------------------------------------------------------------------------- */
/* Encode the image as a raw J2K codestream with one tile-part per resolution */
/* (tile-parts of a given tile are contiguous), returning a malloc'ed buffer. */
/* -------------------------------------------------------------------------- */
static unsigned char *encode_tiled(OPJ_SIZE_T *out_len)
{
    opj_cparameters_t parameters;
    opj_image_t *image;
    opj_codec_t *codec;
    opj_stream_t *stream;
    const char *tmpfile = "test_tile_part_interleaved_tmp.j2k";
    unsigned char *buffer = NULL;
    FILE *f;
    long sz;

    image = create_test_image();
    if (!image) {
        return NULL;
    }

    opj_set_default_encoder_parameters(&parameters);
    parameters.tcp_numlayers = 1;
    parameters.cp_disto_alloc = 1;
    parameters.numresolution = NUM_RES;
    parameters.tile_size_on = OPJ_TRUE;
    parameters.cp_tx0 = 0;
    parameters.cp_ty0 = 0;
    parameters.cp_tdx = TILE_W;
    parameters.cp_tdy = TILE_H;
    parameters.tp_on = 1;
    parameters.tp_flag = 'R'; /* one tile-part per resolution level */

    codec = opj_create_compress(OPJ_CODEC_J2K);
    if (!codec) {
        opj_image_destroy(image);
        return NULL;
    }
    opj_set_info_handler(codec, quiet_callback, NULL);
    opj_set_warning_handler(codec, quiet_callback, NULL);
    opj_set_error_handler(codec, quiet_callback, NULL);

    if (!opj_setup_encoder(codec, &parameters, image)) {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return NULL;
    }

    stream = opj_stream_create_default_file_stream(tmpfile, OPJ_FALSE);
    if (!stream) {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return NULL;
    }

    if (!opj_start_compress(codec, image, stream) ||
            !opj_encode(codec, stream) ||
            !opj_end_compress(codec, stream)) {
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return NULL;
    }

    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    opj_image_destroy(image);

    /* slurp the file back */
    f = fopen(tmpfile, "rb");
    if (!f) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz > 0) {
        buffer = (unsigned char *)malloc((size_t)sz);
        if (buffer && fread(buffer, 1, (size_t)sz, f) != (size_t)sz) {
            free(buffer);
            buffer = NULL;
        }
    }
    fclose(f);
    remove(tmpfile);

    if (buffer) {
        *out_len = (OPJ_SIZE_T)sz;
    }
    return buffer;
}

/* -------------------------------------------------------------------------- */
/* Rewrite the codestream so that tile-parts are interleaved across tiles and */
/* TNsot is only declared in each tile's last tile-part. This reproduces the  */
/* layout that triggered issue #1558. The buffer is rewritten in place; its   */
/* total size is unchanged (only byte order and TNsot bytes change).          */
/* -------------------------------------------------------------------------- */
#define MAX_TILES 64
#define MAX_PARTS 64

static int interleave_tile_parts(unsigned char *buf, OPJ_SIZE_T len)
{
    OPJ_SIZE_T main_header_len;
    OPJ_SIZE_T pos;
    OPJ_SIZE_T eoc_pos;
    /* tile-part records, in stream order */
    OPJ_SIZE_T tp_off[MAX_TILES * MAX_PARTS];
    OPJ_UINT32 tp_len[MAX_TILES * MAX_PARTS];
    OPJ_UINT32 tp_isot[MAX_TILES * MAX_PARTS];
    OPJ_UINT32 tp_tpsot[MAX_TILES * MAX_PARTS];
    OPJ_UINT32 nb_tp = 0;
    OPJ_UINT32 nb_parts_per_tile[MAX_TILES];
    OPJ_UINT32 nb_tiles = 0;
    unsigned char *copy;
    OPJ_SIZE_T wpos;
    OPJ_UINT32 round, max_parts = 0;
    OPJ_UINT32 i, t;

    memset(nb_parts_per_tile, 0, sizeof(nb_parts_per_tile));

    /* main header runs up to the first SOT */
    for (pos = 0; pos + 1 < len; ++pos) {
        if (read_be16(buf + pos) == J2K_MS_SOT) {
            break;
        }
    }
    if (pos + 1 >= len) {
        fprintf(stderr, "could not find first SOT marker\n");
        return 1;
    }
    main_header_len = pos;

    /* walk the tile-parts using Psot to jump from one SOT to the next */
    while (pos + 12 <= len && read_be16(buf + pos) == J2K_MS_SOT) {
        OPJ_UINT32 psot = read_be32(buf + pos + 6);
        OPJ_UINT32 isot = read_be16(buf + pos + 4);
        OPJ_UINT32 tpsot = buf[pos + 10];
        if (psot == 0 || pos + psot > len) {
            fprintf(stderr, "unexpected Psot value\n");
            return 1;
        }
        if (nb_tp >= MAX_TILES * MAX_PARTS || isot >= MAX_TILES) {
            fprintf(stderr, "too many tile-parts/tiles for this test\n");
            return 1;
        }
        tp_off[nb_tp] = pos;
        tp_len[nb_tp] = psot;
        tp_isot[nb_tp] = isot;
        tp_tpsot[nb_tp] = tpsot;
        ++nb_tp;
        nb_parts_per_tile[isot]++;
        if (isot + 1 > nb_tiles) {
            nb_tiles = isot + 1;
        }
        pos += psot;
    }
    eoc_pos = pos;
    if (eoc_pos + 1 >= len || read_be16(buf + eoc_pos) != J2K_MS_EOC) {
        fprintf(stderr, "could not find trailing EOC marker\n");
        return 1;
    }

    if (nb_tiles < 2) {
        fprintf(stderr, "need at least 2 tiles to reproduce the bug\n");
        return 1;
    }
    for (t = 0; t < nb_tiles; ++t) {
        if (nb_parts_per_tile[t] < 2) {
            fprintf(stderr, "need at least 2 tile-parts per tile\n");
            return 1;
        }
        if (nb_parts_per_tile[t] > max_parts) {
            max_parts = nb_parts_per_tile[t];
        }
    }

    /* Clear TNsot in every tile-part except the one with the highest TPsot of
     * each tile (i.e. its last tile-part), where it must hold the total count. */
    for (i = 0; i < nb_tp; ++i) {
        OPJ_UINT32 isot = tp_isot[i];
        if (tp_tpsot[i] + 1 == nb_parts_per_tile[isot]) {
            buf[tp_off[i] + 11] = (unsigned char)nb_parts_per_tile[isot];
        } else {
            buf[tp_off[i] + 11] = 0;
        }
    }

    /* Reassemble: main header, then tile-parts in round-robin order
     * (all tiles' tile-part 0, then all tiles' tile-part 1, ...), then EOC.
     * Within a tile the tile-parts keep increasing TPsot order. */
    copy = (unsigned char *)malloc(len);
    if (!copy) {
        return 1;
    }
    memcpy(copy, buf, len);

    wpos = main_header_len;
    for (round = 0; round < max_parts; ++round) {
        for (i = 0; i < nb_tp; ++i) {
            if (tp_tpsot[i] == round) {
                memcpy(buf + wpos, copy + tp_off[i], tp_len[i]);
                wpos += tp_len[i];
            }
        }
    }
    /* EOC */
    buf[wpos++] = 0xff;
    buf[wpos++] = 0xd9;

    free(copy);

    if (wpos != eoc_pos + 2) {
        fprintf(stderr, "internal error: rewritten size mismatch\n");
        return 1;
    }
    return 0;
}

/* -------------------------------------------------------------------------- */
/* Decode helpers                                                             */
/* -------------------------------------------------------------------------- */
static opj_stream_t *create_memory_stream(unsigned char *buf, OPJ_SIZE_T len);

static opj_image_t *decode_whole(unsigned char *buf, OPJ_SIZE_T len)
{
    opj_dparameters_t parameters;
    opj_codec_t *codec;
    opj_stream_t *stream;
    opj_image_t *image = NULL;

    opj_set_default_decoder_parameters(&parameters);
    codec = opj_create_decompress(OPJ_CODEC_J2K);
    opj_set_info_handler(codec, quiet_callback, NULL);
    opj_set_warning_handler(codec, quiet_callback, NULL);
    opj_set_error_handler(codec, quiet_callback, NULL);
    if (!opj_setup_decoder(codec, &parameters)) {
        opj_destroy_codec(codec);
        return NULL;
    }
    stream = create_memory_stream(buf, len);
    if (!stream) {
        opj_destroy_codec(codec);
        return NULL;
    }
    if (!opj_read_header(stream, codec, &image) ||
            !opj_set_decode_area(codec, image, 0, 0, 0, 0) ||
            !opj_decode(codec, stream, image) ||
            !opj_end_decompress(codec, stream)) {
        opj_image_destroy(image);
        image = NULL;
    }
    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    return image;
}

/* in-memory stream backend */
typedef struct {
    unsigned char *data;
    OPJ_SIZE_T len;
    OPJ_SIZE_T pos;
} mem_stream_t;

static OPJ_SIZE_T mem_read(void *p_buffer, OPJ_SIZE_T p_nb_bytes, void *p_user)
{
    mem_stream_t *m = (mem_stream_t *)p_user;
    OPJ_SIZE_T avail = m->len - m->pos;
    OPJ_SIZE_T n = p_nb_bytes < avail ? p_nb_bytes : avail;
    if (n == 0) {
        return (OPJ_SIZE_T) - 1;
    }
    memcpy(p_buffer, m->data + m->pos, n);
    m->pos += n;
    return n;
}

static OPJ_OFF_T mem_skip(OPJ_OFF_T p_nb_bytes, void *p_user)
{
    mem_stream_t *m = (mem_stream_t *)p_user;
    m->pos += (OPJ_SIZE_T)p_nb_bytes;
    if (m->pos > m->len) {
        m->pos = m->len;
    }
    return p_nb_bytes;
}

static OPJ_BOOL mem_seek(OPJ_OFF_T p_nb_bytes, void *p_user)
{
    mem_stream_t *m = (mem_stream_t *)p_user;
    if ((OPJ_SIZE_T)p_nb_bytes > m->len) {
        return OPJ_FALSE;
    }
    m->pos = (OPJ_SIZE_T)p_nb_bytes;
    return OPJ_TRUE;
}

static opj_stream_t *create_memory_stream(unsigned char *buf, OPJ_SIZE_T len)
{
    opj_stream_t *stream;
    mem_stream_t *m = (mem_stream_t *)malloc(sizeof(mem_stream_t));
    if (!m) {
        return NULL;
    }
    m->data = buf;
    m->len = len;
    m->pos = 0;
    stream = opj_stream_default_create(OPJ_TRUE); /* read stream */
    if (!stream) {
        free(m);
        return NULL;
    }
    opj_stream_set_user_data(stream, m, free);
    opj_stream_set_user_data_length(stream, len);
    opj_stream_set_read_function(stream, mem_read);
    opj_stream_set_skip_function(stream, mem_skip);
    opj_stream_set_seek_function(stream, mem_seek);
    return stream;
}

/* -------------------------------------------------------------------------- */
int main(void)
{
    unsigned char *cs = NULL;
    OPJ_SIZE_T cs_len = 0;
    opj_image_t *full = NULL;
    OPJ_UINT32 tw, th, total_tiles, t;
    int ret = EXIT_FAILURE;

    cs = encode_tiled(&cs_len);
    if (!cs) {
        fprintf(stderr, "ERROR: failed to encode test codestream\n");
        return EXIT_FAILURE;
    }

    if (interleave_tile_parts(cs, cs_len) != 0) {
        fprintf(stderr, "ERROR: failed to build interleaved codestream\n");
        free(cs);
        return EXIT_FAILURE;
    }

    /* reference: full image decode */
    full = decode_whole(cs, cs_len);
    if (!full) {
        fprintf(stderr, "ERROR: failed to decode interleaved codestream as a whole\n");
        free(cs);
        return EXIT_FAILURE;
    }

    /* number of tiles is derived from the image / tile geometry */
    tw = (IMAGE_W + TILE_W - 1) / TILE_W;
    th = (IMAGE_H + TILE_H - 1) / TILE_H;
    total_tiles = tw * th;
    printf("Interleaved codestream: %u x %u tiles\n", tw, th);

    /* Decode every tile in turn while reusing a single codec and stream, as
     * applications (and j2k_random_tile_access) do. The bug fixed for issue
     * #1558 only shows up when the partially-built tile index from a previous
     * opj_get_decoded_tile() call is still present for the next call. */
    {
        opj_dparameters_t parameters;
        opj_codec_t *codec;
        opj_stream_t *stream;
        opj_image_t *image = NULL;

        opj_set_default_decoder_parameters(&parameters);
        codec = opj_create_decompress(OPJ_CODEC_J2K);
        opj_set_info_handler(codec, quiet_callback, NULL);
        opj_set_warning_handler(codec, quiet_callback, NULL);
        opj_set_error_handler(codec, quiet_callback, NULL);
        if (!opj_setup_decoder(codec, &parameters)) {
            opj_destroy_codec(codec);
            goto cleanup;
        }
        stream = create_memory_stream(cs, cs_len);
        if (!stream) {
            opj_destroy_codec(codec);
            goto cleanup;
        }
        if (!opj_read_header(stream, codec, &image)) {
            fprintf(stderr, "ERROR: failed to read header\n");
            opj_stream_destroy(stream);
            opj_destroy_codec(codec);
            goto cleanup;
        }

        for (t = 0; t < total_tiles; ++t) {
            OPJ_UINT32 ox, oy, x, y;
            int mismatch = 0;

            if (!opj_get_decoded_tile(codec, stream, image, t)) {
                fprintf(stderr, "ERROR: failed to decode tile %u (issue #1558)\n", t);
                opj_image_destroy(image);
                opj_stream_destroy(stream);
                opj_destroy_codec(codec);
                goto cleanup;
            }

            /* compare the tile samples against the full-image decode */
            ox = (OPJ_UINT32)image->comps[0].x0 - (OPJ_UINT32)full->comps[0].x0;
            oy = (OPJ_UINT32)image->comps[0].y0 - (OPJ_UINT32)full->comps[0].y0;
            for (y = 0; y < image->comps[0].h && !mismatch; ++y) {
                for (x = 0; x < image->comps[0].w; ++x) {
                    OPJ_INT32 tv = image->comps[0].data[y * image->comps[0].w + x];
                    OPJ_INT32 fv = full->comps[0].data[(oy + y) * full->comps[0].w +
                                                       (ox + x)];
                    if (tv != fv) {
                        fprintf(stderr,
                                "ERROR: tile %u sample (%u,%u) mismatch: tile=%d full=%d\n",
                                t, x, y, tv, fv);
                        mismatch = 1;
                        break;
                    }
                }
            }

            if (mismatch) {
                opj_image_destroy(image);
                opj_stream_destroy(stream);
                opj_destroy_codec(codec);
                goto cleanup;
            }
            printf("Tile %u decoded and verified\n", t);
        }

        opj_image_destroy(image);
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
    }

    printf("All tiles decoded successfully\n");
    ret = EXIT_SUCCESS;

cleanup:
    opj_image_destroy(full);
    free(cs);
    return ret;
}
