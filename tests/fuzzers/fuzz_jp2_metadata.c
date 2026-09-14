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
 * Fuzz harness for JP2 container box/metadata parsing via opj_read_header().
 *
 * Unlike the existing opj_decompress_fuzzer_JP2 harness which exercises the
 * full image decoding pipeline, this harness stops after opj_read_header().
 * That function parses the JP2 superbox structure (JP, FTYP, JP2H, COLR,
 * PCLR, CMAP, CDEF, RES, XML, UUID, UINF boxes) and initialises the embedded
 * colour-space metadata (XMP blobs, ICC profiles, palette/channel tables)
 * without decoding a single codestream tile.
 *
 * Reaching this code path requires only a well-formed outer box skeleton;
 * the codestream inside the jp2c box can be entirely absent or corrupt.
 * Adversarial box lengths (e.g. a box length that extends far past EOF) can
 * therefore trigger out-of-bounds reads before the codestream decoder is
 * ever invoked.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "openjpeg.h"

/* ---- in-memory stream callbacks ---------------------------------------- */

typedef struct {
    const uint8_t *data;
    size_t         length;
    size_t         pos;
} MemStream;

static OPJ_SIZE_T mem_read(void *buf, OPJ_SIZE_T nb, void *user)
{
    MemStream *ms = (MemStream *)user;
    if (ms->pos >= ms->length) {
        return (OPJ_SIZE_T)-1;
    }
    if (ms->pos + nb > ms->length) {
        nb = ms->length - ms->pos;
    }
    if (nb == 0) {
        return (OPJ_SIZE_T)-1;
    }
    memcpy(buf, ms->data + ms->pos, nb);
    ms->pos += nb;
    return nb;
}

static OPJ_OFF_T mem_skip(OPJ_OFF_T nb, void *user)
{
    MemStream *ms = (MemStream *)user;
    ms->pos += (size_t)nb;
    return nb;
}

static OPJ_BOOL mem_seek(OPJ_OFF_T pos, void *user)
{
    MemStream *ms = (MemStream *)user;
    ms->pos = (size_t)pos;
    return OPJ_TRUE;
}

/* ---- silent event handlers --------------------------------------------- */

static void silent_callback(const char *msg, void *data)
{
    (void)msg;
    (void)data;
}

/* ---- fuzzer entry point ------------------------------------------------ */

/* JP2 signature box: 12-byte box (length=0x0000000C, type='jP  ', magic) */
static const uint8_t jp2_box_jp[] = {0x6a, 0x50, 0x20, 0x20}; /* 'jP  ' */

int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len)
{
    /* Quick pre-filter: must look like a JP2 file (signature box at offset 4) */
    if (len < 4 + sizeof(jp2_box_jp)) {
        return 0;
    }
    if (memcmp(buf + 4, jp2_box_jp, sizeof(jp2_box_jp)) != 0) {
        return 0;
    }

    /* Create codec */
    opj_codec_t *codec = opj_create_decompress(OPJ_CODEC_JP2);
    if (!codec) {
        return 0;
    }

    /* Suppress all diagnostic output */
    opj_set_info_handler(codec,    silent_callback, NULL);
    opj_set_warning_handler(codec, silent_callback, NULL);
    opj_set_error_handler(codec,   silent_callback, NULL);

    /* Default decoder parameters */
    opj_dparameters_t params;
    opj_set_default_decoder_parameters(&params);
    opj_setup_decoder(codec, &params);

    /* Wire up the in-memory stream */
    opj_stream_t *stream = opj_stream_create(1024, OPJ_TRUE /* is_input */);
    if (!stream) {
        opj_destroy_codec(codec);
        return 0;
    }

    MemStream ms;
    ms.data   = buf;
    ms.length = len;
    ms.pos    = 0;

    opj_stream_set_user_data(stream, &ms, NULL);
    opj_stream_set_user_data_length(stream, len);
    opj_stream_set_read_function(stream, mem_read);
    opj_stream_set_skip_function(stream, mem_skip);
    opj_stream_set_seek_function(stream, mem_seek);

    /* Parse JP2 container boxes and header — no codestream decoding */
    opj_image_t *image = NULL;
    opj_read_header(stream, codec, &image);

    /* Cleanup */
    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    opj_image_destroy(image);

    return 0;
}
