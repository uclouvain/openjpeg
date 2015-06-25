#pragma once

#include "minpf_plugin.h"
#include "opj_includes.h"

const char* encode_method_name = "encode_tile";
typedef OPJ_BOOL(*ENCODE_TILE)(opj_tcd_t *p_tcd,
							OPJ_UINT32 p_tile_no,
							OPJ_BYTE *p_dest,
							OPJ_UINT32 * p_data_written,
							OPJ_UINT32 p_max_length,
							opj_codestream_info_t *p_cstr_info);

const char* decode_method_name = "decode_tile";
typedef OPJ_BOOL(*DECODE_TILE)(opj_tcd_t *p_tcd,
				OPJ_BYTE *p_src,
				OPJ_UINT32 p_max_length,
				OPJ_UINT32 p_tile_no,
				opj_codestream_index_t *p_cstr_index
				);

