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

#include "minpf_common.h"

typedef struct minpf_dynamic_library {

	char path[MINPF_MAX_PATH_LEN];
	void* handle;

} minpf_dynamic_library;

minpf_dynamic_library* minpf_load_dynamic_library(const char* path, char* error);
void* minpf_get_symbol(minpf_dynamic_library* library, char* symbol);


#ifdef  __cplusplus
}
#endif