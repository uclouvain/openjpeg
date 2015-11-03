# Tests
include_directories(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  ${OPENJPEG_SOURCE_DIR}/applications/codec
  ${OPENJPEG_SOURCE_DIR}/applications/common
  ${Z_INCLUDE_DIRNAME}
  ${PNG_INCLUDE_DIRNAME}
  ${TIFF_INCLUDE_DIRNAME}
  )

# First thing define the common source:
set(compare_images_SRCS compare_images.c
  ${OPENJPEG_SOURCE_DIR}/applications/codec/convert.c
  ${OPENJPEG_SOURCE_DIR}/applications/codec/converttif.c
#  ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c
  )

if(NOT USE_SYSTEM_GETOPT)
  list(APPEND compare_images_SRCS
    ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c
  )
endif()

set(compare_dump_files_SRCS compare_dump_files.c
  #${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c
  )

if(NOT USE_SYSTEM_GETOPT)
  list(APPEND compare_dump_files_SRCS
    ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c
  )
endif()

set(compare_raw_files_SRCS compare_raw_files.c
  #${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c
  )

if(NOT USE_SYSTEM_GETOPT)
  list(APPEND compare_raw_files_SRCS
    ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c
  )
endif()

add_executable(compare_images ${compare_images_SRCS})
target_link_libraries(compare_images
  ${OPENJPEG_LIBRARY_NAME}
  ${PNG_LIBNAME} ${TIFF_LIBNAME}
  )
# To support universal exe:
if(ZLIB_FOUND AND APPLE)
  target_link_libraries(compare_images z)
else(ZLIB_FOUND AND APPLE)
  target_link_libraries(compare_images ${Z_LIBNAME})
endif()

add_executable(compare_dump_files ${compare_dump_files_SRCS})

add_executable(compare_raw_files ${compare_raw_files_SRCS})

# No image send to the dashboard if lib PNG is not available.
if(NOT HAVE_LIBPNG)
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
    set(md5 "d41d8cd98f00b204e9800998ecf8427e")
    add_test(NAME TestJPIP1 COMMAND ${CMAKE_COMMAND} -DD_URL:STRING=${s} -DD_FILE:PATH=${p}
      -DEXPECTED_MD5=${md5} -P ${PROJECT_SOURCE_DIR}/cmake/JPIPTestDriver.cmake)
  endif()
endif()

add_executable(ppm2rgb3 ppm2rgb3.c)
#add_executable(pdf2jp2 pdf2jp2.c)
