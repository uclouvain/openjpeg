INCLUDE_REGULAR_EXPRESSION("^.*$")
# Defines the source code for the library
SET(OpenJPEG_SRCS
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

IF(WIN32 AND BUILD_SHARED_LIBS)
  SET(UNDERSCORE)
  IF(BORLAND)
    SET(UNDERSCORE "_")
  ENDIF(BORLAND)
  CONFIGURE_FILE(
    ${OPENJPEG_SOURCE_DIR}/libopenjpeg/openjpeg.def.in
    ${OPENJPEG_BINARY_DIR}/libopenjpeg/openjpeg.def
    @ONLY IMMEDIATE
    )
  IF(NOT BORLAND)
    SET(OpenJPEG_SRCS ${OpenJPEG_SRCS}
      ${OPENJPEG_BINARY_DIR}/libopenjpeg/openjpeg.def
      )
  ENDIF(NOT BORLAND)
ENDIF(WIN32 AND BUILD_SHARED_LIBS)

# Create the library
ADD_LIBRARY(${OPJ_PREFIX}openjpeg ${OpenJPEG_SRCS})

# Install library
INSTALL_TARGETS(/lib/ ${OPJ_PREFIX}openjpeg)

