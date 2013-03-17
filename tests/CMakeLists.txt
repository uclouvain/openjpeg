# Tests
include_directories(
  ${OPENJPEG_BINARY_DIR}/src/lib/openjp2 # opj_config.h
  ${OPENJPEG_BINARY_DIR}/src/bin/common # opj_apps_config.h
  ${OPENJPEG_SOURCE_DIR}/src/lib/openjp2
  ${OPENJPEG_SOURCE_DIR}/src/bin/jp2
  ${OPENJPEG_SOURCE_DIR}/src/bin/common
  ${Z_INCLUDE_DIRNAME}
  ${PNG_INCLUDE_DIRNAME}
  ${TIFF_INCLUDE_DIRNAME}
  )

# First thing define the common source:
set(comparePGXimages_SRCS comparePGXimages.c
  ${OPENJPEG_SOURCE_DIR}/src/bin/jp2/convert.c
  ${OPENJPEG_SOURCE_DIR}/src/bin/common/opj_getopt.c
  )

set(compare_dump_files_SRCS compare_dump_files.c
  ${OPENJPEG_SOURCE_DIR}/src/bin/common/opj_getopt.c)

set(compareRAWimages_SRCS compareRAWimages.c
  ${OPENJPEG_SOURCE_DIR}/src/bin/common/opj_getopt.c)

add_executable(comparePGXimages ${comparePGXimages_SRCS})
target_link_libraries(comparePGXimages
  ${OPENJPEG_LIBRARY_NAME}
  ${PNG_LIBNAME} ${TIFF_LIBNAME}
  )
# To support universal exe:
if(ZLIB_FOUND AND APPLE)
  target_link_libraries(comparePGXimages z)
else(ZLIB_FOUND AND APPLE)
  target_link_libraries(comparePGXimages ${Z_LIBNAME})
endif()

add_executable(compare_dump_files ${compare_dump_files_SRCS})

add_executable(j2k_random_tile_access j2k_random_tile_access.c)
target_link_libraries(j2k_random_tile_access ${OPENJPEG_LIBRARY_NAME})

add_executable(compareRAWimages ${compareRAWimages_SRCS})

add_executable(test_tile_encoder test_tile_encoder.c)
target_link_libraries(test_tile_encoder ${OPENJPEG_LIBRARY_NAME})

# Let's try a couple of possibilities:
add_test(tte0 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder)
add_test(tte1 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder 3 2048 2048 1024 1024 8 1 tte1.j2k)
add_test(tte2 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder 3 2048 2048 1024 1024 8 1 tte2.jp2)
add_test(tte3 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder 1 2048 2048 1024 1024 8 1 tte3.j2k)
add_test(tte4 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder 1  256  256  128  128 8 0 tte4.j2k)
add_test(tte5 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder 1  512  512  256  256 8 0 tte5.j2k)
#add_test(tte6 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder 1 8192 8192  512  512 8 0 tte6.j2k)
#add_test(tte7 ${EXECUTABLE_OUTPUT_PATH}/test_tile_encoder 1 32768 32768 512  512 8 0 tte7.jp2)

add_executable(test_tile_decoder test_tile_decoder.c)
target_link_libraries(test_tile_decoder ${OPENJPEG_LIBRARY_NAME})

add_test(ttd0 ${EXECUTABLE_OUTPUT_PATH}/test_tile_decoder)
set_property(TEST ttd0 APPEND PROPERTY DEPENDS tte0)
add_test(ttd1 ${EXECUTABLE_OUTPUT_PATH}/test_tile_decoder 0 0 1024 1024 tte1.j2k)
set_property(TEST ttd1 APPEND PROPERTY DEPENDS tte1)
add_test(ttd2 ${EXECUTABLE_OUTPUT_PATH}/test_tile_decoder 0 0 1024 1024 tte2.jp2)
set_property(TEST ttd2 APPEND PROPERTY DEPENDS tte2)
#add_test(ttd6 ${EXECUTABLE_OUTPUT_PATH}/test_tile_decoder 0 0  512  512 tte6.j2k)
#set_property(TEST ttd6 APPEND PROPERTY DEPENDS tte6)
#add_test(ttd7 ${EXECUTABLE_OUTPUT_PATH}/test_tile_decoder 0 0  512  512 tte7.jp2)
#set_property(TEST ttd7 APPEND PROPERTY DEPENDS tte7)

add_test(rta1 ${EXECUTABLE_OUTPUT_PATH}/j2k_random_tile_access tte1.j2k)
set_property(TEST rta1 APPEND PROPERTY DEPENDS tte1)
add_test(rta2 ${EXECUTABLE_OUTPUT_PATH}/j2k_random_tile_access tte2.jp2)
set_property(TEST rta2 APPEND PROPERTY DEPENDS tte2)
add_test(rta3 ${EXECUTABLE_OUTPUT_PATH}/j2k_random_tile_access tte3.j2k)
set_property(TEST rta3 APPEND PROPERTY DEPENDS tte3)
add_test(rta4 ${EXECUTABLE_OUTPUT_PATH}/j2k_random_tile_access tte4.j2k)
set_property(TEST rta4 APPEND PROPERTY DEPENDS tte4)
add_test(rta5 ${EXECUTABLE_OUTPUT_PATH}/j2k_random_tile_access tte5.j2k)
set_property(TEST rta5 APPEND PROPERTY DEPENDS tte5)

# No image send to the dashboard if lib PNG is not available.
if(NOT OPJ_HAVE_LIBPNG)
  message(WARNING "Lib PNG seems to be not available: if you want run the non-regression tests with images reported to the dashboard, you need it (try BUILD_THIRDPARTY)")
endif()

add_subdirectory(conformance)
add_subdirectory(nonregression)
add_subdirectory(unit)

if(BUILD_JPIP)
  if(JPIP_SERVER)
    #set(s "http://jpip.example.com/myFCGI?target=16.jp2&fsiz=170,170&cnew=http&type=jpp-stream")
    set(s "${JPIP_SERVER}?target=16.jp2&fsiz=170,170&cnew=http&type=jpp-stream")
    set(p "${CMAKE_CURRENT_BINARY_DIR}/jpip.dat")
    set(md5 "62b00c620fb0a600c5ffd413cada4674")
    add_test(TestJPIP1 ${CMAKE_COMMAND} -DD_URL:STRING=${s} -DD_FILE:PATH=${p}
      -DEXPECTED_MD5=${md5} -P ${PROJECT_SOURCE_DIR}/CMake/JPIPTestDriver.cmake)
  endif()
endif()
