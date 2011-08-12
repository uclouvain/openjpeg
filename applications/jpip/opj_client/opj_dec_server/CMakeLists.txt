
SET(OPJ_DEC_SERVER_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/cache_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/opj_dec_server.c
${CMAKE_CURRENT_SOURCE_DIR}/jp2k_decoder.c
${CMAKE_CURRENT_SOURCE_DIR}/imgsock_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/jpipstream_manager.c
)

INCLUDE_DIRECTORIES(
  ../../libopenjpip
)

# Build executable

ADD_EXECUTABLE(opj_dec_server ${OPJ_DEC_SERVER_SRCS})
TARGET_LINK_LIBRARIES(opj_dec_server openjpip_local ${OPENJPEG_LIBRARY_NAME})

# On unix you need to link to the math library:
IF(UNIX)
  TARGET_LINK_LIBRARIES(opj_dec_server m)
ENDIF(UNIX)
# Install exe
INSTALL(TARGETS opj_dec_server
  EXPORT OpenJPEGTargets
  DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
)
