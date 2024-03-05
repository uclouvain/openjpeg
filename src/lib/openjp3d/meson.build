# Defines the source code for the library
openjp3d_srcs = ['bio.c'
                 , 'cio.c'
                 , 'dwt.c'
                 , 'event.c'
                 , 'jp3d.c'
                 , 'jp3d_lib.c'
                 , 'mct.c'
                 , 'mqc.c'
                 , 'openjp3d.c'
                 , 'pi.c'
                 , 'raw.c'
                 , 't1.c'
                 , 't1_3d.c'
                 , 't2.c'
                 , 'tcd.c'
                 , 'tgt.c'
                 , 'volume.c'
                 ]
openjp3d_headers = ['openjp3d.h']

inc_dirs = include_directories('.', '../openjp2')

openjp3d_args = []

# build jp3d lib:
openjp3d_lib = library('openjp3d', openjp3d_srcs,
  c_args : openjp3d_args,
  include_directories : inc_dirs,
  dependencies : [m_dep],
  install : true,
  version : meson.project_version())

incdir = include_directories('.')

libopenjp3d_dep = declare_dependency(link_with : openjp3d_lib,
  include_directories : inc_dirs)

install_headers(openjp3d_headers)
