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


/**
 * Decompression handler.
 */
typedef struct opj_decompression
{
	/** Main header reading function handler*/
	opj_bool (* opj_read_header) (	struct opj_stream_private * cio,
									void * p_codec,
									opj_image_t *p_image,
									struct opj_event_mgr * p_manager);
	/** FIXME DOC */
	opj_bool (* opj_decode) (	void * p_codec,
								struct opj_stream_private *p_cio,
								opj_image_t *p_image,
								struct opj_event_mgr * p_manager);
	/** FIXME DOC */
	opj_bool (*opj_read_tile_header)(	void * p_codec,
										OPJ_UINT32 * p_tile_index,
										OPJ_UINT32* p_data_size,
										OPJ_INT32 * p_tile_x0, OPJ_INT32 * p_tile_y0,
										OPJ_INT32 * p_tile_x1, OPJ_INT32 * p_tile_y1,
										OPJ_UINT32 * p_nb_comps,
										opj_bool * p_should_go_on,
										struct opj_stream_private *p_cio,
										struct opj_event_mgr * p_manager);
	/** FIXME DOC */
	opj_bool (*opj_decode_tile_data)(	void * p_codec,
										OPJ_UINT32 p_tile_index,
										OPJ_BYTE * p_data,
										OPJ_UINT32 p_data_size,
										struct opj_stream_private *p_cio,
										struct opj_event_mgr * p_manager);
	/** FIXME DOC */
	opj_bool (* opj_end_decompress) (	void *p_codec,
										struct opj_stream_private *cio,
										struct opj_event_mgr * p_manager);
	/** Codec destroy function handler*/
	void (* opj_destroy) (void * p_codec);
	/** Setup decoder function handler */
	void (*opj_setup_decoder) (void * p_codec, opj_dparameters_t * p_param);
	/** Set decode area function handler */
	opj_bool (*opj_set_decode_area) (	void * p_codec,
										OPJ_INT32 p_start_x, OPJ_INT32 p_end_x,
										OPJ_INT32 p_start_y, OPJ_INT32 p_end_y,
										struct opj_event_mgr * p_manager);
}opj_decompression_t;

/**
 * Compression handler. FIXME DOC
 */
typedef struct opj_compression
{
	opj_bool (* opj_start_compress) (void *p_codec,struct opj_stream_private *cio,struct opj_image * p_image,	struct opj_event_mgr * p_manager);
	opj_bool (* opj_encode) (void * p_codec, struct opj_stream_private *p_cio, struct opj_event_mgr * p_manager);
	opj_bool (* opj_write_tile) (void * p_codec,OPJ_UINT32 p_tile_index,OPJ_BYTE * p_data,OPJ_UINT32 p_data_size,struct opj_stream_private * p_cio,struct opj_event_mgr * p_manager);
	opj_bool (* opj_end_compress) (void * p_codec, struct opj_stream_private *p_cio, struct opj_event_mgr * p_manager);
	void (* opj_destroy) (void * p_codec);
	void (*opj_setup_encoder) (void * p_codec,opj_cparameters_t * p_param,struct opj_image * p_image, struct opj_event_mgr * p_manager);

}opj_compression_t;

/**
 * Main codec handler used for compression or decompression.
 */
typedef struct opj_codec_private
{
	/** FIXME DOC */
	union {
		opj_decompression_t m_decompression;
		opj_compression_t m_compression;
    } m_codec_data;
    /** FIXME DOC*/
	void * m_codec;
	/** Event handler */
	opj_event_mgr_t* m_event_mgr;
	/** Flag to indicate if the codec is used to decode or encode*/
	opj_bool is_decompressor;
	opj_bool (*opj_dump_codec) (void * p_codec, OPJ_INT32 info_flag, FILE* output_stream);
	opj_codestream_info_v2_t* (*opj_get_codec_info)(void* p_codec);
	opj_codestream_index_t* (*opj_get_codec_index)(void* p_codec);
}
opj_codec_private_t;



/* ---------------------------------------------------------------------- */


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
	if (fseek(p_user_data,p_nb_bytes,SEEK_CUR)) {
		return -1;
	}

	return p_nb_bytes;
}

opj_bool opj_seek_from_file (OPJ_SIZE_T p_nb_bytes, FILE * p_user_data)
{
	if (fseek(p_user_data,p_nb_bytes,SEEK_SET)) {
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
			l_info->opj_dump_codec = (opj_bool (*) (void*, OPJ_INT32, FILE*)) j2k_dump;

			l_info->opj_get_codec_info = (opj_codestream_info_v2_t* (*) (void*) ) j2k_get_cstr_info;

			l_info->opj_get_codec_index = (opj_codestream_index_t* (*) (void*) ) j2k_get_cstr_index;

			l_info->m_codec_data.m_decompression.opj_decode =
					(opj_bool (*) (	void *,
									struct opj_stream_private *,
									opj_image_t*, struct opj_event_mgr * )) j2k_decode_v2;

			l_info->m_codec_data.m_decompression.opj_end_decompress =
					(opj_bool (*) (	void *,
									struct opj_stream_private *,
									struct opj_event_mgr *)) j2k_end_decompress;

			l_info->m_codec_data.m_decompression.opj_read_header =
					(opj_bool (*) (	struct opj_stream_private *,
									void *,
									opj_image_t *,
									struct opj_event_mgr * )) j2k_read_header;

			l_info->m_codec_data.m_decompression.opj_destroy =
					(void (*) (void *))j2k_destroy;

			l_info->m_codec_data.m_decompression.opj_setup_decoder =
					(void (*) (void * , opj_dparameters_t * )) j2k_setup_decoder_v2;

			l_info->m_codec_data.m_decompression.opj_read_tile_header =
					(opj_bool (*) (	void *,
									OPJ_UINT32*,
									OPJ_UINT32*,
									OPJ_INT32*, OPJ_INT32*,
									OPJ_INT32*, OPJ_INT32*,
									OPJ_UINT32*,
									opj_bool*,
									struct opj_stream_private *,
									struct opj_event_mgr * )) j2k_read_tile_header;

			l_info->m_codec_data.m_decompression.opj_decode_tile_data =
					(opj_bool (*) (void *, OPJ_UINT32, OPJ_BYTE*, OPJ_UINT32, struct opj_stream_private *, struct opj_event_mgr *)) j2k_decode_tile;

			l_info->m_codec_data.m_decompression.opj_set_decode_area =
					(opj_bool (*) (void *, OPJ_INT32, OPJ_INT32, OPJ_INT32, OPJ_INT32, struct opj_event_mgr *)) j2k_set_decode_area;

			l_info->m_codec = j2k_create_decompress_v2();

			if (! l_info->m_codec) {
				opj_free(l_info);
				return NULL;
			}

			break;

		case CODEC_JP2:
			/* get a JP2 decoder handle */
			l_info->m_codec_data.m_decompression.opj_decode =
					(opj_bool (*) (	void *,
									struct opj_stream_private *,
									opj_image_t*,
									struct opj_event_mgr * )) opj_jp2_decode_v2;

			l_info->m_codec_data.m_decompression.opj_end_decompress =  (opj_bool (*) (void *,struct opj_stream_private *,struct opj_event_mgr *)) jp2_end_decompress;

			l_info->m_codec_data.m_decompression.opj_read_header =  (opj_bool (*) (
					struct opj_stream_private *,
					void *,
					opj_image_t *,
					struct opj_event_mgr * )) jp2_read_header;

			l_info->m_codec_data.m_decompression.opj_read_tile_header = ( opj_bool (*) (
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
	opj_codec_private_t * l_info = (opj_codec_private_t *) p_info;

	if ( !p_info || !parameters || !event_mgr ){
		fprintf(stderr, "[ERROR] Input parameters of the setup_decoder function are incorrect.\n");
		return OPJ_FALSE;
	}

	if ( !event_mgr->error_handler || !event_mgr->warning_handler || !event_mgr->error_handler){
		fprintf(stderr, "[ERROR] Event handler provided to the setup_decoder function is not valid.\n");
		return OPJ_FALSE;
	}

	if (! l_info->is_decompressor) {
		opj_event_msg_v2(event_mgr, EVT_ERROR, "Codec provided to the setup_decoder function is not a decompressor handler.\n");
		return OPJ_FALSE;
	}

	l_info->m_codec_data.m_decompression.opj_setup_decoder(l_info->m_codec, parameters);

	l_info->m_event_mgr = event_mgr;

	return OPJ_TRUE;
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

void OPJ_CALLCONV opj_destroy_cstr_info_v2(opj_codestream_info_v2_t *cstr_info) {
	if (cstr_info) {
		int tileno, compno;

		if (cstr_info->tile_info){
			for (tileno = 0; tileno < cstr_info->tw * cstr_info->th; tileno++) {
				for (compno = 0; compno < cstr_info->nbcomps; compno++){
					opj_free(cstr_info->tile_info[tileno].tccp_info);
				}
			}
			opj_free(cstr_info->tile_info);
		}

		if (cstr_info->m_default_tile_info.tccp_info){
			opj_free(cstr_info->m_default_tile_info.tccp_info);
		}

		opj_free(cstr_info);
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
										opj_image_t *p_image )
{
	if (p_codec && p_cio) {
		opj_codec_private_t* l_info = (opj_codec_private_t*) p_codec;
		opj_stream_private_t* l_cio = (opj_stream_private_t*) p_cio;

		if(! l_info->is_decompressor) {
			opj_event_msg_v2(l_info->m_event_mgr, EVT_ERROR, "Codec provided to the read_header function is not a decompressor handler.\n");
			return OPJ_FALSE;
		}

		return l_info->m_codec_data.m_decompression.opj_read_header(
					l_cio,
					l_info->m_codec,
					p_image,
					l_info->m_event_mgr);
	}

	fprintf(stderr, "[ERROR] Input parameters of the read_header function are incorrect.\n");
	return OPJ_FALSE;
}


void OPJ_CALLCONV opj_destroy_codec(opj_codec_t *p_info)
{
	if (p_info) {
		opj_codec_private_t * l_info = (opj_codec_private_t *) p_info;

		if (l_info->is_decompressor) {
			l_info->m_codec_data.m_decompression.opj_destroy(l_info->m_codec);
		}
		else {
			l_info->m_codec_data.m_compression.opj_destroy(l_info->m_codec);
		}

		l_info->m_codec = 00;
		opj_free(l_info);
	}
}

/**
 * Sets the given area to be decoded. This function should be called right after opj_read_header and before any tile header reading.
 *
 * @param	p_codec			the jpeg2000 codec.
 * @param	p_start_x		the left position of the rectangle to decode (in image coordinates).
 * @param	p_end_x			the right position of the rectangle to decode (in image coordinates).
 * @param	p_start_y		the up position of the rectangle to decode (in image coordinates).
 * @param	p_end_y			the bottom position of the rectangle to decode (in image coordinates).
 *
 * @return	true			if the area could be set.
 */
opj_bool OPJ_CALLCONV opj_set_decode_area(	opj_codec_t *p_codec,
											OPJ_INT32 p_start_x, OPJ_INT32 p_start_y,
											OPJ_INT32 p_end_x, OPJ_INT32 p_end_y
											)
{
	if (p_codec) {
		opj_codec_private_t * l_info = (opj_codec_private_t *) p_codec;
		if (! l_info->is_decompressor) {
			return OPJ_FALSE;
		}

		return  l_info->m_codec_data.m_decompression.opj_set_decode_area(
				l_info->m_codec,
				p_start_x,
				p_start_y,
				p_end_x,
				p_end_y,
				l_info->m_event_mgr);

	}
	return OPJ_FALSE;
}

/**
 * Reads a tile header. This function is compulsory and allows one to know the size of the tile thta will be decoded.
 * The user may need to refer to the image got by opj_read_header to understand the size being taken by the tile.
 *
 * @param	p_codec			the jpeg2000 codec.
 * @param	p_tile_index	pointer to a value that will hold the index of the tile being decoded, in case of success.
 * @param	p_data_size		pointer to a value that will hold the maximum size of the decoded data, in case of success. In case
 *							of truncated codestreams, the actual number of bytes decoded may be lower. The computation of the size is the same
 *							as depicted in opj_write_tile.
 * @param	p_tile_x0		pointer to a value that will hold the x0 pos of the tile (in the image).
 * @param	p_tile_y0		pointer to a value that will hold the y0 pos of the tile (in the image).
 * @param	p_tile_x1		pointer to a value that will hold the x1 pos of the tile (in the image).
 * @param	p_tile_y1		pointer to a value that will hold the y1 pos of the tile (in the image).
 * @param	p_nb_comps		pointer to a value that will hold the number of components in the tile.
 * @param	p_should_go_on	pointer to a boolean that will hold the fact that the decoding should go on. In case the
 *							codestream is over at the time of the call, the value will be set to false. The user should then stop
 *							the decoding.
 * @param	p_stream		the stream to decode.
 * @return	true			if the tile header could be decoded. In case the decoding should end, the returned value is still true.
 *							returning false may be the result of a shortage of memory or an internal error.
 */
opj_bool OPJ_CALLCONV opj_read_tile_header(
					opj_codec_t *p_codec,
					opj_stream_t * p_stream,
					OPJ_UINT32 * p_tile_index,
					OPJ_UINT32 * p_data_size,
					OPJ_INT32 * p_tile_x0, OPJ_INT32 * p_tile_y0,
					OPJ_INT32 * p_tile_x1, OPJ_INT32 * p_tile_y1,
					OPJ_UINT32 * p_nb_comps,
					opj_bool * p_should_go_on)
{
	if (p_codec && p_stream && p_data_size && p_tile_index) {
		opj_codec_private_t * l_info = (opj_codec_private_t *) p_codec;
		opj_stream_private_t * l_cio = (opj_stream_private_t *) p_stream;

		if (! l_info->is_decompressor) {
			return OPJ_FALSE;
		}

		return l_info->m_codec_data.m_decompression.opj_read_tile_header(
			l_info->m_codec,
			p_tile_index,
			p_data_size,
			p_tile_x0, p_tile_y0,
			p_tile_x1, p_tile_y1,
			p_nb_comps,
			p_should_go_on,
			l_cio,
			l_info->m_event_mgr);
	}
	return OPJ_FALSE;
}

/**
 * Reads a tile data. This function is compulsory and allows one to decode tile data. opj_read_tile_header should be called before.
 * The user may need to refer to the image got by opj_read_header to understand the size being taken by the tile.
 *
 * @param	p_codec			the jpeg2000 codec.
 * @param	p_tile_index	the index of the tile being decoded, this should be the value set by opj_read_tile_header.
 * @param	p_data			pointer to a memory block that will hold the decoded data.
 * @param	p_data_size		size of p_data. p_data_size should be bigger or equal to the value set by opj_read_tile_header.
 * @param	p_stream		the stream to decode.
 *
 * @return	true			if the data could be decoded.
 */
opj_bool OPJ_CALLCONV opj_decode_tile_data(
					opj_codec_t *p_codec,
					OPJ_UINT32 p_tile_index,
					OPJ_BYTE * p_data,
					OPJ_UINT32 p_data_size,
					opj_stream_t *p_stream
					)
{
	if (p_codec && p_data && p_stream) {
		opj_codec_private_t * l_info = (opj_codec_private_t *) p_codec;
		opj_stream_private_t * l_cio = (opj_stream_private_t *) p_stream;

		if (! l_info->is_decompressor) {
			return OPJ_FALSE;
		}

		return l_info->m_codec_data.m_decompression.opj_decode_tile_data(	l_info->m_codec,
																			p_tile_index,
																			p_data,
																			p_data_size,
																			l_cio,
																			l_info->m_event_mgr);
	}
	return OPJ_FALSE;
}

/*
 *
 *
 */
void OPJ_CALLCONV opj_dump_codec(	opj_codec_t *p_codec,
									OPJ_INT32 info_flag,
									FILE* output_stream)
{
	if (p_codec) {
		opj_codec_private_t* l_codec = (opj_codec_private_t*) p_codec;

		l_codec->opj_dump_codec(l_codec->m_codec, info_flag, output_stream);
		return;
	}

	fprintf(stderr, "[ERROR] Input parameter of the dump_codec function are incorrect.\n");
	return;
}

/*
 *
 *
 */
opj_codestream_info_v2_t* OPJ_CALLCONV opj_get_cstr_info(opj_codec_t *p_codec)
{
	if (p_codec) {
		opj_codec_private_t* l_codec = (opj_codec_private_t*) p_codec;

		return l_codec->opj_get_codec_info(l_codec->m_codec);
	}

	return NULL;
}

/*
 *
 *
 */
opj_codestream_index_t * OPJ_CALLCONV opj_get_cstr_index(opj_codec_t *p_codec)
{
	if (p_codec) {
		opj_codec_private_t* l_codec = (opj_codec_private_t*) p_codec;

		return l_codec->opj_get_codec_index(l_codec->m_codec);
	}

	return NULL;
}

opj_bool OPJ_CALLCONV opj_decode_v2(opj_codec_t *p_info,
									opj_stream_t *cio,
									opj_image_t* p_image)
{
	if (p_info && cio) {
		opj_codec_private_t * l_info = (opj_codec_private_t *) p_info;
		opj_stream_private_t * l_cio = (opj_stream_private_t *) cio;

		if (! l_info->is_decompressor) {
			return OPJ_FALSE;
		}

		return l_info->m_codec_data.m_decompression.opj_decode(	l_info->m_codec,
																l_cio,
																p_image,
																l_info->m_event_mgr);
	}

	return OPJ_FALSE;
}

opj_bool OPJ_CALLCONV opj_end_decompress (opj_codec_t *p_codec,opj_stream_t *p_cio)
{
	if (p_codec && p_cio) {
		opj_codec_private_t * l_info = (opj_codec_private_t *) p_codec;
		opj_stream_private_t * l_cio = (opj_stream_private_t *) p_cio;

		if (! l_info->is_decompressor) {
			return OPJ_FALSE;
		}
		return l_info->m_codec_data.m_decompression.opj_end_decompress(	l_info->m_codec,
																		l_cio,
																		l_info->m_event_mgr);
	}

	return OPJ_FALSE;
}
