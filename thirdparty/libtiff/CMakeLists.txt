cmake_minimum_required(VERSION 2.6)
set(CMAKE_C_STANDARD 99)
project(libtiff C)

string(REPLACE "-Wdeclaration-after-statement" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "-Werror=declaration-after-statement" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "-Wconversion" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "-Wsign-conversion" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

# This convenient copy of libtiff does not support encapsulated zlib or jpeg
# stream. see ZIP_SUPPORT and JPEG_SUPPORT values

include_directories(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")

set(TARGET_FILES
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
  tif_hash_set.c
  tif_hash_set.h
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
  tif_strip.c
  tif_swab.c
  tif_thunder.c
  tif_tile.c
  tif_version.c
  tif_warning.c
  tif_write.c
  tif_zip.c
  tif_jpeg_12.c
  tif_lzma.c
  uvcode.h
  )

if(UNIX)
  set(TARGET_FILES ${TARGET_FILES} tif_unix.c)
  # Large file support
  # This might not catch every possibility catered for by
  # AC_SYS_LARGEFILE.
  add_definitions(-D_FILE_OFFSET_BITS=64)
  set(_FILE_OFFSET_BITS 64)
else()
  set(TARGET_FILES ${TARGET_FILES} tif_win32.c)
endif()

include(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
set(HOST_BIG_ENDIAN ${WORDS_BIGENDIAN})
if(HOST_BIG_ENDIAN)
set(HOST_FILLORDER "FILLORDER_MSB2LSB")
else()
set(HOST_FILLORDER "FILLORDER_LSB2MSB")
endif()
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckCSourceCompiles)
include(TypeSizeChecks.cmake)
include(LibraryFeatures.cmake)

CHECK_INCLUDE_FILES("zlib.h" HAVE_ZLIB_H)
CHECK_INCLUDE_FILES("jpeglib.h" HAVE_JPEGLIB_H)
if(HAVE_JPEGLIB_H)
  set(JPEG_SUPPORT 1)
endif()
if(HAVE_ZLIB_H)
  include_directories(
    ${ZLIB_INCLUDE_DIR}
    )
  set(ZIP_SUPPORT 1)
  set(PIXARLOG_SUPPORT 1) # require zlib
endif()
CHECK_INCLUDE_FILES("assert.h" HAVE_ASSERT_H)
CHECK_INCLUDE_FILES("dlfcn.h" HAVE_DLFCN_H)
CHECK_INCLUDE_FILES("fcntl.h" HAVE_FCNTL_H)
CHECK_INCLUDE_FILES("inttypes.h" HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES("io.h" HAVE_IO_H)
CHECK_INCLUDE_FILES("limits.h" HAVE_LIMITS_H)
CHECK_INCLUDE_FILES("malloc.h" HAVE_MALLOC_H)
CHECK_INCLUDE_FILES("memory.h" HAVE_MEMORY_H)
CHECK_INCLUDE_FILES("search.h" HAVE_SEARCH_H)
CHECK_INCLUDE_FILES("stdint.h" HAVE_STDINT_H)
CHECK_INCLUDE_FILES("stdlib.h" HAVE_STDLIB_H)
CHECK_INCLUDE_FILES("string.h" HAVE_STRING_H)
CHECK_INCLUDE_FILES("strings.h" HAVE_STRINGS_H)
CHECK_INCLUDE_FILES("sys/stat.h" HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILES("sys/time.h" HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES("time.h" HAVE_TIME_H)
CHECK_INCLUDE_FILES("sys/types.h" HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES("unistd.h" HAVE_UNISTD_H)
CHECK_INCLUDE_FILES("windows.h" HAVE_WINDOWS_H)
CHECK_INCLUDE_FILES("strings.h" HAVE_STRINGS_H)
CHECK_INCLUDE_FILES("ieeefp.h" HAVE_IEEEFP_H)

# wotsit ?
if( HAVE_TIME_H AND HAVE_SYS_TIME_H )
 set(TIME_WITH_SYS_TIME 1)
endif()
set(LZW_SUPPORT 1)
set(LZMA_SUPPORT 0) # ?
set(MDI_SUPPORT 1) # ?
set(STDC_HEADERS 1) # why not ?
set(CCITT_SUPPORT 1)
set(THUNDER_SUPPORT 1)
set(HAVE_IEEEFP 1)

CHECK_FUNCTION_EXISTS(getopt HAVE_GETOPT)
CHECK_FUNCTION_EXISTS(isascii HAVE_ISASCII)
CHECK_FUNCTION_EXISTS(memmove HAVE_MEMMOVE)
CHECK_FUNCTION_EXISTS(memset HAVE_MEMSET)
CHECK_FUNCTION_EXISTS(mmap HAVE_MMAP)
CHECK_FUNCTION_EXISTS(strcasecmp HAVE_STRCASECMP)
CHECK_FUNCTION_EXISTS(strchr HAVE_STRCHR)
CHECK_FUNCTION_EXISTS(strrchr HAVE_STRRCHR)
CHECK_FUNCTION_EXISTS(strstr HAVE_STRSTR)
CHECK_FUNCTION_EXISTS(strtol HAVE_STRTOL)
CHECK_FUNCTION_EXISTS(strtoul HAVE_STRTOUL)
CHECK_FUNCTION_EXISTS(strtoull HAVE_STRTOULL)

#
set(VERSION "\"4.6.0\"")
set(PACKAGE_VERSION ${VERSION})
set(PACKAGE "\"tiff\"")

# universal build ?
if(APPLE)
  set(AC_APPLE_UNIVERSAL_BUILD 1)
endif()
if(UNIX)
  set(CMAKE_REQUIRED_LIBRARIES m)
  set(HAVE_LIBM 1)
endif()
CHECK_SYMBOL_EXISTS(floor "math.h" HAVE_FLOOR)
CHECK_SYMBOL_EXISTS(sqrt "math.h" HAVE_SQRT)
CHECK_SYMBOL_EXISTS(pow "math.h" HAVE_POW)
CHECK_SYMBOL_EXISTS(lfind "search.h" HAVE_LFIND)
CHECK_SYMBOL_EXISTS(setmod "io.h" HAVE_SETMODE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tiffconf.h.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/tiffconf.h @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tif_config.h.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/tif_config.h @ONLY)

set(LIBTARGET "tiff")
#
add_library(${LIBTARGET} STATIC ${TARGET_FILES})
#
set_target_properties(${LIBTARGET}
  PROPERTIES
  OUTPUT_NAME "${LIBTARGET}"
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/thirdparty/lib
)
