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

#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
#define MINPF_FILE_SEPARATOR   "\\"
#else
#define MINPF_FILE_SEPARATOR   "/"
#endif

#define MINPF_MAX_PATH_LEN 4096

#include "minpf_plugin.h"

#ifdef __cplusplus
}
#endif

