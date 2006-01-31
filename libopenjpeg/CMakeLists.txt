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

# Prepare the .def file (Module definition)
# depending on wether we are:
# 1. Building a dll (shared lib)
# 2. Wether we are using cl or bcc32 (underscore convention)
IF(WIN32 AND BUILD_SHARED_LIBS)
  SET(UNDERSCORE)
  # Borland expect symbol to start with _
  IF(BORLAND)
    SET(UNDERSCORE "_")
  ENDIF(BORLAND)
  # Borland will by default use a .def file if present in
  # the binary directory
  CONFIGURE_FILE(
    ${OPENJPEG_SOURCE_DIR}/libopenjpeg/openjpeg.def.in
    ${OPENJPEG_BINARY_DIR}/libopenjpeg/openjpeg.def
    @ONLY IMMEDIATE
    )
  # You can directly pass a .def file to cl this is
  # recognized
  IF(NOT BORLAND AND NOT MINGW)
    SET(OpenJPEG_SRCS ${OpenJPEG_SRCS}
      ${OPENJPEG_BINARY_DIR}/libopenjpeg/openjpeg.def
      )
  ENDIF(NOT BORLAND AND NOT MINGW)
ENDIF(WIN32 AND BUILD_SHARED_LIBS)

# Create the library
ADD_LIBRARY(${OPJ_PREFIX}openjpeg ${OpenJPEG_SRCS})

# Install library
INSTALL_TARGETS(/lib/ ${OPJ_PREFIX}openjpeg)

