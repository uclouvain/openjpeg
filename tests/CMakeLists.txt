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

ADD_EXECUTABLE(j2k_random_tile_access j2k_random_tile_access.c)
TARGET_LINK_LIBRARIES(j2k_random_tile_access ${OPENJPEG_LIBRARY_NAME})

ADD_EXECUTABLE(compareRAWimages ${compareRAWimages_SRCS})

ADD_EXECUTABLE(test_tile_encoder test_tile_encoder.c)
TARGET_LINK_LIBRARIES(test_tile_encoder ${OPENJPEG_LIBRARY_NAME})

# Let's try a couple of possibilities:
ADD_TEST(tte0 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder)
ADD_TEST(tte1 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder 3 2048 2048 1024 1024 8 tte1.j2k)
ADD_TEST(tte2 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder 3 2048 2048 1024 1024 8 tte1.jp2)

ADD_EXECUTABLE(test_tile_decoder test_tile_decoder.c)
TARGET_LINK_LIBRARIES(test_tile_decoder ${OPENJPEG_LIBRARY_NAME})

ADD_TEST(ttd0 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder)
set_property(TEST ttd0 APPEND PROPERTY DEPENDS tte0)
ADD_TEST(ttd1 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder 0 0 1024 1024 tte1.j2k)
set_property(TEST ttd1 APPEND PROPERTY DEPENDS tte1)
ADD_TEST(ttd2 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder 0 0 1024 1024 tte1.jp2)
set_property(TEST ttd2 APPEND PROPERTY DEPENDS tte2)

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
