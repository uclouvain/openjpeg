# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/applications/jpip/libopenjpip
  ${FCGI_INCLUDE_DIRNAME}
)

SET(OPJ_SERVER_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/opj_server.c
  )

# Build executable
ADD_EXECUTABLE(opj_server ${OPJ_SERVER_SRCS})
TARGET_LINK_LIBRARIES(opj_server openjpip_server ${FCGI_LIBNAME})
SET_TARGET_PROPERTIES(opj_server
  PROPERTIES COMPILE_FLAGS "-DSERVER -DQUIT_SIGNAL=\\\"quitJPIP\\\""
  )

# On unix you need to link to the math library:
IF(UNIX)
  TARGET_LINK_LIBRARIES(opj_server m)
ENDIF(UNIX)

# Install exe
INSTALL(TARGETS opj_server
  EXPORT OpenJPEGTargets
  DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
  )
#
SET(EXES
  opj_dec_server
  jpip_to_jp2
  jpip_to_j2k
  test_index
  )
FOREACH(exe ${EXES})
  ADD_EXECUTABLE(${exe} ${exe}.c)
  TARGET_LINK_LIBRARIES(${exe} openjpip_local)
  INSTALL(TARGETS ${exe}
    EXPORT OpenJPEGTargets
    DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
    )
ENDFOREACH(exe)
