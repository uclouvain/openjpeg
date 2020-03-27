# Build the demo app, small examples
# First thing define the common source:
common_srcs = ['convert.c'
               ,'../common/opj_getopt.c']

inc_dirs = include_directories('../../lib/openjp2', '../../lib/openjp3d', '../common')

openjp3d_c_args = []

# Loop over all executables:
foreach exe :  ['opj_jp3d_compress', 'opj_jp3d_decompress']
  exe_c = exe + '.c'
  exe_src = [exe_c] + common_srcs
  exe_deps = [libopenjp3d_dep, zlib_dep, rt_dep, m_dep]
  exe_exe = executable (exe, exe_src
                       , c_args : openjp3d_c_args
                       , include_directories : inc_dirs
                       , dependencies: exe_deps
                       , install: true
                       )
endforeach
