# Build the demo app, small examples

# First thing define the common source:
SET(common_SRCS
  convert.c
  )
# Then check if getopt is present:
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
SET(DONT_HAVE_GETOPT 1)
IF(UNIX) #I am pretty sure only *nix sys have this anyway
  CHECK_INCLUDE_FILE("getopt.h" CMAKE_HAVE_GETOPT_H)
  # Seems like we need the contrary:
  IF(CMAKE_HAVE_GETOPT_H)
    SET(DONT_HAVE_GETOPT 0)
  ENDIF(CMAKE_HAVE_GETOPT_H)
ENDIF(UNIX)

# If not getopt was found then add it to the lib:
IF(DONT_HAVE_GETOPT)
  ADD_DEFINITIONS(-DDONT_HAVE_GETOPT)
  SET(common_SRCS
    ${common_SRCS}
    getopt.c
  )
ENDIF(DONT_HAVE_GETOPT)

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
  TARGET_LINK_LIBRARIES(${exe} ${OPENJPEG_LIBRARY_NAME}_JP3D) # ${TIFF_LIBRARIES})
  # On unix you need to link to the math library:
  IF(UNIX)
    TARGET_LINK_LIBRARIES(${exe} m)
  ENDIF(UNIX)
  # Install exe
  INSTALL_TARGETS(/bin/ ${exe})
ENDFOREACH(exe)


