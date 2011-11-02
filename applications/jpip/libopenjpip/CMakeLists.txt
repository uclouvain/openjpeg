INCLUDE_REGULAR_EXPRESSION("^.*$")

INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  ${FCGI_INCLUDE_DIRNAME}
)

# Defines the source code for the library
SET(OPENJPIP_SRCS
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
)

SET(LOCAL_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/jp2k_decoder.c
${CMAKE_CURRENT_SOURCE_DIR}/imgsock_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/jpipstream_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/cache_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/dec_clientmsg_handler.c
)

# Build the library
ADD_LIBRARY(openjpip_local STATIC ${OPENJPIP_SRCS} ${LOCAL_SRCS})
TARGET_LINK_LIBRARIES(openjpip_local ${OPENJPEG_LIBRARY_NAME})

ADD_LIBRARY(openjpip_server STATIC ${OPENJPIP_SRCS})
TARGET_LINK_LIBRARIES(openjpip_server ${FCGI_LIBNAME})
SET_TARGET_PROPERTIES(openjpip_server
  PROPERTIES COMPILE_FLAGS "-DSERVER")


# Install library
 INSTALL(TARGETS openjpip_local
   EXPORT OpenJPEGTargets
   DESTINATION ${OPENJPEG_INSTALL_LIB_DIR} COMPONENT Libraries
 )

 INSTALL(TARGETS openjpip_server
   EXPORT OpenJPEGTargets
   DESTINATION ${OPENJPEG_INSTALL_LIB_DIR} COMPONENT Libraries
 )
