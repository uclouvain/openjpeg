SET(INDEXER_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/j2k_to_idxjp2.c
${CMAKE_CURRENT_SOURCE_DIR}/ext_libopenjpeg/cidx_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/ext_libopenjpeg/cio_ext.c
${CMAKE_CURRENT_SOURCE_DIR}/event_mgr_handler.c
${CMAKE_CURRENT_SOURCE_DIR}/idxjp2_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/j2k_decoder.c
${CMAKE_CURRENT_SOURCE_DIR}/ext_libopenjpeg/phix_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/ext_libopenjpeg/ppix_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/ext_libopenjpeg/thix_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/ext_libopenjpeg/tpix_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/ext_libopenjpeg/ext_j2k.c
${CMAKE_CURRENT_SOURCE_DIR}/ext_libopenjpeg/ext_jp2.c
)

# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  )

# Build executable

ADD_EXECUTABLE(j2k_to_idxjp2 ${INDEXER_SRCS})
TARGET_LINK_LIBRARIES(j2k_to_idxjp2 ${OPENJPEG_LIBRARY_NAME})

# On unix you need to link to the math library:
IF(UNIX)
  TARGET_LINK_LIBRARIES(j2k_to_idxjp2 m)
ENDIF(UNIX)
# Install exe
INSTALL(TARGETS j2k_to_idxjp2
  EXPORT OpenJPEGTargets
  DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
)
