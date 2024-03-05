# jpwl apps
# First thing define the common source:
common_srcs = ['convert.c'
              , 'index.c'
              , '../common/color.c'
              , '../common/opj_getopt.c']
# Headers file are located here:
inc_dirs = include_directories('../../lib/openmj2', '../../lib/openjp2', '../common')

openjpwl_c_args = []


openjpwl_c_args += ['-DOPJ_USE_LEGACY', '-DUSE_JPWL']


foreach exe :  ['decompress', 'compress']
  jpwl_exe = 'opj_jpwl_' + exe
  exe_c = jpwl_exe + '.c'
  exe_src = [exe_c] + common_srcs
  exe_deps = [libopenjpwl_dep,  libpng_dep, libtiff_dep, liblcms2_dep, m_dep, zlib_dep ]
  exe_exe = executable (exe, exe_src
                        , c_args : openjpwl_c_args
                        , include_directories : inc_dirs
                        , dependencies: exe_deps
                        , install: true
                        )
endforeach
