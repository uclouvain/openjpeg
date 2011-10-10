
INCLUDE_DIRECTORIES(
  ../libopenjpip
)

ADD_EXECUTABLE(jpip_to_j2k jpip_to_j2k.c)
TARGET_LINK_LIBRARIES(jpip_to_j2k openjpip_local)
IF(UNIX)
  TARGET_LINK_LIBRARIES(jpip_to_j2k m)
ENDIF(UNIX)
# Install exe
INSTALL(TARGETS jpip_to_j2k
  EXPORT OpenJPEGTargets
  DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
)

ADD_EXECUTABLE(jpip_to_jp2 jpip_to_jp2.c)
TARGET_LINK_LIBRARIES(jpip_to_jp2 openjpip_local)
IF(UNIX)
  TARGET_LINK_LIBRARIES(jpip_to_jp2 m)
ENDIF(UNIX)
# Install exe
INSTALL(TARGETS jpip_to_jp2
  EXPORT OpenJPEGTargets
  DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
)

ADD_EXECUTABLE(test_index test_index.c)
TARGET_LINK_LIBRARIES(test_index openjpip_local)
IF(UNIX)
  TARGET_LINK_LIBRARIES(test_index m)
ENDIF(UNIX)
# Install exe
INSTALL(TARGETS test_index
  EXPORT OpenJPEGTargets
  DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
)
