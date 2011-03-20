CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(libtiff C)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
#
ADD_DEFINITIONS(-DHAVE_STRING_H=1)

SET(TARGET_FILES
	t4.h
    tiffiop.h
    tif_aux.c
    tif_close.c
    tif_codec.c
    tif_color.c
    tif_compress.c
    tif_dir.c
    tif_dir.h
    tif_dirinfo.c
    tif_dirread.c
    tif_dirwrite.c
    tif_dumpmode.c
    tif_error.c
    tif_extension.c
    tif_fax3.c
    tif_fax3.h
    tif_fax3sm.c
    tif_flush.c
    tif_getimage.c
    tif_jbig.c
    tif_jpeg.c
    tif_luv.c
    tif_lzw.c
    tif_next.c
    tif_ojpeg.c
    tif_open.c
    tif_packbits.c
    tif_pixarlog.c
    tif_predict.c
    tif_predict.h
    tif_print.c
    tif_read.c
    tif_stream.cxx
    tif_strip.c
    tif_swab.c
    tif_thunder.c
    tif_tile.c
    tif_version.c
    tif_warning.c
    tif_write.c
    tif_zip.c
    uvcode.h
    )

IF(UNIX)
    SET(TARGET_FILES ${TARGET_FILES} tif_unix.c)
ENDIF()

IF(WIN32)
    SET(TARGET_FILES ${TARGET_FILES} tif_win32.c)
ENDIF(WIN32)

#IF(APPLE)
#    SET(TARGET_FILES ${TARGET_FILES} tif_apple.c)
#ENDIF(APPLE)

SET(LIBTARGET "tiff")
#
ADD_LIBRARY(${LIBTARGET} STATIC ${TARGET_FILES})
#
IF(MSVC)
  SET_TARGET_PROPERTIES(${LIBTARGET} PROPERTIES PREFIX "lib")
ENDIF(MSVC)
#
SET_TARGET_PROPERTIES(${LIBTARGET}
  PROPERTIES
  OUTPUT_NAME "${LIBTARGET}"
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/thirdparty/lib
)
#
