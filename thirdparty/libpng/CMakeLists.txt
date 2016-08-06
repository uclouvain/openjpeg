# CMakeLists.txt

# Copyright (C) 2007-2015 Glenn Randers-Pehrson

# This code is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h

#cmake_minimum_required(VERSION 2.4.4)
#set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

#set(CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo")

project(libpng C)

# Use the prebuilt pnglibconf.h file from the scripts folder
# TODO: fix this by building with awk; without this no cmake build can be
# configured directly (to do so indirectly use your local awk to build a
# pnglibconf.h in the build directory.)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpng/scripts/pnglibconf.h.prebuilt
               ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpng/png.h ${CMAKE_CURRENT_BINARY_DIR}/png.h COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngconf.h ${CMAKE_CURRENT_BINARY_DIR}/pngconf.h COPYONLY)


# OUR SOURCES
set(libpng_public_hdrs
  libpng/png.h
  libpng/pngconf.h
  ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h
)
set(libpng_sources
  ${libpng_public_hdrs}
  libpng/pngdebug.h
  libpng/pnginfo.h
  libpng/pngpriv.h
  libpng/pngstruct.h
  libpng/png.c
  libpng/pngerror.c
  libpng/pngget.c
  libpng/pngmem.c
  libpng/pngpread.c
  libpng/pngread.c
  libpng/pngrio.c
  libpng/pngrtran.c
  libpng/pngrutil.c
  libpng/pngset.c
  libpng/pngtrans.c
  libpng/pngwio.c
  libpng/pngwrite.c
  libpng/pngwtran.c
  libpng/pngwutil.c
)

# SOME NEEDED DEFINITIONS

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(MSVC)

# NOW BUILD OUR TARGET
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libpng ${CMAKE_CURRENT_BINARY_DIR}/../zlib)

# does not work without changing name
add_library(pngstatic STATIC ${libpng_sources})
target_link_libraries(pngstatic ${Z_LIBNAME} ${M_LIBRARY})

SET_TARGET_PROPERTIES(pngstatic
  PROPERTIES
  OUTPUT_NAME "pngstatic"
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/thirdparty/lib
)