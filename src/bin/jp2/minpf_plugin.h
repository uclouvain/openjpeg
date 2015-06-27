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

#include <stdint.h>
 
struct minpf_platform_services;


typedef struct minpf_object_params
{
  const char * id;
  const struct minpf_platform_services * platformServices;
} minpf_object_params;

typedef struct minpf_plugin_api_version
{
  int32_t major;
  int32_t minor;
} minpf_plugin_api_version;

typedef void * (*minpf_create_func)(minpf_object_params *); 
typedef int32_t (*minpf_destroy_func)(void *);


typedef struct minpf_register_params
{
  minpf_plugin_api_version version;
  minpf_create_func createFunc;
  minpf_destroy_func destroyFunc;
} minpf_register_params;


typedef int32_t (*minpf_register_func)(const char * nodeType, const minpf_register_params * params);
typedef int32_t (*minpf_invoke_service_func)(const char * serviceName, void * serviceParams);

typedef struct minpf_platform_services
{
  minpf_plugin_api_version version;
  minpf_register_func registerObject; 
  minpf_invoke_service_func invokeService; 
} minpf_platform_services;


typedef int32_t (*minpf_exit_func)();

typedef minpf_exit_func (*minpf_init_func)(const minpf_platform_services *);

#ifndef PLUGIN_API
  #ifdef WIN32
	#define PLUGIN_API __declspec(dllexport)
  #else
    #define PLUGIN_API
  #endif
#endif

extern 
#ifdef  __cplusplus
"C" 
#endif
PLUGIN_API minpf_exit_func minpf_init_plugin(const minpf_platform_services * params);

#ifdef  __cplusplus
}
#endif

