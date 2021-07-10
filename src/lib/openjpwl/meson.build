# Makefile for the main JPWL OpenJPEG codecs: JPWL_ j2k_to_image and JPWL_image_to_j2k

openjpeg_srcs = ['../openmj2/bio.c'
                 , '../openmj2/cio.c'
                 , '../openmj2/dwt.c'
                 , '../openmj2/event.c'
                 , '../openmj2/image.c'
                 , '../openmj2/j2k.c'
                 , '../openmj2/jp2.c'
                 , '../openmj2/jpt.c'
                 , '../openmj2/mct.c'
                 , '../openmj2/mqc.c'
                 , '../openmj2/openjpeg.c'
                 , '../openmj2/j2k_lib.c'
                 , '../openmj2/pi.c'
                 , '../openmj2/raw.c'
                 , '../openmj2/t1.c'
                 , '../openmj2/t2.c'
                 , '../openmj2/tcd.c'
                 , '../openmj2/tgt.c']

jpwl_srcs = ['crc.c', 'jpwl.c', 'jpwl_lib.c', 'rs.c']

jpwl_headers = ['jpwl.h']

inc_dirs = include_directories('.', '..', '../openmj2', '../openjp2', '../openjpwl')

openjpwl_c_args = ['-DUSE_JPWL']
openjpwl_link_args = []

if ['darwin', 'ios'].contains(host_system)
  openjpwl_c_args += ['-fno-common']
  # set_source_files_properties(['rs.c', 'PROPERTIES', 'COMPILE_FLAGS', '-fno-common'])
endif


openjpwl_lib = library('openjpwl', openjpeg_srcs + jpwl_srcs,
                        c_args : openjpwl_c_args,
                        link_args : openjpwl_link_args,
                        install : true,
                        dependencies : [m_dep],
                        include_directories : inc_dirs,
                        version : meson.project_version()
                      )
libopenjpwl_dep = declare_dependency(link_with : openjpwl_lib,
              include_directories : inc_dirs)

install_headers(jpwl_headers)
