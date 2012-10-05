include_regular_expression("^.*$")

add_definitions(-DUSE_JPIP)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OPENJPEG_BINARY_DIR}/src/lib/openjp2 # opj_config.h
  ${OPENJPEG_SOURCE_DIR}/src/lib/openjp2
  ${FCGI_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
)

# Defines the source code for the library
set(OPENJPIP_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/boxheader_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/codestream_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/imgreg_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/marker_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/msgqueue_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/box_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/faixbox_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/index_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/metadata_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/placeholder_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/byte_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/ihdrbox_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/manfbox_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/mhixbox_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/target_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cachemodel_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/j2kheader_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/jp2k_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjpip.c
  ${CMAKE_CURRENT_SOURCE_DIR}/query_parser.c
  ${CMAKE_CURRENT_SOURCE_DIR}/channel_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/session_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/jpip_parser.c
  ${CMAKE_CURRENT_SOURCE_DIR}/sock_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cidx_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/phix_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/ppix_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/thix_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/tpix_manager.c
  )

set(SERVER_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/auxtrans_manager.c
  )

set(LOCAL_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/jp2k_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/imgsock_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/jpipstream_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cache_manager.c
  ${CMAKE_CURRENT_SOURCE_DIR}/dec_clientmsg_handler.c
  )

# Build the library
add_library(openjpip_local STATIC ${OPENJPIP_SRCS} ${LOCAL_SRCS})
target_link_libraries(openjpip_local ${OPENJPEG_LIBRARY_NAME})
if(WIN32)
  # add Winsock on windows+mingw
  target_link_libraries(openjpip_local ws2_32)
endif()

# Install library
install(TARGETS openjpip_local
  EXPORT OpenJPEGTargets
  DESTINATION ${OPENJPEG_INSTALL_LIB_DIR} COMPONENT Libraries
  )

if(BUILD_JPIP_SERVER)
  add_library(openjpip_server STATIC ${OPENJPIP_SRCS} ${SERVER_SRCS})
  target_link_libraries(openjpip_server ${FCGI_LIBRARIES} ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  set_target_properties(openjpip_server
    PROPERTIES COMPILE_FLAGS "-DSERVER")
  install(TARGETS openjpip_server
    EXPORT OpenJPEGTargets
    DESTINATION ${OPENJPEG_INSTALL_LIB_DIR} COMPONENT Libraries
    )
endif()
