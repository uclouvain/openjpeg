/*
* Copyright (c) 2014-2015, Aaron Boxer (https://github.com/boxerab/MinPF)
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "minpf_dynamic_library.h"
#include "hashmap.h"

#include <stdint.h>

#define MINPF_MAX_PLUGINS 32

typedef struct minpf_plugin_manager {

	minpf_dynamic_library* dynamic_libraries[MINPF_MAX_PLUGINS];
	size_t num_libraries;

	minpf_exit_func exit_functions[MINPF_MAX_PLUGINS];
	size_t num_exit_functions;

    minpf_platform_services platformServices;

	map_t plugins;


} minpf_plugin_manager;


minpf_plugin_manager*  minpf_get_plugin_manager(void);
void                   minpf_cleanup_plugin_manager(void);

int32_t minpf_load_by_path(const char* path);
int32_t minpf_load_all(const char* path, minpf_invoke_service_func func);


#ifdef  __cplusplus
}
#endif