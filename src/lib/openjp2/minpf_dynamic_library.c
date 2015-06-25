/*
* Copyright (c) 2014-2015, Aaron Boxer (https://github.com/boxerab/MinPF)
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/
#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif


#include "minpf_dynamic_library.h"

#pragma warning(disable: 4996) // strcpy may be unsafe



minpf_dynamic_library*  minpf_load_dynamic_library(const char* path, char* error) {

  minpf_dynamic_library* lib = NULL;
  void * handle = NULL;

  if (!path)
	  return NULL;

  #ifdef WIN32
    handle = LoadLibrary(path);
    if (handle == NULL)
    {
		return NULL;
    }
  #else
    handle = ::dlopen(name, RTLD_NOW);
    if (!handle) 
    {
    
       //report error
      return NULL;
    }

  #endif

	lib = (minpf_dynamic_library*)calloc(1, sizeof(minpf_dynamic_library));
	if (!lib)
		return NULL;
	strcpy(lib->path, path);
	lib->handle = handle;
	return lib;
}

void* minpf_get_symbol(minpf_dynamic_library* library, char* symbol) {

  if (!library || !library->handle)
	 return NULL;

  void* rc = NULL;
  
  #ifdef WIN32
    rc =  GetProcAddress((HMODULE)library->handle, symbol);
	if (!rc) {

		int err = GetLastError();
	}
  #else
    rc =  dlsym(library->handle, symbol);
  #endif

	return rc;

}
