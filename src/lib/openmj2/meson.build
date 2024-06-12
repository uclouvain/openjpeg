openmj2_srcs = ['mj2.c'
                , 'mj2_convert.c'
                , 'j2k_lib.c'
                , 'cio.c'
                , 'jp2.c'
                , 'j2k.c'
                , 'tcd.c'
                , 'mct.c'
                , 't1.c'
                , 't2.c'
                , 'pi.c'
                , 'bio.c'
                , 'mqc.c'
                , 'tgt.c'
                , 'dwt.c'
                , 'event.c'
                , 'image.c'
                , 'jpt.c'
                , 'raw.c'
                , 'openjpeg.c']


openmj2_headers = ['opj_includes.h', 'openjpeg.h']

inc_dirs = include_directories('.', '../openjp2', '../openjpwl')

openmj2_c_args = ['-DUSE_JPWL']
openmj2_link_args = []


# build mj2 lib:
openmj2_lib = library('openmj2',
                      openmj2_srcs,
                      c_args : openmj2_c_args,
                      link_args : openmj2_link_args,
                      install : true,
                      dependencies : [libopenjpwl_dep, m_dep],
                      include_directories : inc_dirs,
                      version : meson.project_version()
                      )

libopenmj2_dep = declare_dependency(link_with : openmj2_lib,
              include_directories : inc_dirs)

install_headers(openmj2_headers)
