
# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/applications/jpip/libopenjpip
  ${FCGI_INCLUDE_DIRNAME}
)

SET(OPJ_SERVER_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/opj_server.c
${CMAKE_CURRENT_SOURCE_DIR}/query_parser.c
${CMAKE_CURRENT_SOURCE_DIR}/channel_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/session_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/jpip_parser.c
)

# Build executable
ADD_DEFINITIONS(-DSERVER)
ADD_EXECUTABLE(opj_server ${OPJ_SERVER_SRCS})
TARGET_LINK_LIBRARIES(opj_server openjpip_server ${FCGI_LIBNAME})

# On unix you need to link to the math library:
IF(UNIX)
  TARGET_LINK_LIBRARIES(opj_server m)
ENDIF(UNIX)

# Install exe
INSTALL(TARGETS opj_server
  EXPORT OpenJPEGTargets
  DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
)
