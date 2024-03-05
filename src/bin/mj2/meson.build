# Makefile for the MJ2 codecs of the OpenJPEG library: frames_to_mj2, mj2_to_frames, extract_j2k_from_mj2 and wrap_j2k_in_mj2
common_srcs = ['../common/opj_getopt.c']
inc_dirs = include_directories('../../lib/openmj2','../../lib/openjp2', '../common')

foreach exe :  ['opj_mj2_wrap', 'opj_mj2_extract', 'opj_mj2_decompress', 'opj_mj2_compress']
  jpwl_exe = 'opj_jpwl_${exe}'
  exe_c = exe + '.c'
  exe_src = [exe_c] + common_srcs
  c_args = ['-DUSE_MJ2', '-DUSE_JPWL']
  exe_deps = [common_dep, libopenjp2_dep, libopenmj2_dep, liblcms2_dep, m_dep]
  exe_exe = executable (exe, exe_src
                        , c_args : c_args
                        , include_directories : inc_dirs
                        , dependencies: exe_deps
                        , install: true
                        )
endforeach
