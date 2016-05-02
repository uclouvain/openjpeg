project(libpng C)

file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/libpng/png.h ${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngconf.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpng/scripts/pnglibconf.h.prebuilt ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/libpng"
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../libz
)

file(GLOB SRCS libpng/*.c)
file(GLOB HDRS libpng/*.h)

#
set(LIBTARGET "png")
#
add_library(${LIBTARGET} STATIC ${SRCS} ${HDRS})
#
if(MSVC)
  set_target_properties(${LIBTARGET} PROPERTIES PREFIX "lib")
endif(MSVC)
#
target_link_libraries(${LIBTARGET} ${Z_LIBNAME} ${M_LIBRARY})
#
set_target_properties(${LIBTARGET}
	PROPERTIES
    OUTPUT_NAME "${LIBTARGET}"
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/thirdparty/lib)
#
