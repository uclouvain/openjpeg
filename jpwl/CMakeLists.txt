# Makefile for the main JPWL OpenJPEG codecs: JPWL_ j2k_to_image and JPWL_image_to_j2k

ADD_DEFINITIONS(-DUSE_JPWL)

SET(OPJ_SRCS
../libopenjpeg/bio.c
../libopenjpeg/cio.c
../libopenjpeg/dwt.c
../libopenjpeg/event.c
../libopenjpeg/image.c
../libopenjpeg/j2k.c
../libopenjpeg/j2k_lib.c
../libopenjpeg/jp2.c
../libopenjpeg/jpt.c
../libopenjpeg/mct.c
../libopenjpeg/mqc.c
../libopenjpeg/openjpeg.c
../libopenjpeg/pi.c
../libopenjpeg/raw.c
../libopenjpeg/t1.c
../libopenjpeg/t2.c
../libopenjpeg/tcd.c
../libopenjpeg/tgt.c
)
SET(JPWL_SRCS crc.c jpwl.c jpwl_lib.c rs.c)

SET(common_SRCS "")
# If not getopt was found then add it to the lib:
IF(DONT_HAVE_GETOPT)
  SET(common_SRCS ${OPENJPEG_SOURCE_DIR}/common/getopt.c)
ENDIF(DONT_HAVE_GETOPT)

# Build the library
IF(WIN32)
  IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_EXPORTS)
  ELSE(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_STATIC)
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(WIN32)
ADD_LIBRARY(${OPENJPEG_LIBRARY_NAME}_JPWL ${JPWL_SRCS} ${OPJ_SRCS})
SET_TARGET_PROPERTIES(${OPENJPEG_LIBRARY_NAME}_JPWL PROPERTIES ${OPENJPEG_LIBRARY_PROPERTIES})

# Install library
INSTALL(TARGETS ${OPENJPEG_LIBRARY_NAME}_JPWL
  DESTINATION ${OPENJPEG_INSTALL_LIB_DIR} COMPONENT Libraries
)

# Build executables

INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  ${OPENJPEG_SOURCE_DIR}/common
  )

IF(LCMS_INCLUDE_DIR)
  INCLUDE_DIRECTORIES( ${LCMS_INCLUDE_DIR} )
ENDIF(LCMS_INCLUDE_DIR)
IF(PNG_FOUND)
  INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
ENDIF(PNG_FOUND)
IF(TIFF_FOUND)
  INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
ENDIF(TIFF_FOUND)


ADD_EXECUTABLE(JPWL_j2k_to_image
../codec/j2k_to_image.c
../codec/convert.c
../codec/index.c
${OPENJPEG_SOURCE_DIR}/common/color.c
${common_SRCS}
)

TARGET_LINK_LIBRARIES(JPWL_j2k_to_image ${OPENJPEG_LIBRARY_NAME}_JPWL ${LCMS_LIB})
IF(PNG_FOUND)
  TARGET_LINK_LIBRARIES(JPWL_j2k_to_image ${PNG_LIBRARIES})
ENDIF(PNG_FOUND)
IF(TIFF_FOUND)
  TARGET_LINK_LIBRARIES(JPWL_j2k_to_image ${TIFF_LIBRARIES})
ENDIF(TIFF_FOUND)
IF(UNIX)
  TARGET_LINK_LIBRARIES(JPWL_j2k_to_image m)
ENDIF(UNIX)


ADD_EXECUTABLE(JPWL_image_to_j2k
../codec/convert.c
../codec/index.c
../codec/image_to_j2k.c
${common_SRCS}
)

TARGET_LINK_LIBRARIES(JPWL_image_to_j2k ${OPENJPEG_LIBRARY_NAME}_JPWL ${LCMS_LIB})
IF(PNG_FOUND)
  TARGET_LINK_LIBRARIES(JPWL_image_to_j2k ${PNG_LIBRARIES})
ENDIF(PNG_FOUND)
IF(TIFF_FOUND)
  TARGET_LINK_LIBRARIES(JPWL_image_to_j2k ${TIFF_LIBRARIES})
ENDIF(TIFF_FOUND)
IF(UNIX)
  TARGET_LINK_LIBRARIES(JPWL_image_to_j2k m)
ENDIF(UNIX)

INSTALL(TARGETS JPWL_image_to_j2k JPWL_j2k_to_image
  DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Binaries
)
