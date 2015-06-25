/*
* Copyright (c) 2014-2015, Aaron Boxer (https://github.com/boxerab/MinPF)
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/

#include "minpf_plugin_manager.h"
#include "minpf_plugin.h"

#pragma warning(disable: 4996) // strcpy may be unsafe

#ifdef _WIN32
#include "windirent.h"
static const char* dynamic_library_extension = "dll";
#else
#include <dirent.h>
#if defined(__APPLE__)
  static const char* dynamic_library_extension = "dylib";
#elif defined(__linux)
    static const char* dynamic_library_extension = "so";
#endif
#endif /* _WIN32 */

minpf_plugin_manager* manager;


static int32_t minpf_initialize_plugin(minpf_init_func initFunc);
static const char *get_filename_ext(const char *filename);


static uint32_t minpf_is_valid_plugin(const char * id, const minpf_register_params * params)
{
  if (!id || id[0] == '\0')
     return 0;
  if (!params ||!params->createFunc || !params->destroyFunc)
    return 0;
  
  return 1;
}


int32_t minpf_register_object(const char * id, const minpf_register_params * params)
{
   int error;
   minpf_plugin_api_version v;
   minpf_register_params* existing_plugin_params;
   minpf_plugin_manager* manager = minpf_get_plugin_manager();
   minpf_register_params* registered_params = NULL;

   if (!id || id[0] == '\0' || !params)
	   return -1;

  if (!minpf_is_valid_plugin(id, params))
    return -1;
 

  v = manager->platformServices.version;
  if (v.major != params->version.major)
    return -1;
  
  // check if plugin is already registered
  existing_plugin_params = (minpf_register_params*)malloc(sizeof(minpf_register_params));
  error = hashmap_get(manager->plugins, id, (void**)(&existing_plugin_params));
  free(existing_plugin_params);
  existing_plugin_params = NULL;
  if (error != MAP_MISSING)
	  return 0;

  registered_params = (minpf_register_params*)malloc(sizeof(minpf_register_params));
  *registered_params = *params;
  error = hashmap_put(manager->plugins, id, registered_params);

  return (error==MAP_OK) ? 0 : -1;

}

void minpf_initialize_plugin_manager(minpf_plugin_manager* manager) {
  if (!manager)
	  return;
  manager->platformServices.version.major = 1;
  manager->platformServices.version.minor = 0;
  manager->platformServices.invokeService = NULL; 
  manager->platformServices.registerObject = minpf_register_object;

  manager->plugins  = hashmap_new();

}

minpf_plugin_manager*  minpf_get_plugin_manager(void){
	if (!manager) {
		manager = (minpf_plugin_manager*)calloc(1, sizeof(minpf_plugin_manager));
		if (!manager)
			return NULL;
		minpf_initialize_plugin_manager(manager);
	}
	return manager;

}

static int minpf_free_hash_value(any_t item, any_t data) {
	if (data)
		free(data);
	return 0;

}
void   minpf_cleanup_plugin_manager(void){
	if (manager) {
		size_t i = 0;

		for (i = 0; i < manager->num_exit_functions; ++i)
			manager->exit_functions[i]();

		for (i = 0; i < manager->num_libraries; ++i) {
			if (manager->dynamic_libraries[i])
				free(manager->dynamic_libraries[i]);
		}

		hashmap_iterate(manager->plugins, minpf_free_hash_value, NULL);
		hashmap_free(manager->plugins);
		free(manager);
	}
	manager = NULL;
}

int32_t minpf_load_by_path(const char* path){
	minpf_init_func initFunc = NULL;
	minpf_dynamic_library* lib = NULL;
	int32_t res = 0;

	minpf_plugin_manager* mgr = minpf_get_plugin_manager();
	if (!mgr || mgr->num_libraries == MINPF_MAX_PLUGINS) {
		return -1;
	}
	lib = minpf_load_dynamic_library(path, NULL);
	if (!lib) {

		return -1;
	}
	initFunc = (minpf_init_func)(minpf_get_symbol(lib, "minpf_init_plugin"));
 	if (!initFunc) {
      return -1;
    }
	mgr->dynamic_libraries[mgr->num_libraries++] = lib;
	return minpf_initialize_plugin(initFunc);
	
}

int32_t minpf_load_all(const char* directory_path, minpf_invoke_service_func func) {
	DIR *dir;
    struct dirent* content;
	char libraryPath[MINPF_MAX_PATH_LEN];
	minpf_plugin_manager* mgr = minpf_get_plugin_manager();

	if (!directory_path || directory_path[0] == '\0') // Check that the path is non-empty.
	  return -1;

    mgr->platformServices.invokeService = func;

    dir= opendir(directory_path);
    if(!dir){
        fprintf(stderr,"Unable to open folder %s\n",directory_path);
        return -1;
    }

    while((content=readdir(dir))!=NULL){

        if(strcmp(".",content->d_name)==0 || strcmp("..",content->d_name)==0 )
            continue;

		//ignore files with incorrect extensions
		if (  strcmp(get_filename_ext(content->d_name), dynamic_library_extension) != 0)
			continue;
		strcpy(libraryPath, directory_path);
		strcat(libraryPath, MINPF_FILE_SEPARATOR);
		strcat(libraryPath, content->d_name);
		if (minpf_load_by_path(libraryPath) != 0) 
			continue;
    }
	free(dir);
	return 0;
}


static int32_t minpf_initialize_plugin(minpf_init_func initFunc)
{
  minpf_plugin_manager* mgr = minpf_get_plugin_manager();

  minpf_exit_func exitFunc = initFunc(&mgr->platformServices);
  if (!exitFunc)
    return -1;
  
  mgr->exit_functions[mgr->num_exit_functions++] = exitFunc;
  return 0;
}


static const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
		return "";
    return dot + 1;
}