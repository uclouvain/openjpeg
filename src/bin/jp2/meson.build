# Build the demo app, small examples

# First thing define the common source:
common_srcs = ['convert.c'
               , 'convertbmp.c'
               , 'index.c'
               , '../common/color.c'
               , '../common/opj_getopt.c'
               ]

if libtiff_dep.found()
  common_srcs += ['converttif.c']
endif

if libpng_dep.found()
  common_srcs += ['convertpng.c']
endif

# Headers file are located here:
inc_dirs = include_directories('../../lib/openjp2', '../common')

openjp2_c_args = []

# Loop over all executables:
foreach exe :  ['opj_decompress', 'opj_compress', 'opj_dump']
  exe_c = exe + '.c'
  exe_src = [exe_c] + common_srcs
  exe_deps = [libopenjp2_dep, libpng_dep, libtiff_dep, liblcms2_dep, zlib_dep, rt_dep, m_dep]
  exe_exe = executable (exe, exe_src
                        , c_args : openjp2_c_args
                        , include_directories : inc_dirs
                        , dependencies : exe_deps
                        , install : true
                        )
endforeach

#if get_option('opj_use_dsymutil')
  # add_custom_command(['TARGET', exe, 'POST_BUILD', 'COMMAND', 'dsymutil', '$<TARGET_FILE:${exe}>', 'COMMENT', 'dsymutil $<TARGET_FILE:${exe}>', 'DEPENDS', exe])
#endif

#if get_option('build_doc')
  # Install man pages
  # install(['FILES', openjpeg_source_dir, '/doc/man/man1/opj_compress.1', openjpeg_source_dir, '/doc/man/man1/opj_decompress.1', openjpeg_source_dir, '/doc/man/man1/opj_dump.1', 'DESTINATION', openjpeg_install_man_dir, '/man1'])
  #
#endif
