openjpip_srcs = [  'boxheader_manager.c'
                 , 'codestream_manager.c'
                 , 'imgreg_manager.c'
                 , 'marker_manager.c'
                 , 'msgqueue_manager.c'
                 , 'box_manager.c'
                 , 'faixbox_manager.c'
                 , 'index_manager.c'
                 , 'metadata_manager.c'
                 , 'placeholder_manager.c'
                 , 'byte_manager.c'
                 , 'ihdrbox_manager.c'
                 , 'manfbox_manager.c'
                 , 'mhixbox_manager.c'
                 , 'target_manager.c'
                 , 'cachemodel_manager.c'
                 , 'j2kheader_manager.c'
                 , 'jp2k_encoder.c'
                 , 'openjpip.c'
                 , 'query_parser.c'
                 , 'channel_manager.c'
                 , 'session_manager.c'
                 , 'jpip_parser.c'
                 , 'sock_manager.c'
                 , '../openjp2/opj_malloc.c']

server_srcs = ['auxtrans_manager.c']

local_srcs = [  'jp2k_decoder.c'
              , 'imgsock_manager.c'
              , 'jpipstream_manager.c'
              , 'cache_manager.c'
              , 'dec_clientmsg_handler.c']

openjpip_headers = ['openjpip.h'
                    , 'session_manager.h'
                    , 'target_manager.h'
                    , 'query_parser.h'
                    , 'msgqueue_manager.h'
                    , 'sock_manager.h'
                    , 'auxtrans_manager.h']

openjpip_args = ['-DUSE_JPIP']

inc_dirs = include_directories('.', '../openjp2')

deps = [libopenjp2_dep, m_dep]

if host_system == 'windows'
    deps += [cc.find_library('ws2_32')]
endif

# build jpip lib:
openjpip_lib = library('openjpip', openjpip_srcs + local_srcs,
  c_args : openjpip_args,
  include_directories : inc_dirs,
  dependencies: deps,
  install : true,
  version : meson.project_version())

libopenjpip_dep = declare_dependency(link_with : openjpip_lib,
  include_directories : inc_dirs)


# Install library
if get_option('build_jpip_server')
  openjpip_server_lib = static_library('openjpip_server', openjpip_srcs + server_srcs
                                      , c_args : ['-DSERVER']
                                      , dependencies: [fcgi_dep, curl_dep]
                                      , version: meson.project_version())
endif

install_headers(openjpip_headers)
