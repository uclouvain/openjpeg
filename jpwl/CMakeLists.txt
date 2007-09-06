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

ADD_LIBRARY(openjpeg_JPWL ${JPWL_SRCS} ${OPJ_SRCS})

# Do the proper thing when building static...if only there was configured
# headers or def files instead
#IF(NOT BUILD_SHARED_LIBS)
#  ADD_DEFINITIONS(-DOPJ_STATIC)
#ENDIF(NOT BUILD_SHARED_LIBS)

INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  )

FIND_PACKAGE(TIFF REQUIRED)

ADD_EXECUTABLE(JPWL_j2k_to_image
../codec/convert.c ../codec/j2k_to_image.c
)
TARGET_LINK_LIBRARIES(JPWL_j2k_to_image openjpeg_JPWL ${TIFF_LIBRARIES})
IF(UNIX)
  TARGET_LINK_LIBRARIES(JPWL_j2k_to_image m)
ENDIF(UNIX)

ADD_EXECUTABLE(JPWL_image_to_j2k
../codec/convert.c ../codec/image_to_j2k.c)
TARGET_LINK_LIBRARIES(JPWL_image_to_j2k openjpeg_JPWL ${TIFF_LIBRARIES})
IF(UNIX)
  TARGET_LINK_LIBRARIES(JPWL_image_to_j2k m)
ENDIF(UNIX)

