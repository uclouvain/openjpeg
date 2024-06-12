
project('openjp2', 'c', 'cpp', version : '2.3.1',
        meson_version : '>= 0.55.0',
          default_options : [
          'buildtype=debugoptimized',
          'c_std=gnu89'
        ]
        )

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
host_system = host_machine.system()
host_cpu = host_machine.cpu_family()

fs = import('fs')

openjpeg_version_major = '2'
openjpeg_version_minor = '3'
openjpeg_version_build = '1'
openjpeg_package_version = '@0@.@1@.@2@'.format(openjpeg_version_major, openjpeg_version_minor, openjpeg_version_build)


m_dep = cc.find_library('m')
thread_dep = dependency('threads')


# clock_gettime might require -rt, or it might not. find out
if not cc.has_function('clock_gettime', prefix : '#define _GNU_SOURCE\n#include <time.h>')
  # XXX: untested
  rt_dep = cc.find_library('rt')
else
  rt_dep = []
endif

wxwidgets_dep = dependency('wxWidgets', required: get_option('build_viewer'))

subdir('thirdparty')

if host_system == 'windows'
  if get_option('default_library') == 'static'
    add_project_arguments(['-DOPJ_STATIC'], language: 'c')
  else
    add_project_arguments(['-DOPJ_EXPORTS'], language: 'c')
  endif
endif

#-----------------------------------------------------------------------------
# Big endian test:
# test_big_endian('OPJ_BIG_ENDIAN')

#-----------------------------------------------------------------------------
# OpenJPEG build configuration options.
#executable_output_path = [openjpeg_binary_dir, '/bin', 'CACHE', 'PATH', 'Single output directory for building all executables.']
#library_output_path = [openjpeg_binary_dir, '/bin', 'CACHE', 'PATH', 'Single output directory for building all libraries.']
# mark_as_advanced(['LIBRARY_OUTPUT_PATH', 'EXECUTABLE_OUTPUT_PATH'])
#-----------------------------------------------------------------------------
# configure name mangling to allow multiple libraries to coexist
# peacefully

#if fs.exists('openjpeg_mangle.h.in')
#  mangle_prefix = openjpeg_library_name
  # configure_file([cmake_current_source_dir, '/openjpeg_mangle.h.in', cmake_current_binary_dir, '/openjpeg_mangle.h', '@ONLY'])
#endif
#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
# Build Library
if get_option('build_jpip_server')
  curl_dep = dependency('libcurl', version : '>= 7.55.0', required : false)
  fcgi_dep = dependency('fcgi')
  threads_dep = dependency('threads')
endif

subdir('src/lib')

#-----------------------------------------------------------------------------
# Build Applications

if get_option('build_codec') or  get_option('build_mj2')
  # OFF: It will only build 3rd party libs if they are not found on the system
  # ON: 3rd party libs will ALWAYS be build, and used
  subdir('src/bin')
endif

subdir('wrapping')
#-----------------------------------------------------------------------------
# opj_config.h generation (2/2)
# configure_file([cmake_current_source_dir, '/src/lib/openjp2/opj_config.h.cmake.in', cmake_current_binary_dir, '/src/lib/openjp2/opj_config.h', '@ONLY'])
# configure_file([cmake_current_source_dir, '/src/lib/openjp2/opj_config_private.h.cmake.in', cmake_current_binary_dir, '/src/lib/openjp2/opj_config_private.h', '@ONLY'])
#-----------------------------------------------------------------------------

# build documentation in doc subdir(TODO)
#if get_option('build_doc')
#  subdir('doc')
#endif

#-----------------------------------------------------------------------------
# Buld Testing
if get_option('build_testing')
  if get_option('build_codec')
    # Search openjpeg data needed for the tests
    # They could be found via git on the OpenJPEG GitHub code project
    # git clone https://github.com/uclouvain/openjpeg-data.git
    # find_path(['OPJ_DATA_ROOT', 'README-OPJ-Data', 'PATHS', '$ENV{OPJ_DATA_ROOT}', cmake_source_dir, '/../data', 'NO_DEFAULT_PATH', 'NO_CMAKE_FIND_ROOT_PATH'])
    # Add repository where to find tests
    #subdir('tests')
  else
    # message('FATAL_ERROR: You need build codec to run the tests')
  endif
endif

# install CHANGES and LICENSE
#if get_option('build_doc')
#  if fs.exists('./CHANGES')
#    # install(['FILES', 'CHANGES', 'DESTINATION', openjpeg_install_doc_dir])
#  endif
  # install(['FILES', 'LICENSE', 'DESTINATION', openjpeg_install_doc_dir])
#endif
#-----------------------------------------------------------------------------
# pkgconfig support

if get_option('build_pkgconfig_files')
  pkgconfig = import('pkgconfig')
  pkgconfig.generate(openjp2_lib, name: 'libopenjp2',
                       description: 'JPEG2000 library (Part 1 and 2)',
                       version: meson.project_version(),
                       libraries: openjp2_lib)
  if get_option('build_jpwl')
    pkgconfig.generate(openjpwl_lib, name: 'libopenjpwl',
                       description: 'JPEG2000 Wireless library (Part 11)',
                       version: meson.project_version(),
                       libraries: openjpwl_lib)
  endif
  if get_option('build_jpip')
    pkgconfig.generate(openjpip_lib, name: 'libopenjpip',
                       description: 'JPEG2000 Interactivity tools, APIs and protocols (Part 9)',
                       version: meson.project_version(),
                       libraries: openjpip_lib)
  endif
  if get_option('build_jp3d')
      pkgconfig.generate(openjp3d_lib, name: 'libopenjp3d',
                       description: 'JPEG2000 Extensions for three-dimensional data (Part 10)',
                       version: meson.project_version(),
                       libraries: openjp3d_lib)
  endif
endif
#-----------------------------------------------------------------------------
# build our version of astyle
#with_astyle = ['FALSE', 'CACHE', 'BOOL', 'If you plan to contribute you should reindent with scripts/prepare-commit.sh (using 'our' astyle)']
