INCLUDE_REGULAR_EXPRESSION("^.*$")
# Defines the source code for the library
SET(JP3DVM_SRCS
bio.c  cio.c  dwt.c  event.c  jp3d.c  jp3d_lib.c  mct.c  mqc.c  openjpeg.c  pi.c  raw.c  t1.c  t1_3d.c  t2.c  tcd.c  tgt.c  volume.c
)

# Pass proper definition to preprocessor to generate shared lib
IF(WIN32)
  IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_EXPORTS)
  ELSE(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_STATIC)
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(WIN32)

# Create the library
#ADD_LIBRARY(${OPENJPEG_LIBRARY_NAME} ${OPENJPEG_SRCS})
ADD_LIBRARY(${OPJ_PREFIX}openjp3dvm ${JP3DVM_SRCS})

# Install library
INSTALL_TARGETS(/lib/ ${OPJ_PREFIX}openjp3dvm)

# Install includes files
INSTALL_FILES(/include .h)
