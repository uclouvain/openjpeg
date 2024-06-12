# opj_apps_config.h generation
configure_file(input : 'opj_apps_config.h.cmake.in', output : 'opj_apps_config.h', configuration : cappdata, format: 'cmake@')

common_srcs = [ 'color.c']


inc_dirs = include_directories('.','../../lib/openjp2')

common_lib = static_library('common', common_srcs,
                          include_directories : inc_dirs,
                          dependencies : [liblcms2_dep]
                          )
common_dep = declare_dependency(link_with : common_lib,
          include_directories : inc_dirs)
