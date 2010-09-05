# Build the demo app, small examples

# First thing define the common source:
SET(common_SRCS
  convert.c
  )

# If not getopt was found then add it to the lib:
IF(DONT_HAVE_GETOPT)
  SET(common_SRCS
    ${common_SRCS}
    compat/getopt.c
  )
ENDIF(DONT_HAVE_GETOPT)


# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  )

# Do the proper thing when building static...if only there was configured
# headers or def files instead
IF(NOT BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DOPJ_STATIC)
ENDIF(NOT BUILD_SHARED_LIBS)

FIND_PACKAGE(TIFF REQUIRED)

# Loop over all executables:
FOREACH(exe j2k_to_image image_to_j2k)
  ADD_EXECUTABLE(${exe} ${exe}.c ${common_SRCS})
  TARGET_LINK_LIBRARIES(${exe} ${OPJ_PREFIX}openjpeg ${TIFF_LIBRARIES})
  # On unix you need to link to the math library:
  IF(UNIX)
    TARGET_LINK_LIBRARIES(${exe} -lm)
  ENDIF(UNIX)
  # Install exe
  INSTALL_TARGETS(/bin/ ${exe})
ENDFOREACH(exe)


