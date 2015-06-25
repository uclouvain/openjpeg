#include "t1_plugin.h"

static const char* PluginId = "t1_plugin";

PLUGIN_API int32_t exit_func()
{
	return 0;
}

PLUGIN_API  void * create(minpf_object_params *params) {
	return 0;
}

PLUGIN_API  int32_t destroy(void * object) {
	return 0;
}

PLUGIN_API minpf_exit_func minpf_init_plugin(const minpf_platform_services * params)
{
	int res = 0;

	minpf_register_params rp;
	rp.version.major = 1;
	rp.version.minor = 0;

	rp.createFunc = create;
	rp.destroyFunc = destroy;

	res = params->registerObject(PluginId, &rp);
	if (res < 0)
		return 0;

#ifdef _OPENMP
	omp_set_num_threads(OPJ_NUM_CORES);
#endif

	return exit_func;
}


PLUGIN_API OPJ_BOOL encode_tile(opj_tcd_t *p_tcd,
	OPJ_UINT32 p_tile_no,
	OPJ_BYTE *p_dest,
	OPJ_UINT32 * p_data_written,
	OPJ_UINT32 p_max_length,
	opj_codestream_info_t *p_cstr_info) {

	return opj_tcd_encode_tile(p_tcd,
								p_tile_no,
								p_dest,
								p_data_written,
								p_max_length,
								p_cstr_info);

}

PLUGIN_API OPJ_BOOL decode_tile(opj_tcd_t *p_tcd,
	OPJ_BYTE *p_src,
	OPJ_UINT32 p_max_length,
	OPJ_UINT32 p_tile_no,
	opj_codestream_index_t *p_cstr_index) {

	return opj_tcd_decode_tile(p_tcd,
								p_src,
								p_max_length,
								p_tile_no,
								p_cstr_index);
}


