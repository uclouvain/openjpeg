# Tests
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  ${OPENJPEG_SOURCE_DIR}/applications/codec
  ${OPENJPEG_SOURCE_DIR}/applications/common
  ${Z_INCLUDE_DIRNAME}
  ${PNG_INCLUDE_DIRNAME}
  ${TIFF_INCLUDE_DIRNAME}
  )

# First thing define the common source:
SET(comparePGXimages_SRCS comparePGXimages.c
  ${OPENJPEG_SOURCE_DIR}/applications/codec/convert.c
  ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c
  )

SET(compare_dump_files_SRCS compare_dump_files.c
  ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c)

SET(compareRAWimages_SRCS compareRAWimages.c
  ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c)

ADD_EXECUTABLE(comparePGXimages ${comparePGXimages_SRCS})
TARGET_LINK_LIBRARIES(comparePGXimages
  ${OPENJPEG_LIBRARY_NAME}
  ${PNG_LIBNAME} ${TIFF_LIBNAME}
  )
# To support universal exe:
IF(ZLIB_FOUND AND APPLE)
  TARGET_LINK_LIBRARIES(comparePGXimages z)
ELSe(ZLIB_FOUND AND APPLE)
  TARGET_LINK_LIBRARIES(comparePGXimages ${Z_LIBNAME})
ENDIF(ZLIB_FOUND AND APPLE)

ADD_EXECUTABLE(compare_dump_files ${compare_dump_files_SRCS})

ADD_EXECUTABLE(compareRAWimages ${compareRAWimages_SRCS})

# No image send to the dashboard if lib PNG is not available.
IF(NOT HAVE_LIBPNG)
  MESSAGE(WARNING "Lib PNG seems to be not available: if you want run the non-regression tests with images reported to the dashboard, you need it (try BUILD_THIRDPARTY)") 
ENDIF(NOT HAVE_LIBPNG)

ADD_SUBDIRECTORY(conformance)
ADD_SUBDIRECTORY(nonregression)

IF(BUILD_JPIP)
  IF(JPIP_SERVER)
    #SET(s "http://jpip.example.com/myFCGI?target=16.jp2&fsiz=170,170&cnew=http&type=jpp-stream")
    SET(s "${JPIP_SERVER}?target=16.jp2&fsiz=170,170&cnew=http&type=jpp-stream")
    SET(p "${CMAKE_CURRENT_BINARY_DIR}/jpip.dat")
    SET(md5 "62b00c620fb0a600c5ffd413cada4674")
    ADD_TEST(TestJPIP1 ${CMAKE_COMMAND} -DD_URL:STRING=${s} -DD_FILE:PATH=${p}
      -DEXPECTED_MD5=${md5} -P ${PROJECT_SOURCE_DIR}/CMake/JPIPTestDriver.cmake)
  ENDIF(JPIP_SERVER)
ENDIF(BUILD_JPIP)
