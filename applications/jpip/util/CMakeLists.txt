# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/applications/jpip/libopenjpip
  ${FCGI_INCLUDE_DIRNAME}
)

SET(OPJ_SERVER_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/opj_server.c
)

# Build executable
ADD_DEFINITIONS(-DSERVER -DQUIT_SIGNAL="quitJPIP")
#
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
#
REMOVE_DEFINITIONS(-DSERVER -DQUIT_SIGNAL="quitJPIP")
#
FOREACH(exe opj_dec_server jpip_to_jp2 jpip_to_j2k test_index)
#
 ADD_EXECUTABLE(${exe} ${exe}.c)
 TARGET_LINK_LIBRARIES(${exe} openjpip_local ${OPENJPEG_LIBRARY_NAME})
  INSTALL(TARGETS ${exe}
   EXPORT OpenJPEGTargets
   DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
  )
ENDFOREACH(exe)
