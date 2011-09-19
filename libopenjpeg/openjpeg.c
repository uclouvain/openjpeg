/*
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
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

#include "opj_config.h"
#include "opj_includes.h"

typedef struct opj_decompression
{
	opj_bool (* opj_read_header) (
			struct opj_stream_private * cio,
			void * p_codec,
			opj_image_header_t ** image_header,
			opj_codestream_info_t ** cstr_info,
			struct opj_event_mgr * p_manager);
	opj_image_t* (* opj_decode) (void * p_codec, struct opj_stream_private *p_cio, struct opj_event_mgr * p_manager);
	opj_bool (*opj_read_tile_header)(
		void * p_codec,
		OPJ_UINT32 * p_tile_index,
		OPJ_UINT32* p_data_size,
		OPJ_INT32 * p_tile_x0,
		OPJ_INT32 * p_tile_y0,
		OPJ_INT32 * p_tile_x1,
		OPJ_INT32 * p_tile_y1,
		OPJ_UINT32 * p_nb_comps,
		opj_bool * p_should_go_on,
		struct opj_stream_private *p_cio,
		struct opj_event_mgr * p_manager);
		opj_bool (*opj_decode_tile_data)(void * p_codec,OPJ_UINT32 p_tile_index,OPJ_BYTE * p_data,OPJ_UINT32 p_data_size,struct opj_stream_private *p_cio,struct opj_event_mgr * p_manager);
	opj_bool (* opj_end_decompress) (void *p_codec,struct opj_stream_private *cio,struct opj_event_mgr * p_manager);
	void (* opj_destroy) (void * p_codec);
	void (*opj_setup_decoder) (void * p_codec, opj_dparameters_t * p_param);
	opj_bool (*opj_set_decode_area) (void * p_codec,OPJ_INT32 p_start_x,OPJ_INT32 p_end_x,OPJ_INT32 p_start_y,OPJ_INT32 p_end_y,struct opj_event_mgr * p_manager);


}opj_decompression_t;

typedef struct opj_compression
{
	opj_bool (* opj_start_compress) (void *p_codec,struct opj_stream_private *cio,struct opj_image * p_image,	struct opj_event_mgr * p_manager);
	opj_bool (* opj_encode) (void * p_codec, struct opj_stream_private *p_cio, struct opj_event_mgr * p_manager);
	opj_bool (* opj_write_tile) (void * p_codec,OPJ_UINT32 p_tile_index,OPJ_BYTE * p_data,OPJ_UINT32 p_data_size,struct opj_stream_private * p_cio,struct opj_event_mgr * p_manager);
	opj_bool (* opj_end_compress) (void * p_codec, struct opj_stream_private *p_cio, struct opj_event_mgr * p_manager);
	void (* opj_destroy) (void * p_codec);
	void (*opj_setup_encoder) (void * p_codec,opj_cparameters_t * p_param,struct opj_image * p_image, struct opj_event_mgr * p_manager);

}opj_compression_t;

typedef struct opj_codec_private
{
	union
	{		/* code-blocks informations */
	  opj_decompression_t m_decompression;
	  opj_compression_t m_compression;
    } m_codec_data;
	void * m_codec;
	opj_event_mgr_t* m_event_mgr;
	unsigned is_decompressor : 1;
}
opj_codec_private_t;

/**
 * Default callback function.
 * Do nothing.
 */
void opj_default_callback (const char *msg, void *client_data)
{
	//FIXME V2 -> V1 cf below
}

void set_default_event_handler(opj_event_mgr_t * p_manager)
{
	//FIXME V2 -> V1
	//p_manager->m_error_data = 00;
	//p_manager->m_warning_data = 00;
	//p_manager->m_info_data = 00;
	p_manager->error_handler = opj_default_callback;
	p_manager->info_handler = opj_default_callback;
	p_manager->warning_handler = opj_default_callback;
}

OPJ_UINT32 opj_read_from_file (void * p_buffer, OPJ_UINT32 p_nb_bytes, FILE * p_file)
{
	OPJ_UINT32 l_nb_read = fread(p_buffer,1,p_nb_bytes,p_file);
	return l_nb_read ? l_nb_read : -1;
}

OPJ_UINT32 opj_write_from_file (void * p_buffer, OPJ_UINT32 p_nb_bytes, FILE * p_file)
{
	return fwrite(p_buffer,1,p_nb_bytes,p_file);
}

OPJ_SIZE_T opj_skip_from_file (OPJ_SIZE_T p_nb_bytes, FILE * p_user_data)
{
	if
		(fseek(p_user_data,p_nb_bytes,SEEK_CUR))
	{
		return -1;
	}
	return p_nb_bytes;
}

opj_bool opj_seek_from_file (OPJ_SIZE_T p_nb_bytes, FILE * p_user_data)
{
	if
		(fseek(p_user_data,p_nb_bytes,SEEK_SET))
	{
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/* ---------------------------------------------------------------------- */
#ifdef _WIN32
#ifndef OPJ_STATIC
BOOL APIENTRY
DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {

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


const char* OPJ_CALLCONV opj_version(void) {
    return PACKAGE_VERSION;
}

opj_dinfo_t* OPJ_CALLCONV opj_create_decompress(OPJ_CODEC_FORMAT format) {
	opj_dinfo_t *dinfo = (opj_dinfo_t*)opj_calloc(1, sizeof(opj_dinfo_t));
	if(!dinfo) return NULL;
	dinfo->is_decompressor = OPJ_TRUE;
	switch(format) {
		case CODEC_J2K:
		case CODEC_JPT:
			/* get a J2K decoder handle */
			dinfo->j2k_handle = (void*)j2k_create_decompress((opj_common_ptr)dinfo);
			if(!dinfo->j2k_handle) {
				opj_free(dinfo);
				return NULL;
			}
			break;
		case CODEC_JP2:
			/* get a JP2 decoder handle */
			dinfo->jp2_handle = (void*)jp2_create_decompress((opj_common_ptr)dinfo);
			if(!dinfo->jp2_handle) {
				opj_free(dinfo);
				return NULL;
			}
			break;
		case CODEC_UNKNOWN:
		default:
			opj_free(dinfo);
			return NULL;
	}

	dinfo->codec_format = format;

	return dinfo;
}

opj_codec_t* OPJ_CALLCONV opj_create_decompress_v2(OPJ_CODEC_FORMAT p_format)
{
	opj_codec_private_t *l_info = 00;

	l_info = (opj_codec_private_t*) opj_calloc(1, sizeof(opj_codec_private_t));
	if (!l_info){
		return 00;
	}
	memset(l_info, 0, sizeof(opj_codec_private_t));

	l_info->is_decompressor = 1;

	switch (p_format) {
		case CODEC_J2K:
			l_info->m_codec_data.m_decompression.opj_decode = (opj_image_t* (*) (void *, struct opj_stream_private *, struct opj_event_mgr * ))j2k_decode; // TODO MSD
			l_info->m_codec_data.m_decompression.opj_end_decompress =  (opj_bool (*) (void *,struct opj_stream_private *,struct opj_event_mgr *))j2k_end_decompress;
			l_info->m_codec_data.m_decompression.opj_read_header =  (opj_bool (*) (
					struct opj_stream_private *,
					void *,
					opj_image_header_t **,
					opj_codestream_info_t**,
					struct opj_event_mgr * )) j2k_read_header;
			l_info->m_codec_data.m_decompression.opj_destroy = (void (*) (void *))j2k_destroy;
			l_info->m_codec_data.m_decompression.opj_setup_decoder = (void (*) (void * ,opj_dparameters_t * )) j2k_setup_decoder_v2;
			l_info->m_codec_data.m_decompression.opj_read_tile_header = (opj_bool (*) (
				void *,
				OPJ_UINT32*,
				OPJ_UINT32*,
				OPJ_INT32 * ,
				OPJ_INT32 * ,
				OPJ_INT32 * ,
				OPJ_INT32 * ,
				OPJ_UINT32 * ,
				opj_bool *,
				struct opj_stream_private *,
				struct opj_event_mgr * )) j2k_read_tile_header;
				l_info->m_codec_data.m_decompression.opj_decode_tile_data = (opj_bool (*) (void *,OPJ_UINT32,OPJ_BYTE*,OPJ_UINT32,struct opj_stream_private *,	struct opj_event_mgr * )) j2k_decode_tile;
			l_info->m_codec_data.m_decompression.opj_set_decode_area = (opj_bool (*) (void *,OPJ_INT32,OPJ_INT32,OPJ_INT32,OPJ_INT32, struct opj_event_mgr * )) j2k_set_decode_area;
			l_info->m_codec = j2k_create_decompress_v2();

			if (! l_info->m_codec) {
				opj_free(l_info);
				return NULL;
			}

			break;

		case CODEC_JP2:
			/* get a JP2 decoder handle */
			l_info->m_codec_data.m_decompression.opj_decode = (opj_image_t* (*) (void *, struct opj_stream_private *, struct opj_event_mgr * ))opj_jp2_decode; // TODO MSD
			l_info->m_codec_data.m_decompression.opj_end_decompress =  (opj_bool (*) (void *,struct opj_stream_private *,struct opj_event_mgr *)) jp2_end_decompress;
			l_info->m_codec_data.m_decompression.opj_read_header =  (opj_bool (*) (
					struct opj_stream_private *,
					void *,
					opj_image_header_t **,
					opj_codestream_info_t**,
					struct opj_event_mgr * )) jp2_read_header;

			l_info->m_codec_data.m_decompression.opj_read_tile_header = (
				opj_bool (*) (
					void *,
					OPJ_UINT32*,
					OPJ_UINT32*,
					OPJ_INT32*,
					OPJ_INT32*,
					OPJ_INT32 * ,
					OPJ_INT32 * ,
					OPJ_UINT32 * ,
					opj_bool *,
					struct opj_stream_private *,
					struct opj_event_mgr * )) jp2_read_tile_header;

			l_info->m_codec_data.m_decompression.opj_decode_tile_data = (opj_bool (*) (void *,OPJ_UINT32,OPJ_BYTE*,OPJ_UINT32,struct opj_stream_private *,	struct opj_event_mgr * )) opj_jp2_decode_tile;

			l_info->m_codec_data.m_decompression.opj_destroy = (void (*) (void *))jp2_destroy;
			l_info->m_codec_data.m_decompression.opj_setup_decoder = (void (*) (void * ,opj_dparameters_t * )) jp2_setup_decoder_v2;
			l_info->m_codec_data.m_decompression.opj_set_decode_area = (opj_bool (*) (void *,OPJ_INT32,OPJ_INT32,OPJ_INT32,OPJ_INT32, struct opj_event_mgr * )) jp2_set_decode_area;

			l_info->m_codec = jp2_create(OPJ_TRUE);

			if (! l_info->m_codec) {
				opj_free(l_info);
				return 00;
			}

			break;
		case CODEC_UNKNOWN:
		case CODEC_JPT:
		default:
			opj_free(l_info);
			return 00;
	}

	// FIXME set_default_event_handler(&(l_info->m_event_mgr));
	return (opj_codec_t*) l_info;
}


void OPJ_CALLCONV opj_destroy_decompress(opj_dinfo_t *dinfo) {
	if(dinfo) {
		/* destroy the codec */
		switch(dinfo->codec_format) {
			case CODEC_J2K:
			case CODEC_JPT:
				j2k_destroy_decompress((opj_j2k_t*)dinfo->j2k_handle);
				break;
			case CODEC_JP2:
				jp2_destroy_decompress((opj_jp2_t*)dinfo->jp2_handle);
				break;
			case CODEC_UNKNOWN:
			default:
				break;
		}
		/* destroy the decompressor */
		opj_free(dinfo);
	}
}

void OPJ_CALLCONV opj_set_default_decoder_parameters(opj_dparameters_t *parameters) {
	if(parameters) {
		memset(parameters, 0, sizeof(opj_dparameters_t));
		/* default decoding parameters */
		parameters->cp_layer = 0;
		parameters->cp_reduce = 0;
		parameters->cp_limit_decoding = NO_LIMITATION;

		parameters->decod_format = -1;
		parameters->cod_format = -1;
/* UniPG>> */
#ifdef USE_JPWL
		parameters->jpwl_correct = OPJ_FALSE;
		parameters->jpwl_exp_comps = JPWL_EXPECTED_COMPONENTS;
		parameters->jpwl_max_tiles = JPWL_MAXIMUM_TILES;
#endif /* USE_JPWL */
/* <<UniPG */
	}
}

void OPJ_CALLCONV opj_setup_decoder(opj_dinfo_t *dinfo, opj_dparameters_t *parameters) {
	if(dinfo && parameters) {
		switch(dinfo->codec_format) {
			case CODEC_J2K:
			case CODEC_JPT:
				j2k_setup_decoder((opj_j2k_t*)dinfo->j2k_handle, parameters);
				break;
			case CODEC_JP2:
				jp2_setup_decoder((opj_jp2_t*)dinfo->jp2_handle, parameters);
				break;
			case CODEC_UNKNOWN:
			default:
				break;
		}
	}
}

opj_bool OPJ_CALLCONV opj_setup_decoder_v2(opj_codec_t *p_info, opj_dparameters_t *parameters, opj_event_mgr_t* event_mgr)
{
	if (p_info && parameters) {
		opj_codec_private_t * l_info = (opj_codec_private_t *) p_info;

		if (! l_info->is_decompressor) {
			return OPJ_FALSE;
		}

		l_info->m_codec_data.m_decompression.opj_setup_decoder(l_info->m_codec,parameters);

		if (event_mgr == NULL)
		{
			l_info->m_event_mgr->error_handler = opj_default_callback ;
			l_info->m_event_mgr->warning_handler = opj_default_callback ;
			l_info->m_event_mgr->info_handler = opj_default_callback ;
			l_info->m_event_mgr->client_data = stderr;
		}
		else
			l_info->m_event_mgr = event_mgr;
		return OPJ_TRUE;
	}
	return OPJ_FALSE;
}

opj_image_t* OPJ_CALLCONV opj_decode(opj_dinfo_t *dinfo, opj_cio_t *cio) {
	return opj_decode_with_info(dinfo, cio, NULL);
}

opj_image_t* OPJ_CALLCONV opj_decode_with_info(opj_dinfo_t *dinfo, opj_cio_t *cio, opj_codestream_info_t *cstr_info) {
	if(dinfo && cio) {
		switch(dinfo->codec_format) {
			case CODEC_J2K:
				return j2k_decode((opj_j2k_t*)dinfo->j2k_handle, cio, cstr_info);
			case CODEC_JPT:
				return j2k_decode_jpt_stream((opj_j2k_t*)dinfo->j2k_handle, cio, cstr_info);
			case CODEC_JP2:
				return opj_jp2_decode((opj_jp2_t*)dinfo->jp2_handle, cio, cstr_info);
			case CODEC_UNKNOWN:
			default:
				break;
		}
	}
	return NULL;
}

opj_cinfo_t* OPJ_CALLCONV opj_create_compress(OPJ_CODEC_FORMAT format) {
	opj_cinfo_t *cinfo = (opj_cinfo_t*)opj_calloc(1, sizeof(opj_cinfo_t));
	if(!cinfo) return NULL;
	cinfo->is_decompressor = OPJ_FALSE;
	switch(format) {
		case CODEC_J2K:
			/* get a J2K coder handle */
			cinfo->j2k_handle = (void*)j2k_create_compress((opj_common_ptr)cinfo);
			if(!cinfo->j2k_handle) {
				opj_free(cinfo);
				return NULL;
			}
			break;
		case CODEC_JP2:
			/* get a JP2 coder handle */
			cinfo->jp2_handle = (void*)jp2_create_compress((opj_common_ptr)cinfo);
			if(!cinfo->jp2_handle) {
				opj_free(cinfo);
				return NULL;
			}
			break;
		case CODEC_JPT:
		case CODEC_UNKNOWN:
		default:
			opj_free(cinfo);
			return NULL;
	}

	cinfo->codec_format = format;

	return cinfo;
}

void OPJ_CALLCONV opj_destroy_compress(opj_cinfo_t *cinfo) {
	if(cinfo) {
		/* destroy the codec */
		switch(cinfo->codec_format) {
			case CODEC_J2K:
				j2k_destroy_compress((opj_j2k_t*)cinfo->j2k_handle);
				break;
			case CODEC_JP2:
				jp2_destroy_compress((opj_jp2_t*)cinfo->jp2_handle);
				break;
			case CODEC_JPT:
			case CODEC_UNKNOWN:
			default:
				break;
		}
		/* destroy the decompressor */
		opj_free(cinfo);
	}
}

void OPJ_CALLCONV opj_set_default_encoder_parameters(opj_cparameters_t *parameters) {
	if(parameters) {
		memset(parameters, 0, sizeof(opj_cparameters_t));
		/* default coding parameters */
		parameters->cp_cinema = OFF; 
		parameters->max_comp_size = 0;
		parameters->numresolution = 6;
		parameters->cp_rsiz = STD_RSIZ;
		parameters->cblockw_init = 64;
		parameters->cblockh_init = 64;
		parameters->prog_order = LRCP;
		parameters->roi_compno = -1;		/* no ROI */
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

/**
 * Helper function.
 * Sets the stream to be a file stream. The FILE must have been open previously.
 * @param		p_stream	the stream to modify
 * @param		p_file		handler to an already open file.
*/
opj_stream_t* OPJ_CALLCONV opj_stream_create_default_file_stream (FILE * p_file, opj_bool p_is_read_stream)
{
	return opj_stream_create_file_stream(p_file,J2K_STREAM_CHUNK_SIZE,p_is_read_stream);
}

opj_stream_t* OPJ_CALLCONV opj_stream_create_file_stream (FILE * p_file, OPJ_UINT32 p_size, opj_bool p_is_read_stream)
{
	opj_stream_t* l_stream = 00;

	if (! p_file) {
		return NULL;
	}

	l_stream = opj_stream_create(p_size,p_is_read_stream);
	if (! l_stream) {
		return NULL;
	}

	opj_stream_set_user_data(l_stream, p_file);
	opj_stream_set_read_function(l_stream, (opj_stream_read_fn) opj_read_from_file);
	opj_stream_set_write_function(l_stream, (opj_stream_write_fn) opj_write_from_file);
	opj_stream_set_skip_function(l_stream, (opj_stream_skip_fn) opj_skip_from_file);
	opj_stream_set_seek_function(l_stream, (opj_stream_seek_fn) opj_seek_from_file);

	return l_stream;
}

void OPJ_CALLCONV opj_setup_encoder(opj_cinfo_t *cinfo, opj_cparameters_t *parameters, opj_image_t *image) {
	if(cinfo && parameters && image) {
		switch(cinfo->codec_format) {
			case CODEC_J2K:
				j2k_setup_encoder((opj_j2k_t*)cinfo->j2k_handle, parameters, image);
				break;
			case CODEC_JP2:
				jp2_setup_encoder((opj_jp2_t*)cinfo->jp2_handle, parameters, image);
				break;
			case CODEC_JPT:
			case CODEC_UNKNOWN:
			default:
				break;
		}
	}
}

opj_bool OPJ_CALLCONV opj_encode(opj_cinfo_t *cinfo, opj_cio_t *cio, opj_image_t *image, char *index) {
	if (index != NULL)
		opj_event_msg((opj_common_ptr)cinfo, EVT_WARNING, "Set index to NULL when calling the opj_encode function.\n"
		"To extract the index, use the opj_encode_with_info() function.\n"
		"No index will be generated during this encoding\n");
	return opj_encode_with_info(cinfo, cio, image, NULL);
}

opj_bool OPJ_CALLCONV opj_encode_with_info(opj_cinfo_t *cinfo, opj_cio_t *cio, opj_image_t *image, opj_codestream_info_t *cstr_info) {
	if(cinfo && cio && image) {
		switch(cinfo->codec_format) {
			case CODEC_J2K:
				return j2k_encode((opj_j2k_t*)cinfo->j2k_handle, cio, image, cstr_info);
			case CODEC_JP2:
				return opj_jp2_encode((opj_jp2_t*)cinfo->jp2_handle, cio, image, cstr_info);	    
			case CODEC_JPT:
			case CODEC_UNKNOWN:
			default:
				break;
		}
	}
	return OPJ_FALSE;
}

void OPJ_CALLCONV opj_destroy_cstr_info(opj_codestream_info_t *cstr_info) {
	if (cstr_info) {
		int tileno;
		for (tileno = 0; tileno < cstr_info->tw * cstr_info->th; tileno++) {
			opj_tile_info_t *tile_info = &cstr_info->tile[tileno];
			opj_free(tile_info->thresh);
			opj_free(tile_info->packet);
			opj_free(tile_info->tp);
			opj_free(tile_info->marker);
		}
		opj_free(cstr_info->tile);
		opj_free(cstr_info->marker);
		opj_free(cstr_info->numdecompos);
	}
}

#ifdef OLD_WAY_MS
opj_bool OPJ_CALLCONV opj_read_header (
								   opj_codec_t *p_codec,
								   opj_image_t ** p_image,
								   OPJ_INT32 * p_tile_x0,
								   OPJ_INT32 * p_tile_y0,
								   OPJ_UINT32 * p_tile_width,
								   OPJ_UINT32 * p_tile_height,
								   OPJ_UINT32 * p_nb_tiles_x,
								   OPJ_UINT32 * p_nb_tiles_y,
								   opj_stream_t *p_cio)
{
	if (p_codec && p_cio) {
		opj_codec_private_t * l_info = (opj_codec_private_t *) p_codec;
		opj_stream_private_t * l_cio = (opj_stream_private_t *) p_cio;

		if(! l_info->is_decompressor) {
			return OPJ_FALSE;
		}

		return l_info->m_codec_data.m_decompression.opj_read_header(
			l_info->m_codec,
			p_image,
			p_tile_x0,
			p_tile_y0,
			p_tile_width,
			p_tile_height,
			p_nb_tiles_x,
			p_nb_tiles_y,
			l_cio,
			l_info->m_event_mgr); //&(l_info->m_event_mgr));
	}
	return OPJ_FALSE;
}
#endif

opj_bool OPJ_CALLCONV opj_read_header (	opj_stream_t *p_cio,
										opj_codec_t *p_codec,
										opj_image_header_t **p_image_header,
										opj_codestream_info_t **p_cstr_info	)

{
	if (p_codec && p_cio) {
		opj_codec_private_t* l_info = (opj_codec_private_t*) p_codec;
		opj_stream_private_t* l_cio = (opj_stream_private_t*) p_cio;

		if(! l_info->is_decompressor) {
			return OPJ_FALSE;
		}

		return l_info->m_codec_data.m_decompression.opj_read_header(
					l_cio,
					l_info->m_codec,
					p_image_header,
					p_cstr_info,
					l_info->m_event_mgr);
	}
	return OPJ_FALSE;
}


void OPJ_CALLCONV opj_destroy_codec(opj_codec_t *p_info)
{
	if
		(p_info)
	{
		opj_codec_private_t * l_info = (opj_codec_private_t *) p_info;
		if
			(l_info->is_decompressor)
		{
			l_info->m_codec_data.m_decompression.opj_destroy(l_info->m_codec);
		}
		else
		{
			l_info->m_codec_data.m_compression.opj_destroy(l_info->m_codec);
		}
		l_info->m_codec = 00;
		opj_free(l_info);
	}
}
