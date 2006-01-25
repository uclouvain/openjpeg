INCLUDE_REGULAR_EXPRESSION("^.*$")
# Defines the source code for the library
SET(openjpeg_SRCS
  bio.c
  cio.c
  dwt.c
  event.c
  fix.c
  image.c
  int.c
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
  t2.c
  tcd.c
  tgt.c
)

# Handle the dllimport/dllexport for building shared lib on Win32
IF (WIN32)
  IF (BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_SHARED)
  ELSE (BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_STATIC)
  ENDIF (BUILD_SHARED_LIBS)
ENDIF (WIN32)

# Create the library
ADD_LIBRARY(${OPJ_PREFIX}openjpeg ${openjpeg_SRCS})

