project(libpng C)

string(REPLACE "-Wconversion" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "-Wsign-conversion" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  ${OPENJPEG_SOURCE_DIR}/thirdparty/include
)

file(GLOB SRCS *.c)
file(GLOB HDRS *.h)
set(EXT_HDRS
 ${OPENJPEG_SOURCE_DIR}/thirdparty/include/zlib.h
 ${OPENJPEG_SOURCE_DIR}/thirdparty/include/zconf.h
)
#
set(LIBTARGET "png")
#
add_library(${LIBTARGET} STATIC ${SRCS} ${HDRS} ${EXT_HDRS})
#
if(MSVC)
  set_target_properties(${LIBTARGET} PROPERTIES PREFIX "lib")
endif(MSVC)
#
target_compile_definitions(${LIBTARGET} PRIVATE PNG_ARM_NEON_OPT=0)
target_link_libraries(${LIBTARGET} ${Z_LIBNAME} ${M_LIBRARY})
#
set_target_properties(${LIBTARGET}
	PROPERTIES
    OUTPUT_NAME "${LIBTARGET}"
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/thirdparty/lib)
#
