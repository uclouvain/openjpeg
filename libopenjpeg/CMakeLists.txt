INCLUDE_REGULAR_EXPRESSION("^.*$")
# Defines the source code for the library
SET(OPENJPEG_SRCS
  tcd.c
  bio.c
  cio.c
  dwt.c
  event.c
  image.c
  j2k.c
  j2k_lib.c
  jp2.c
  jpt.c
  mct.c
  mqc.c
  openjpeg.c
  pi.c
  raw.c
  t1.c
  tgt.c
  profile.c
  invert.c
  function_list.c
  t2.c
  bio.h
  cio.h
  dwt.h
  event.h
  image.h
  j2k.h
  j2k_lib.h
  jp2.h
  jpt.h
  mct.h
  mqc.h
  openjpeg.h
  pi.h
  int.h
  raw.h
  t1.h
  t2.h
  tcd.h
  tgt.h
  profile.h
  invert.h
  function_list.h
)

# Pass proper definition to preprocessor to generate shared lib
IF(WIN32)
  IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_EXPORTS)
  ELSE(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_STATIC)
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(WIN32)

IF(ENABLE_PROFILING)
    ADD_DEFINITIONS(-D_PROFILE)
ENDIF(ENABLE_PROFILING)
  
# Create the library
ADD_LIBRARY(${OPENJPEG_LIBRARY_NAME} ${OPENJPEG_SRCS})
SET_TARGET_PROPERTIES(${OPENJPEG_LIBRARY_NAME} PROPERTIES
	${OPENJPEG_LIBRARY_PROPERTIES})
IF(UNIX)
  TARGET_LINK_LIBRARIES(${OPENJPEG_LIBRARY_NAME} m)
ENDIF(UNIX)


# Install library
INSTALL(TARGETS ${OPENJPEG_LIBRARY_NAME}
	DESTINATION lib)

# Install includes files
INSTALL(FILES openjpeg.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/openjpeg)
