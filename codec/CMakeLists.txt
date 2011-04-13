# Build the demo app, small examples

# First thing define the common source:
SET(common_SRCS
  convert.c
  )

IF(WIN32)
  IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_EXPORTS)
  ELSE(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_STATIC)
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(WIN32)

# Loop over all executables:
FOREACH(exe jp3d_to_volume volume_to_jp3d)
  ADD_EXECUTABLE(${exe} ${exe}.c ${common_SRCS})
  TARGET_LINK_LIBRARIES(${exe} ${OPENJPEG3D_LIBRARY_NAME})
  # On unix you need to link to the math library:
  IF(UNIX)
    TARGET_LINK_LIBRARIES(${exe} m)
  ENDIF(UNIX)
  # Install exe
  INSTALL(TARGETS ${exe}
    EXPORT OpenJPEG3DTargets
    DESTINATION ${OPENJPEG3D_INSTALL_BIN_DIR} COMPONENT Applications
  )
ENDFOREACH(exe)
