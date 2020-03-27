openjp2_srcs = [   'thread.c'
                  , 'bio.c'
                  , 'cio.c'
                  , 'dwt.c'
                  , 'event.c'
                  , 'image.c'
                  , 'invert.c'
                  , 'j2k.c'
                  , 'jp2.c'
                  , 'mct.c'
                  , 'mqc.c'
                  , 'openjpeg.c'
                  , 'opj_clock.c'
                  , 'pi.c'
                  , 't1.c'
                  , 't2.c'
                  , 'tcd.c'
                  , 'tgt.c'
                  , 'function_list.c'
                  , 'opj_malloc.c'
                  , 'sparse_array.c'
                  ]
openjp2_headers = ['thread.h'
                    , 'bio.h'
                    , 'cio.h'
                    , 'dwt.h'
                    , 'event.h'
                    , 'image.h'
                    , 'invert.h'
                    , 'j2k.h'
                    , 'jp2.h'
                    , 'mct.h'
                    , 'mqc.h'
                    , 'mqc_inl.h'
                    , 'openjpeg.h'
                    , 'opj_clock.h'
                    , 'pi.h'
                    , 't1.h'
                    , 't2.h'
                    , 'tcd.h'
                    , 'tgt.h'
                    , 'function_list.h'
                    , 'opj_codec.h'
                    , 'opj_includes.h'
                    , 'opj_intmath.h'
                    , 'opj_malloc.h'
                    , 'opj_stdint.h'
                    , 'sparse_array.h'
                    ]

if get_option('build_jpip')
  add_project_arguments('-DUSE_JPIP', language: 'c')
  openjp2_srcs += [   'cidx_manager.c'
                     , 'phix_manager.c'
                     , 'ppix_manager.c'
                     , 'thix_manager.c'
                     , 'tpix_manager.c'
                     ]
  openjp2_headers += [ 'cidx_manager.h', 'indexbox_manager.h']
endif

opj_config = configuration_data()
opj_config.set('OPENJPEG_VERSION_MAJOR', openjpeg_version_major)
opj_config.set('OPENJPEG_VERSION_MINOR', openjpeg_version_minor)
opj_config.set('OPENJPEG_VERSION_BUILD', openjpeg_version_build)
check_headers = [
  ['OPJ_HAVE_STDINT_H', 'stdint.h'],
]
foreach h : check_headers
  if cc.has_header(h.get(1))
    opj_config.set(h.get(0), 1)
  endif
endforeach

opj_private_config = configuration_data()
check_headers = [
  ['HAVE_ASSERT_H', 'assert.h'],
  ['HAVE_CTYPE_H', 'ctype.h'],
  ['HAVE_DLFCN_H', 'dlfcn.h'],
  ['HAVE_FLOAT_H', 'float.h'],
  ['OPJ_HAVE_INTTYPES_H', 'inttypes.h'],
  ['OPJ_HAVE_MALLOC_H', 'malloc.h'],
  ['HAVE_MATH_H', 'math.h'],
  ['HAVE_MEMORY_H', 'memory.h'],
  ['HAVE_STDARG_H', 'stdarg.h'],
  ['OPJ_HAVE_STDINT_H', 'stdint.h'],
  ['HAVE_STDLIB_H', 'stdlib.h'],
  ['HAVE_STDIO_H', 'stdio.h'],
  ['HAVE_STRINGS_H', 'strings.h'],
  ['HAVE_STRING_H', 'string.h'],
  ['HAVE_SYS_STAT_H', 'sys/stat.h'],
  ['HAVE_SYS_TYPES_H', 'sys/types.h'],
  ['HAVE_TIME_H', 'time.h'],
  ['HAVE_UNISTD_H', 'unistd.h'],
  ['HAVE_WINSOCK2_H', 'winsock2.h'],
]

foreach h : check_headers
  if cc.has_header(h.get(1))
    opj_private_config.set(h.get(0), 1)
  endif
endforeach

if cc.has_function('_aligned_malloc', prefix : '#include <malloc.h>')
  opj_private_config.set('OPJ_HAVE__ALIGNED_MALLOC', 1)
endif

if cc.has_function('memalign', prefix : '#include <malloc.h>')
  opj_private_config.set('OPJ_HAVE_MEMALIGN', 1)
endif

if cc.has_function('posix_memalign', prefix : '#include <stdlib.h>')
  opj_private_config.set('OPJ_HAVE_POSIX_MEMALIGN', 1)
endif

opj_private_config.set('PACKAGE_VERSION', openjpeg_package_version)

if cc.has_function('fseeko')
  opj_private_config.set('OPJ_HAVE_FSEEKO', 'ON')
else
  opj_private_config.set('OPJ_HAVE_FSEEKO', 'OFF')
endif

#-----------------------------------------------------------------------------
# opj_config.h generation (1/2)
# Check if some include files are provided by the system
# These files are mandatory

# For the following files, we provide an alternative, they are not mandatory


# Enable Large file support
opj_config.set('OPJ_HAVE_LARGEFILES', 1)
# opj_test_large_files('OPJ_HAVE_LARGEFILES')

configure_file(input: 'opj_config.h.cmake.in', output : 'opj_config.h', configuration : opj_config, format : 'cmake@', install_dir: get_option('includedir'))
configure_file(input : 'opj_config_private.h.cmake.in', output : 'opj_config_private.h', configuration : opj_private_config, format : 'cmake@')

inc_dirs = include_directories('.')

openjp2_c_args = ['-DNDEBUG',
      '-fstrict-aliasing',
      '-ffast-math',
      '-funroll-loops',
      '-fomit-frame-pointer',
      '-ffinite-math-only',
      '-Wall',
      '-std=c99']

if get_option('opj_disable_tpsot_fix')
  openjp2_c_args += ['-DOPJ_DISABLE_TPSOT_FIX']
endif


#if get_option('build_doc')
  # install man page of the library
  # install(['FILES', openjpeg_source_dir, '/doc/man/man3/libopenjp2.3', 'DESTINATION', openjpeg_install_man_dir, '/man3'])
#endif

if get_option('build_luts_generator')
  t1_generate_luts_exe = executable('t1_generate_luts', 't1_generate_luts.c', dependencies : [libm])
endif

#################################################################################
# threading configuration
#################################################################################
cmake_thread_prefer_pthread = 'TRUE'
if not get_option('opj_use_thread')
  openjp2_c_args += ('-DMUTEX_stub')
endif

if get_option('opj_use_thread') and host_system == 'windows' and not thread_dep.found()
  # add_definitions('-DMUTEX_win32')
  openjp2_c_args += ('-DMUTEX_win32')
endif

if get_option('opj_use_thread') and thread_dep.found()
  openjp2_c_args += ('-DMUTEX_pthread')
endif

if get_option('opj_use_thread') and not thread_dep.found()
  error('FATAL_ERROR: No thread library found and thread/mutex support is required by "opj_use_thread" option')
endif

open_jp2_deps = [m_dep, thread_dep]

# Build the library
openjp2_lib = library('openjp2', openjp2_srcs,
  include_directories : inc_dirs,
  c_args : openjp2_c_args,
  install : true,
  dependencies : open_jp2_deps,
  version : meson.project_version())

libopenjp2_dep = declare_dependency(link_with : openjp2_lib,
              include_directories : inc_dirs)

install_headers(openjp2_headers)

if get_option('build_unit_tests')
  bench_dwt_exe = executable('bench_dwt', 'bench_dwt.c',
                              dependencies : [libopenjp2_dep, m_dep, thread_dep])

  test_sparse_array_exe = executable('test_sparse_array', 'test_sparse_array.c',
                                      dependencies : [libopenjp2_dep, m_dep, thread_dep])
endif
