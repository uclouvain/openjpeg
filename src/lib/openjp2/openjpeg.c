/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * Copyright (c) 2008, 2011-2012, Centre National d'Etudes Spatiales (CNES), FR
 * Copyright (c) 2012, CS Systemes d'Information, France
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

#ifdef _WIN32
#include <windows.h>
#endif /* _WIN32 */

#include "opj_includes.h"


/* ---------------------------------------------------------------------- */
/* Functions to set the message handlers */

OPJ_BOOL OPJ_CALLCONV opj_set_info_handler(opj_codec_t * p_codec,
        opj_msg_callback p_callback,
        void * p_user_data)
{
    opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
    if (! l_codec) {
        return OPJ_FALSE;
    }

    l_codec->m_event_mgr.info_handler = p_callback;
    l_codec->m_event_mgr.m_info_data = p_user_data;

    return OPJ_TRUE;
}

OPJ_BOOL OPJ_CALLCONV opj_set_warning_handler(opj_codec_t * p_codec,
        opj_msg_callback p_callback,
        void * p_user_data)
{
    opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
    if (! l_codec) {
        return OPJ_FALSE;
    }

    l_codec->m_event_mgr.warning_handler = p_callback;
    l_codec->m_event_mgr.m_warning_data = p_user_data;

    return OPJ_TRUE;
}

OPJ_BOOL OPJ_CALLCONV opj_set_error_handler(opj_codec_t * p_codec,
        opj_msg_callback p_callback,
        void * p_user_data)
{
    opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
    if (! l_codec) {
        return OPJ_FALSE;
    }

    l_codec->m_event_mgr.error_handler = p_callback;
    l_codec->m_event_mgr.m_error_data = p_user_data;

    return OPJ_TRUE;
}

/* ---------------------------------------------------------------------- */

static OPJ_SIZE_T opj_read_from_file(void * p_buffer, OPJ_SIZE_T p_nb_bytes,
                                     FILE * p_file)
{
    OPJ_SIZE_T l_nb_read = fread(p_buffer, 1, p_nb_bytes, p_file);
    return l_nb_read ? l_nb_read : (OPJ_SIZE_T) - 1;
}

static OPJ_UINT64 opj_get_data_length_from_file(FILE * p_file)
{
    OPJ_OFF_T file_length = 0;

    OPJ_FSEEK(p_file, 0, SEEK_END);
    file_length = (OPJ_OFF_T)OPJ_FTELL(p_file);
    OPJ_FSEEK(p_file, 0, SEEK_SET);

    return (OPJ_UINT64)file_length;
}

static OPJ_SIZE_T opj_write_from_file(void * p_buffer, OPJ_SIZE_T p_nb_bytes,
                                      FILE * p_file)
{
    return fwrite(p_buffer, 1, p_nb_bytes, p_file);
}

static OPJ_OFF_T opj_skip_from_file(OPJ_OFF_T p_nb_bytes, FILE * p_user_data)
{
    if (OPJ_FSEEK(p_user_data, p_nb_bytes, SEEK_CUR)) {
        return -1;
    }

    return p_nb_bytes;
}

static OPJ_BOOL opj_seek_from_file(OPJ_OFF_T p_nb_bytes, FILE * p_user_data)
{
    if (OPJ_FSEEK(p_user_data, p_nb_bytes, SEEK_SET)) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

/* ---------------------------------------------------------------------- */
#ifdef _WIN32
#ifndef OPJ_STATIC
BOOL APIENTRY
DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

    OPJ_ARG_NOT_USED(lpReserved);
    OPJ_ARG_NOT_USED(hModule);

    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH :
        break;
    case DLL_PROCESS_DETACH :
        break;
    case DLL_THREAD_ATTACH :
    case DLL_THREAD_DETACH :
        break;
    }

    return TRUE;
}
#endif /* OPJ_STATIC */
#endif /* _WIN32 */

/* ---------------------------------------------------------------------- */

const char* OPJ_CALLCONV opj_version(void)
{
    return OPJ_PACKAGE_VERSION;
}

/* ---------------------------------------------------------------------- */
/* DECOMPRESSION FUNCTIONS*/

opj_codec_t* OPJ_CALLCONV opj_create_decompress(OPJ_CODEC_FORMAT p_format)
{
    opj_codec_private_t *l_codec = 00;

    l_codec = (opj_codec_private_t*) opj_calloc(1, sizeof(opj_codec_private_t));
    if (!l_codec) {
        return 00;
    }

    l_codec->is_decompressor = 1;

    switch (p_format) {
    case OPJ_CODEC_J2K:
        l_codec->opj_dump_codec = (void (*)(void*, OPJ_INT32, FILE*)) j2k_dump;

        l_codec->opj_get_codec_info = (opj_codestream_info_v2_t* (*)(
                                           void*)) j2k_get_cstr_info;

        l_codec->opj_get_codec_index = (opj_codestream_index_t* (*)(
                                            void*)) j2k_get_cstr_index;

        l_codec->m_codec_data.m_decompression.opj_decode =
            (OPJ_BOOL(*)(void *,
                         struct opj_stream_private *,
                         opj_image_t*, struct opj_event_mgr *)) opj_j2k_decode;

        l_codec->m_codec_data.m_decompression.opj_end_decompress =
            (OPJ_BOOL(*)(void *,
                         struct opj_stream_private *,
                         struct opj_event_mgr *)) opj_j2k_end_decompress;

        l_codec->m_codec_data.m_decompression.opj_read_header =
            (OPJ_BOOL(*)(struct opj_stream_private *,
                         void *,
                         opj_image_t **,
                         struct opj_event_mgr *)) opj_j2k_read_header;

        l_codec->m_codec_data.m_decompression.opj_destroy =
            (void (*)(void *))opj_j2k_destroy;

        l_codec->m_codec_data.m_decompression.opj_setup_decoder =
            (void (*)(void *, opj_dparameters_t *)) opj_j2k_setup_decoder;

        l_codec->m_codec_data.m_decompression.opj_read_tile_header =
            (OPJ_BOOL(*)(void *,
                         OPJ_UINT32*,
                         OPJ_UINT32*,
                         OPJ_INT32*, OPJ_INT32*,
                         OPJ_INT32*, OPJ_INT32*,
                         OPJ_UINT32*,
                         OPJ_BOOL*,
                         struct opj_stream_private *,
                         struct opj_event_mgr *)) opj_j2k_read_tile_header;

        l_codec->m_codec_data.m_decompression.opj_decode_tile_data =
            (OPJ_BOOL(*)(void *,
                         OPJ_UINT32,
                         OPJ_BYTE*,
                         OPJ_UINT32,
                         struct opj_stream_private *,
                         struct opj_event_mgr *)) opj_j2k_decode_tile;

        l_codec->m_codec_data.m_decompression.opj_set_decode_area =
            (OPJ_BOOL(*)(void *,
                         opj_image_t*,
                         OPJ_INT32, OPJ_INT32, OPJ_INT32, OPJ_INT32,
                         struct opj_event_mgr *)) opj_j2k_set_decode_area;

        l_codec->m_codec_data.m_decompression.opj_get_decoded_tile =
            (OPJ_BOOL(*)(void *p_codec,
                         opj_stream_private_t *p_cio,
                         opj_image_t *p_image,
                         struct opj_event_mgr * p_manager,
                         OPJ_UINT32 tile_index)) opj_j2k_get_tile;

        l_codec->m_codec_data.m_decompression.opj_set_decoded_resolution_factor =
            (OPJ_BOOL(*)(void * p_codec,
                         OPJ_UINT32 res_factor,
                         struct opj_event_mgr * p_manager)) opj_j2k_set_decoded_resolution_factor;

        l_codec->m_codec_data.m_decompression.opj_set_decoded_components =
            (OPJ_BOOL(*)(void * p_codec,
                         OPJ_UINT32 numcomps,
                         const OPJ_UINT32 * comps_indices,
                         struct opj_event_mgr * p_manager)) opj_j2k_set_decoded_components;

        l_codec->opj_set_threads =
            (OPJ_BOOL(*)(void * p_codec, OPJ_UINT32 num_threads)) opj_j2k_set_threads;

        l_codec->m_codec = opj_j2k_create_decompress();

        if (! l_codec->m_codec) {
            opj_free(l_codec);
            return NULL;
        }

        break;

    case OPJ_CODEC_JP2:
        /* get a JP2 decoder handle */
        l_codec->opj_dump_codec = (void (*)(void*, OPJ_INT32, FILE*)) jp2_dump;

        l_codec->opj_get_codec_info = (opj_codestream_info_v2_t* (*)(
                                           void*)) jp2_get_cstr_info;

        l_codec->opj_get_codec_index = (opj_codestream_index_t* (*)(
                                            void*)) jp2_get_cstr_index;

        l_codec->m_codec_data.m_decompression.opj_decode =
            (OPJ_BOOL(*)(void *,
                         struct opj_stream_private *,
                         opj_image_t*,
                         struct opj_event_mgr *)) opj_jp2_decode;

        l_codec->m_codec_data.m_decompression.opj_end_decompress =
            (OPJ_BOOL(*)(void *,
                         struct opj_stream_private *,
                         struct opj_event_mgr *)) opj_jp2_end_decompress;

        l_codec->m_codec_data.m_decompression.opj_read_header =
            (OPJ_BOOL(*)(struct opj_stream_private *,
                         void *,
                         opj_image_t **,
                         struct opj_event_mgr *)) opj_jp2_read_header;

        l_codec->m_codec_data.m_decompression.opj_read_tile_header =
            (OPJ_BOOL(*)(void *,
                         OPJ_UINT32*,
                         OPJ_UINT32*,
                         OPJ_INT32*,
                         OPJ_INT32*,
                         OPJ_INT32 *,
                         OPJ_INT32 *,
                         OPJ_UINT32 *,
                         OPJ_BOOL *,
                         struct opj_stream_private *,
                         struct opj_event_mgr *)) opj_jp2_read_tile_header;

        l_codec->m_codec_data.m_decompression.opj_decode_tile_data =
            (OPJ_BOOL(*)(void *,
                         OPJ_UINT32, OPJ_BYTE*, OPJ_UINT32,
                         struct opj_stream_private *,
                         struct opj_event_mgr *)) opj_jp2_decode_tile;

        l_codec->m_codec_data.m_decompression.opj_destroy = (void (*)(
                    void *))opj_jp2_destroy;

        l_codec->m_codec_data.m_decompression.opj_setup_decoder =
            (void (*)(void *, opj_dparameters_t *)) opj_jp2_setup_decoder;

        l_codec->m_codec_data.m_decompression.opj_set_decode_area =
            (OPJ_BOOL(*)(void *,
                         opj_image_t*,
                         OPJ_INT32, OPJ_INT32, OPJ_INT32, OPJ_INT32,
                         struct opj_event_mgr *)) opj_jp2_set_decode_area;

        l_codec->m_codec_data.m_decompression.opj_get_decoded_tile =
            (OPJ_BOOL(*)(void *p_codec,
                         opj_stream_private_t *p_cio,
                         opj_image_t *p_image,
                         struct opj_event_mgr * p_manager,
                         OPJ_UINT32 tile_index)) opj_jp2_get_tile;

        l_codec->m_codec_data.m_decompression.opj_set_decoded_resolution_factor =
            (OPJ_BOOL(*)(void * p_codec,
                         OPJ_UINT32 res_factor,
                         opj_event_mgr_t * p_manager)) opj_jp2_set_decoded_resolution_factor;

        l_codec->m_codec_data.m_decompression.opj_set_decoded_components =
            (OPJ_BOOL(*)(void * p_codec,
                         OPJ_UINT32 numcomps,
                         const OPJ_UINT32 * comps_indices,
                         struct opj_event_mgr * p_manager)) opj_jp2_set_decoded_components;

        l_codec->opj_set_threads =
            (OPJ_BOOL(*)(void * p_codec, OPJ_UINT32 num_threads)) opj_jp2_set_threads;

        l_codec->m_codec = opj_jp2_create(OPJ_TRUE);

        if (! l_codec->m_codec) {
            opj_free(l_codec);
            return 00;
        }

        break;
    case OPJ_CODEC_UNKNOWN:
    case OPJ_CODEC_JPT:
    default:
        opj_free(l_codec);
        return 00;
    }

    opj_set_default_event_handler(&(l_codec->m_event_mgr));
    return (opj_codec_t*) l_codec;
}

void OPJ_CALLCONV opj_set_default_decoder_parameters(opj_dparameters_t
        *parameters)
{
    if (parameters) {
        memset(parameters, 0, sizeof(opj_dparameters_t));
        /* default decoding parameters */
        parameters->cp_layer = 0;
        parameters->cp_reduce = 0;

        parameters->decod_format = -1;
        parameters->cod_format = -1;
        parameters->flags = 0;
        /* UniPG>> */
#ifdef USE_JPWL
        parameters->jpwl_correct = OPJ_FALSE;
        parameters->jpwl_exp_comps = JPWL_EXPECTED_COMPONENTS;
        parameters->jpwl_max_tiles = JPWL_MAXIMUM_TILES;
#endif /* USE_JPWL */
        /* <<UniPG */
    }
}


OPJ_BOOL OPJ_CALLCONV opj_codec_set_threads(opj_codec_t *p_codec,
        int num_threads)
{
    if (p_codec && (num_threads >= 0)) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;

        return l_codec->opj_set_threads(l_codec->m_codec, (OPJ_UINT32)num_threads);
    }
    return OPJ_FALSE;
}

OPJ_BOOL OPJ_CALLCONV opj_setup_decoder(opj_codec_t *p_codec,
                                        opj_dparameters_t *parameters
                                       )
{
    if (p_codec && parameters) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;

        if (! l_codec->is_decompressor) {
            opj_event_msg(&(l_codec->m_event_mgr), EVT_ERROR,
                          "Codec provided to the opj_setup_decoder function is not a decompressor handler.\n");
            return OPJ_FALSE;
        }

        l_codec->m_codec_data.m_decompression.opj_setup_decoder(l_codec->m_codec,
                parameters);
        return OPJ_TRUE;
    }
    return OPJ_FALSE;
}

OPJ_BOOL OPJ_CALLCONV opj_read_header(opj_stream_t *p_stream,
                                      opj_codec_t *p_codec,
                                      opj_image_t **p_image)
{
    if (p_codec && p_stream) {
        opj_codec_private_t* l_codec = (opj_codec_private_t*) p_codec;
        opj_stream_private_t* l_stream = (opj_stream_private_t*) p_stream;

        if (! l_codec->is_decompressor) {
            opj_event_msg(&(l_codec->m_event_mgr), EVT_ERROR,
                          "Codec provided to the opj_read_header function is not a decompressor handler.\n");
            return OPJ_FALSE;
        }

        return l_codec->m_codec_data.m_decompression.opj_read_header(l_stream,
                l_codec->m_codec,
                p_image,
                &(l_codec->m_event_mgr));
    }

    return OPJ_FALSE;
}


OPJ_BOOL OPJ_CALLCONV opj_set_decoded_components(opj_codec_t *p_codec,
        OPJ_UINT32 numcomps,
        const OPJ_UINT32* comps_indices,
        OPJ_BOOL apply_color_transforms)
{
    if (p_codec) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;

        if (! l_codec->is_decompressor) {
            opj_event_msg(&(l_codec->m_event_mgr), EVT_ERROR,
                          "Codec provided to the opj_set_decoded_components function is not a decompressor handler.\n");
            return OPJ_FALSE;
        }

        if (apply_color_transforms) {
            opj_event_msg(&(l_codec->m_event_mgr), EVT_ERROR,
                          "apply_color_transforms = OPJ_TRUE is not supported.\n");
            return OPJ_FALSE;
        }

        return  l_codec->m_codec_data.m_decompression.opj_set_decoded_components(
                    l_codec->m_codec,
                    numcomps,
                    comps_indices,
                    &(l_codec->m_event_mgr));
    }
    return OPJ_FALSE;
}

OPJ_BOOL OPJ_CALLCONV opj_decode(opj_codec_t *p_codec,
                                 opj_stream_t *p_stream,
                                 opj_image_t* p_image)
{
    if (p_codec && p_stream) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        if (! l_codec->is_decompressor) {
            return OPJ_FALSE;
        }

        return l_codec->m_codec_data.m_decompression.opj_decode(l_codec->m_codec,
                l_stream,
                p_image,
                &(l_codec->m_event_mgr));
    }

    return OPJ_FALSE;
}

OPJ_BOOL OPJ_CALLCONV opj_set_decode_area(opj_codec_t *p_codec,
        opj_image_t* p_image,
        OPJ_INT32 p_start_x, OPJ_INT32 p_start_y,
        OPJ_INT32 p_end_x, OPJ_INT32 p_end_y
                                         )
{
    if (p_codec) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;

        if (! l_codec->is_decompressor) {
            return OPJ_FALSE;
        }

        return  l_codec->m_codec_data.m_decompression.opj_set_decode_area(
                    l_codec->m_codec,
                    p_image,
                    p_start_x, p_start_y,
                    p_end_x, p_end_y,
                    &(l_codec->m_event_mgr));
    }
    return OPJ_FALSE;
}

OPJ_BOOL OPJ_CALLCONV opj_read_tile_header(opj_codec_t *p_codec,
        opj_stream_t * p_stream,
        OPJ_UINT32 * p_tile_index,
        OPJ_UINT32 * p_data_size,
        OPJ_INT32 * p_tile_x0, OPJ_INT32 * p_tile_y0,
        OPJ_INT32 * p_tile_x1, OPJ_INT32 * p_tile_y1,
        OPJ_UINT32 * p_nb_comps,
        OPJ_BOOL * p_should_go_on)
{
    if (p_codec && p_stream && p_data_size && p_tile_index) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        if (! l_codec->is_decompressor) {
            return OPJ_FALSE;
        }

        return l_codec->m_codec_data.m_decompression.opj_read_tile_header(
                   l_codec->m_codec,
                   p_tile_index,
                   p_data_size,
                   p_tile_x0, p_tile_y0,
                   p_tile_x1, p_tile_y1,
                   p_nb_comps,
                   p_should_go_on,
                   l_stream,
                   &(l_codec->m_event_mgr));
    }
    return OPJ_FALSE;
}

OPJ_BOOL OPJ_CALLCONV opj_decode_tile_data(opj_codec_t *p_codec,
        OPJ_UINT32 p_tile_index,
        OPJ_BYTE * p_data,
        OPJ_UINT32 p_data_size,
        opj_stream_t *p_stream
                                          )
{
    if (p_codec && p_data && p_stream) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        if (! l_codec->is_decompressor) {
            return OPJ_FALSE;
        }

        return l_codec->m_codec_data.m_decompression.opj_decode_tile_data(
                   l_codec->m_codec,
                   p_tile_index,
                   p_data,
                   p_data_size,
                   l_stream,
                   &(l_codec->m_event_mgr));
    }
    return OPJ_FALSE;
}

OPJ_BOOL OPJ_CALLCONV opj_get_decoded_tile(opj_codec_t *p_codec,
        opj_stream_t *p_stream,
        opj_image_t *p_image,
        OPJ_UINT32 tile_index)
{
    if (p_codec && p_stream) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        if (! l_codec->is_decompressor) {
            return OPJ_FALSE;
        }

        return l_codec->m_codec_data.m_decompression.opj_get_decoded_tile(
                   l_codec->m_codec,
                   l_stream,
                   p_image,
                   &(l_codec->m_event_mgr),
                   tile_index);
    }

    return OPJ_FALSE;
}

OPJ_BOOL OPJ_CALLCONV opj_set_decoded_resolution_factor(opj_codec_t *p_codec,
        OPJ_UINT32 res_factor)
{
    opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;

    if (!l_codec) {
        return OPJ_FALSE;
    }

    return l_codec->m_codec_data.m_decompression.opj_set_decoded_resolution_factor(
               l_codec->m_codec,
               res_factor,
               &(l_codec->m_event_mgr));
}

/* ---------------------------------------------------------------------- */
/* COMPRESSION FUNCTIONS*/

opj_codec_t* OPJ_CALLCONV opj_create_compress(OPJ_CODEC_FORMAT p_format)
{
    opj_codec_private_t *l_codec = 00;

    l_codec = (opj_codec_private_t*)opj_calloc(1, sizeof(opj_codec_private_t));
    if (!l_codec) {
        return 00;
    }

    l_codec->is_decompressor = 0;

    switch (p_format) {
    case OPJ_CODEC_J2K:
        l_codec->m_codec_data.m_compression.opj_encode = (OPJ_BOOL(*)(void *,
                struct opj_stream_private *,
                struct opj_event_mgr *)) opj_j2k_encode;

        l_codec->m_codec_data.m_compression.opj_end_compress = (OPJ_BOOL(*)(void *,
                struct opj_stream_private *,
                struct opj_event_mgr *)) opj_j2k_end_compress;

        l_codec->m_codec_data.m_compression.opj_start_compress = (OPJ_BOOL(*)(void *,
                struct opj_stream_private *,
                struct opj_image *,
                struct opj_event_mgr *)) opj_j2k_start_compress;

        l_codec->m_codec_data.m_compression.opj_write_tile = (OPJ_BOOL(*)(void *,
                OPJ_UINT32,
                OPJ_BYTE*,
                OPJ_UINT32,
                struct opj_stream_private *,
                struct opj_event_mgr *)) opj_j2k_write_tile;

        l_codec->m_codec_data.m_compression.opj_destroy = (void (*)(
                    void *)) opj_j2k_destroy;

        l_codec->m_codec_data.m_compression.opj_setup_encoder = (OPJ_BOOL(*)(void *,
                opj_cparameters_t *,
                struct opj_image *,
                struct opj_event_mgr *)) opj_j2k_setup_encoder;

        l_codec->m_codec = opj_j2k_create_compress();
        if (! l_codec->m_codec) {
            opj_free(l_codec);
            return 00;
        }

        break;

    case OPJ_CODEC_JP2:
        /* get a JP2 decoder handle */
        l_codec->m_codec_data.m_compression.opj_encode = (OPJ_BOOL(*)(void *,
                struct opj_stream_private *,
                struct opj_event_mgr *)) opj_jp2_encode;

        l_codec->m_codec_data.m_compression.opj_end_compress = (OPJ_BOOL(*)(void *,
                struct opj_stream_private *,
                struct opj_event_mgr *)) opj_jp2_end_compress;

        l_codec->m_codec_data.m_compression.opj_start_compress = (OPJ_BOOL(*)(void *,
                struct opj_stream_private *,
                struct opj_image *,
                struct opj_event_mgr *))  opj_jp2_start_compress;

        l_codec->m_codec_data.m_compression.opj_write_tile = (OPJ_BOOL(*)(void *,
                OPJ_UINT32,
                OPJ_BYTE*,
                OPJ_UINT32,
                struct opj_stream_private *,
                struct opj_event_mgr *)) opj_jp2_write_tile;

        l_codec->m_codec_data.m_compression.opj_destroy = (void (*)(
                    void *)) opj_jp2_destroy;

        l_codec->m_codec_data.m_compression.opj_setup_encoder = (OPJ_BOOL(*)(void *,
                opj_cparameters_t *,
                struct opj_image *,
                struct opj_event_mgr *)) opj_jp2_setup_encoder;

        l_codec->m_codec = opj_jp2_create(OPJ_FALSE);
        if (! l_codec->m_codec) {
            opj_free(l_codec);
            return 00;
        }

        break;

    case OPJ_CODEC_UNKNOWN:
    case OPJ_CODEC_JPT:
    default:
        opj_free(l_codec);
        return 00;
    }

    opj_set_default_event_handler(&(l_codec->m_event_mgr));
    return (opj_codec_t*) l_codec;
}

void OPJ_CALLCONV opj_set_default_encoder_parameters(opj_cparameters_t
        *parameters)
{
    if (parameters) {
        memset(parameters, 0, sizeof(opj_cparameters_t));
        /* default coding parameters */
        parameters->cp_cinema = OPJ_OFF; /* DEPRECATED */
        parameters->rsiz = OPJ_PROFILE_NONE;
        parameters->max_comp_size = 0;
        parameters->numresolution = 6;
        parameters->cp_rsiz = OPJ_STD_RSIZ; /* DEPRECATED */
        parameters->cblockw_init = 64;
        parameters->cblockh_init = 64;
        parameters->prog_order = OPJ_LRCP;
        parameters->roi_compno = -1;        /* no ROI */
        parameters->subsampling_dx = 1;
        parameters->subsampling_dy = 1;
        parameters->tp_on = 0;
        parameters->decod_format = -1;
        parameters->cod_format = -1;
        parameters->tcp_rates[0] = 0;
        parameters->tcp_numlayers = 0;
        parameters->cp_disto_alloc = 0;
        parameters->cp_fixed_alloc = 0;
        parameters->cp_fixed_quality = 0;
        parameters->jpip_on = OPJ_FALSE;
        /* UniPG>> */
#ifdef USE_JPWL
        parameters->jpwl_epc_on = OPJ_FALSE;
        parameters->jpwl_hprot_MH = -1; /* -1 means unassigned */
        {
            int i;
            for (i = 0; i < JPWL_MAX_NO_TILESPECS; i++) {
                parameters->jpwl_hprot_TPH_tileno[i] = -1; /* unassigned */
                parameters->jpwl_hprot_TPH[i] = 0; /* absent */
            }
        };
        {
            int i;
            for (i = 0; i < JPWL_MAX_NO_PACKSPECS; i++) {
                parameters->jpwl_pprot_tileno[i] = -1; /* unassigned */
                parameters->jpwl_pprot_packno[i] = -1; /* unassigned */
                parameters->jpwl_pprot[i] = 0; /* absent */
            }
        };
        parameters->jpwl_sens_size = 0; /* 0 means no ESD */
        parameters->jpwl_sens_addr = 0; /* 0 means auto */
        parameters->jpwl_sens_range = 0; /* 0 means packet */
        parameters->jpwl_sens_MH = -1; /* -1 means unassigned */
        {
            int i;
            for (i = 0; i < JPWL_MAX_NO_TILESPECS; i++) {
                parameters->jpwl_sens_TPH_tileno[i] = -1; /* unassigned */
                parameters->jpwl_sens_TPH[i] = -1; /* absent */
            }
        };
#endif /* USE_JPWL */
        /* <<UniPG */
    }
}

OPJ_BOOL OPJ_CALLCONV opj_setup_encoder(opj_codec_t *p_codec,
                                        opj_cparameters_t *parameters,
                                        opj_image_t *p_image)
{
    if (p_codec && parameters && p_image) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;

        if (! l_codec->is_decompressor) {
            return l_codec->m_codec_data.m_compression.opj_setup_encoder(l_codec->m_codec,
                    parameters,
                    p_image,
                    &(l_codec->m_event_mgr));
        }
    }

    return OPJ_FALSE;
}

OPJ_BOOL OPJ_CALLCONV opj_start_compress(opj_codec_t *p_codec,
        opj_image_t * p_image,
        opj_stream_t *p_stream)
{
    if (p_codec && p_stream) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        if (! l_codec->is_decompressor) {
            return l_codec->m_codec_data.m_compression.opj_start_compress(l_codec->m_codec,
                    l_stream,
                    p_image,
                    &(l_codec->m_event_mgr));
        }
    }

    return OPJ_FALSE;
}

OPJ_BOOL OPJ_CALLCONV opj_encode(opj_codec_t *p_info, opj_stream_t *p_stream)
{
    if (p_info && p_stream) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_info;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        if (! l_codec->is_decompressor) {
            return l_codec->m_codec_data.m_compression.opj_encode(l_codec->m_codec,
                    l_stream,
                    &(l_codec->m_event_mgr));
        }
    }

    return OPJ_FALSE;

}

OPJ_BOOL OPJ_CALLCONV opj_end_compress(opj_codec_t *p_codec,
                                       opj_stream_t *p_stream)
{
    if (p_codec && p_stream) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        if (! l_codec->is_decompressor) {
            return l_codec->m_codec_data.m_compression.opj_end_compress(l_codec->m_codec,
                    l_stream,
                    &(l_codec->m_event_mgr));
        }
    }
    return OPJ_FALSE;

}

OPJ_BOOL OPJ_CALLCONV opj_end_decompress(opj_codec_t *p_codec,
        opj_stream_t *p_stream)
{
    if (p_codec && p_stream) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        if (! l_codec->is_decompressor) {
            return OPJ_FALSE;
        }

        return l_codec->m_codec_data.m_decompression.opj_end_decompress(
                   l_codec->m_codec,
                   l_stream,
                   &(l_codec->m_event_mgr));
    }

    return OPJ_FALSE;
}

OPJ_BOOL OPJ_CALLCONV opj_set_MCT(opj_cparameters_t *parameters,
                                  OPJ_FLOAT32 * pEncodingMatrix,
                                  OPJ_INT32 * p_dc_shift, OPJ_UINT32 pNbComp)
{
    OPJ_UINT32 l_matrix_size = pNbComp * pNbComp * (OPJ_UINT32)sizeof(OPJ_FLOAT32);
    OPJ_UINT32 l_dc_shift_size = pNbComp * (OPJ_UINT32)sizeof(OPJ_INT32);
    OPJ_UINT32 l_mct_total_size = l_matrix_size + l_dc_shift_size;

    /* add MCT capability */
    if (OPJ_IS_PART2(parameters->rsiz)) {
        parameters->rsiz |= OPJ_EXTENSION_MCT;
    } else {
        parameters->rsiz = ((OPJ_PROFILE_PART2) | (OPJ_EXTENSION_MCT));
    }
    parameters->irreversible = 1;

    /* use array based MCT */
    parameters->tcp_mct = 2;
    parameters->mct_data = opj_malloc(l_mct_total_size);
    if (! parameters->mct_data) {
        return OPJ_FALSE;
    }

    memcpy(parameters->mct_data, pEncodingMatrix, l_matrix_size);
    memcpy(((OPJ_BYTE *) parameters->mct_data) +  l_matrix_size, p_dc_shift,
           l_dc_shift_size);

    return OPJ_TRUE;
}

OPJ_BOOL OPJ_CALLCONV opj_write_tile(opj_codec_t *p_codec,
                                     OPJ_UINT32 p_tile_index,
                                     OPJ_BYTE * p_data,
                                     OPJ_UINT32 p_data_size,
                                     opj_stream_t *p_stream)
{
    if (p_codec && p_stream && p_data) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;
        opj_stream_private_t * l_stream = (opj_stream_private_t *) p_stream;

        if (l_codec->is_decompressor) {
            return OPJ_FALSE;
        }

        return l_codec->m_codec_data.m_compression.opj_write_tile(l_codec->m_codec,
                p_tile_index,
                p_data,
                p_data_size,
                l_stream,
                &(l_codec->m_event_mgr));
    }

    return OPJ_FALSE;
}

/* ---------------------------------------------------------------------- */

void OPJ_CALLCONV opj_destroy_codec(opj_codec_t *p_codec)
{
    if (p_codec) {
        opj_codec_private_t * l_codec = (opj_codec_private_t *) p_codec;

        if (l_codec->is_decompressor) {
            l_codec->m_codec_data.m_decompression.opj_destroy(l_codec->m_codec);
        } else {
            l_codec->m_codec_data.m_compression.opj_destroy(l_codec->m_codec);
        }

        l_codec->m_codec = 00;
        opj_free(l_codec);
    }
}

/* ---------------------------------------------------------------------- */

void OPJ_CALLCONV opj_dump_codec(opj_codec_t *p_codec,
                                 OPJ_INT32 info_flag,
                                 FILE* output_stream)
{
    if (p_codec) {
        opj_codec_private_t* l_codec = (opj_codec_private_t*) p_codec;

        l_codec->opj_dump_codec(l_codec->m_codec, info_flag, output_stream);
        return;
    }

    /* TODO return error */
    /* fprintf(stderr, "[ERROR] Input parameter of the dump_codec function are incorrect.\n"); */
    return;
}

opj_codestream_info_v2_t* OPJ_CALLCONV opj_get_cstr_info(opj_codec_t *p_codec)
{
    if (p_codec) {
        opj_codec_private_t* l_codec = (opj_codec_private_t*) p_codec;

        return l_codec->opj_get_codec_info(l_codec->m_codec);
    }

    return NULL;
}

void OPJ_CALLCONV opj_destroy_cstr_info(opj_codestream_info_v2_t **cstr_info)
{
    if (cstr_info) {

        if ((*cstr_info)->m_default_tile_info.tccp_info) {
            opj_free((*cstr_info)->m_default_tile_info.tccp_info);
        }

        if ((*cstr_info)->tile_info) {
            /* FIXME not used for the moment*/
        }

        opj_free((*cstr_info));
        (*cstr_info) = NULL;
    }
}

opj_codestream_index_t * OPJ_CALLCONV opj_get_cstr_index(opj_codec_t *p_codec)
{
    if (p_codec) {
        opj_codec_private_t* l_codec = (opj_codec_private_t*) p_codec;

        return l_codec->opj_get_codec_index(l_codec->m_codec);
    }

    return NULL;
}

void OPJ_CALLCONV opj_destroy_cstr_index(opj_codestream_index_t **p_cstr_index)
{
    if (*p_cstr_index) {
        j2k_destroy_cstr_index(*p_cstr_index);
        (*p_cstr_index) = NULL;
    }
}

opj_stream_t* OPJ_CALLCONV opj_stream_create_default_file_stream(
    const char *fname, OPJ_BOOL p_is_read_stream)
{
    return opj_stream_create_file_stream(fname, OPJ_J2K_STREAM_CHUNK_SIZE,
                                         p_is_read_stream);
}

opj_stream_t* OPJ_CALLCONV opj_stream_create_file_stream(
    const char *fname,
    OPJ_SIZE_T p_size,
    OPJ_BOOL p_is_read_stream)
{
    opj_stream_t* l_stream = 00;
    FILE *p_file;
    const char *mode;

    if (! fname) {
        return NULL;
    }

    if (p_is_read_stream) {
        mode = "rb";
    } else {
        mode = "wb";
    }

    p_file = fopen(fname, mode);

    if (! p_file) {
        return NULL;
    }

    l_stream = opj_stream_create(p_size, p_is_read_stream);
    if (! l_stream) {
        fclose(p_file);
        return NULL;
    }

    opj_stream_set_user_data(l_stream, p_file,
                             (opj_stream_free_user_data_fn) fclose);
    opj_stream_set_user_data_length(l_stream,
                                    opj_get_data_length_from_file(p_file));
    opj_stream_set_read_function(l_stream, (opj_stream_read_fn) opj_read_from_file);
    opj_stream_set_write_function(l_stream,
                                  (opj_stream_write_fn) opj_write_from_file);
    opj_stream_set_skip_function(l_stream, (opj_stream_skip_fn) opj_skip_from_file);
    opj_stream_set_seek_function(l_stream, (opj_stream_seek_fn) opj_seek_from_file);

    return l_stream;
}

/** CPB *******************************************************************/
/* Added for Omics Data Automation                                        */
/**************************************************************************/


opj_stream_t* OPJ_CALLCONV opj_stream_memory_create(OPJ_SIZE_T p_buffer_size,
        OPJ_BOOL l_is_input)
{
    opj_stream_private_t * l_stream = 00;
    mem_stream_t * m_stream = 00;
    l_stream = (opj_stream_private_t*) opj_calloc(1, sizeof(opj_stream_private_t));
    if (! l_stream) {
        return 00;
    }

    m_stream = (mem_stream_t*) opj_calloc(1, sizeof(mem_stream_t));
    if (! m_stream) {
        return 00;
    }
    if (! l_is_input) {  /* writing to user data (compression) */
       /* Allocate initial stream data holder as twice default size */
       m_stream->mem_data = opj_malloc(2 * p_buffer_size);
       m_stream->mem_cursize = 2 * p_buffer_size;
       /* Set resize function to reallocate mem_data size if write_fn */
       /*  atempts to add more than currently allocated size          */
       m_stream->mem_resize_fn = (mem_stream_resize_fn) mem_stream_resize;
    }
    m_stream->mem_curidx = 0; /* Initially at first elelment [0] */

    l_stream->m_user_data = (void *)m_stream;

/** CPB **/
/*
    if (OPJ_TRUE) {
       mem_stream_t * my_data;
       my_data = (mem_stream_t*) l_stream->m_user_data;
       fprintf(stderr, "opj_stream_memory_create: memory stream data, size idx: %d %d\n",my_data->mem_cursize, my_data->mem_curidx);
    }
*/
/** CPB debug END **/

    l_stream->m_buffer_size = p_buffer_size;
    l_stream->m_stored_data = (OPJ_BYTE *) opj_malloc(p_buffer_size);
    if (! l_stream->m_stored_data) {
        opj_free(l_stream);
        return 00;
    }

    l_stream->m_current_data = l_stream->m_stored_data;

    if (l_is_input) {
        l_stream->m_status |= OPJ_STREAM_STATUS_INPUT;
        l_stream->m_opj_skip = opj_stream_read_skip;
        l_stream->m_opj_seek = opj_stream_read_seek;
    } else {
        l_stream->m_status |= OPJ_STREAM_STATUS_OUTPUT;
        l_stream->m_opj_skip = opj_stream_write_skip;
        l_stream->m_opj_seek = opj_stream_write_seek;
    }

    l_stream->m_read_fn  = (opj_stream_read_fn)  mem_stream_read;
    l_stream->m_write_fn = (opj_stream_write_fn) mem_stream_write;
    l_stream->m_skip_fn  = (opj_stream_skip_fn)  mem_stream_skip;
    l_stream->m_seek_fn  = (opj_stream_seek_fn)  mem_stream_seek;
    l_stream->m_free_user_data_fn  = (opj_stream_free_user_data_fn) mem_stream_free;

    return (opj_stream_t *) l_stream;
}

opj_stream_t* OPJ_CALLCONV opj_stream_create_default_memory_stream(OPJ_BOOL l_is_input)
{
    return opj_stream_memory_create(OPJ_J2K_STREAM_CHUNK_SIZE, l_is_input);
}

/** CPB **/ 
void OPJ_CALLCONV opj_stream_mem_set_user_data(opj_stream_t* p_stream,
        void * p_data, OPJ_UINT64 size_in,
        opj_stream_free_user_data_fn p_function)
{
    mem_stream_t * m_stream = 00;
    opj_stream_private_t* l_stream = (opj_stream_private_t*) p_stream;
    if (!l_stream) {
        return;
    }

    m_stream = (mem_stream_t*) opj_calloc(1, sizeof(mem_stream_t));
    if (! m_stream) {
        return;
    }

    m_stream->mem_data = opj_malloc(size_in * sizeof(OPJ_BYTE));
    if (! m_stream->mem_data) {
        return;
    }

    memcpy((void*)m_stream->mem_data, p_data, size_in);
    m_stream->mem_curidx = 0;
    m_stream->mem_cursize = size_in;
    // should only need to read from mem_data
    m_stream->mem_resize_fn = NULL;

/* CPB: if called after default chunk of memory allocated */
    if (l_stream->m_user_data) p_function(l_stream->m_user_data);

    l_stream->m_user_data = (void*)m_stream;
    l_stream->m_user_data_length = size_in;
    if (p_function)
       l_stream->m_free_user_data_fn = p_function;
    else // Default
       l_stream->m_free_user_data_fn =
           (opj_stream_free_user_data_fn)mem_stream_free;
}

/** CPB END **/


opj_stream_t* OPJ_CALLCONV opj_stream_create_memory_stream(
    void *buffer,
    OPJ_SIZE_T p_size,
    OPJ_BOOL p_is_read_stream)
{
    opj_stream_t* l_stream = 00;

    if (! buffer) {
        return NULL;
    }

    if (p_size == 0) {} /* Memory buffer is empty? */
                        /* Use next call to allocate OPJ_J2K_STREAM_CHUNK_SIZE as default initial size? */
    l_stream = opj_stream_memory_create(OPJ_J2K_STREAM_CHUNK_SIZE, p_is_read_stream);
    if (! l_stream) {
        return NULL;
    }

    opj_stream_mem_set_user_data(l_stream, buffer, p_size,
                             (opj_stream_free_user_data_fn) mem_stream_free);
    opj_stream_set_read_function(l_stream, (opj_stream_read_fn) mem_stream_read);
    opj_stream_set_write_function(l_stream,
                                  (opj_stream_write_fn) mem_stream_write);
    opj_stream_set_skip_function(l_stream, (opj_stream_skip_fn) mem_stream_skip);
    opj_stream_set_seek_function(l_stream, (opj_stream_seek_fn) mem_stream_seek);

    return l_stream;
}

OPJ_BOOL mem_stream_resize(mem_stream_t * mem_stream)
{
    size_t cursize = mem_stream->mem_cursize;
    OPJ_BYTE *old_data = mem_stream->mem_data;

/* Allocate twice the amount of memory currently used */
    OPJ_BYTE *new_data = opj_malloc((cursize * 2) * sizeof(OPJ_BYTE));

    if (! new_data) {
       return OPJ_FALSE;  /* resize failed */
    }

    memcpy(new_data, old_data, cursize);
    mem_stream->mem_data = new_data;
    mem_stream->mem_cursize = cursize * 2;
    opj_free(old_data);

    return OPJ_TRUE;
}

OPJ_SIZE_T mem_stream_read(void * p_buffer, OPJ_SIZE_T p_nb_bytes,
                           void * p_user_data)
{
    size_t available;
    size_t l_nb_bytes_to_read;
    mem_stream_t *l_dest = 00;

    l_dest = (mem_stream_t *) p_user_data;

    available = l_dest->mem_cursize - l_dest->mem_curidx;
    if (! available) 
       return (OPJ_SIZE_T) -1;

    l_nb_bytes_to_read = (available < p_nb_bytes) ? available : p_nb_bytes;
    memcpy(p_buffer, &(l_dest->mem_data[l_dest->mem_curidx]), l_nb_bytes_to_read); 
    l_dest->mem_curidx += l_nb_bytes_to_read;
    
    return l_nb_bytes_to_read;
}

OPJ_SIZE_T mem_stream_write(void * p_buffer, OPJ_SIZE_T p_nb_bytes,
                            void * p_user_data)
{
    mem_stream_t *l_dest = 00;
    l_dest = (mem_stream_t *) p_user_data;

/* Ensure there is enough space to hold p_nb_bytes more bytes of data */
    while ((l_dest->mem_cursize - l_dest->mem_curidx) < p_nb_bytes) {
       if (!l_dest->mem_resize_fn(l_dest)) {
          return (OPJ_SIZE_T) -1;
       }
    }

   memcpy((void *) &(l_dest->mem_data[l_dest->mem_curidx]), 
                       p_buffer, p_nb_bytes); 
   l_dest->mem_curidx += p_nb_bytes;

   return (OPJ_SIZE_T) p_nb_bytes;
}

OPJ_OFF_T mem_stream_skip(OPJ_OFF_T p_nb_bytes, void * p_user_data)
{
/* Position user memory pointer to p_nb_bytes bytes from CURRENT */
    mem_stream_t *l_mem_stream = 00;
    l_mem_stream = (mem_stream_t *) p_user_data;

    if (p_nb_bytes >= 0) {
       OPJ_UINT64 new_pos = l_mem_stream->mem_curidx + (OPJ_UINT64) p_nb_bytes;
       if (new_pos > l_mem_stream->mem_cursize) {
          /* Not enough bytes to move to SEEK position */
          return (OPJ_OFF_T) -1;  
       }

       l_mem_stream->mem_curidx = new_pos;
    } 
    else { 
    /* p_nb_bytes is negative, ensure it's absolute value is less than curidx */
       OPJ_OFF_T new_pos = (OPJ_OFF_T)l_mem_stream->mem_curidx + p_nb_bytes;
       if (new_pos < 0) {
          /* Can't move past [0] */
          return (OPJ_OFF_T) -1;  
       }

       l_mem_stream->mem_curidx = (OPJ_UINT64)new_pos; 
    }      
    return p_nb_bytes;
}

OPJ_BOOL mem_stream_seek(OPJ_OFF_T p_position, void * p_user_data)
{
/* Position user memory pointer to p_position bytes from START */
/* Return OPJ_TRUE if successful to recreate negation of fseek() results */
    mem_stream_t *l_mem_stream = 00;
    l_mem_stream = (mem_stream_t *) p_user_data;

    if (p_position < 0) {
       /* Can't move to negative index position */
       return OPJ_FALSE;
    }
    if (l_mem_stream->mem_cursize < (OPJ_UINT64)p_position) {
       /* Not enough bytes to move to SKIP position */
       return OPJ_FALSE;
    }

    l_mem_stream->mem_curidx = (OPJ_UINT64)p_position; 
    return OPJ_TRUE; /* Success */
}

OPJ_BOOL mem_stream_free(void * p_user_data)
{
    mem_stream_t *l_mem_stream = 00;
    l_mem_stream = (mem_stream_t *) p_user_data;

    opj_free(l_mem_stream->mem_data);
    opj_free(l_mem_stream);

    return (l_mem_stream == NULL);
}

OPJ_BYTE * opj_mem_stream_copy(opj_stream_t *s, size_t* size_out)
{
   opj_stream_private_t* p = (opj_stream_private_t*) s;
   mem_stream_t* m = (mem_stream_t*) p->m_user_data;

   size_t image_bytes = m->mem_curidx;

   OPJ_BYTE *out_buff = (OPJ_BYTE *) opj_malloc(image_bytes * sizeof(OPJ_BYTE));
   if (! out_buff) {
     fprintf(stderr, "Fail to allocate %u bytes: image_copy_out\n", (unsigned int)image_bytes);
     return NULL;
   }
   memcpy(out_buff, (m->mem_data), image_bytes);

   *size_out = image_bytes;
   return out_buff;
}

/**************************************************************************/
/* End Additions for Omics Data Automation                                */
/** CPB END ***************************************************************/


void* OPJ_CALLCONV opj_image_data_alloc(OPJ_SIZE_T size)
{
    void* ret = opj_aligned_malloc(size);
    /* printf("opj_image_data_alloc %p\n", ret); */
    return ret;
}

void OPJ_CALLCONV opj_image_data_free(void* ptr)
{
    /* printf("opj_image_data_free %p\n", ptr); */
    opj_aligned_free(ptr);
}
