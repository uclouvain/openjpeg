SET(INDEXER_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/j2k_to_idxjp2.c
${CMAKE_CURRENT_SOURCE_DIR}/cidx_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/cio_ext.c
${CMAKE_CURRENT_SOURCE_DIR}/event_mgr_handler.c
${CMAKE_CURRENT_SOURCE_DIR}/idxjp2_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/j2k_decoder.c
${CMAKE_CURRENT_SOURCE_DIR}/phix_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/ppix_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/thix_manager.c
${CMAKE_CURRENT_SOURCE_DIR}/tpix_manager.c
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
