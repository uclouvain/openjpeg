# Main CMakeLists.txt to build the OpenJPEG project using CMake (www.cmake.org)
# Written by Mathieu Malaterre

# This CMake project will by default create a library called openjpeg
# But if you want to use this project within your own (CMake) project
# you will eventually like to prefix the library to avoid linking confusion
# For this purpose you can define a CMake var: OPENJPEG_NAMESPACE to whatever you like
# e.g.:
# SET(OPENJPEG_NAMESPACE "GDCMOPENJPEG")
PROJECT(OPENJPEG C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.2)

IF(NOT OPENJPEG_NAMESPACE)
  SET(OPENJPEG_NAMESPACE "OPENJPEG")
  SET(OPENJPEG_STANDALONE 1)
ENDIF(NOT OPENJPEG_NAMESPACE)
# In all cases:
STRING(TOLOWER ${OPENJPEG_NAMESPACE} OPENJPEG_LIBRARY_NAME)

PROJECT(${JPEG_NAMESPACE} C)

# Do full dependency headers.
INCLUDE_REGULAR_EXPRESSION("^.*$")

#-----------------------------------------------------------------------------
# OPENJPEG version number, usefull for packaging and doxygen doc:
SET(OPENJPEG_MAJOR_VERSION 1)
SET(OPENJPEG_MINOR_VERSION 0)
SET(OPENJPEG_BUILD_VERSION 0)
SET(OPENJPEG_VERSION
  "${OPENJPEG_MAJOR_VERSION}.${OPENJPEG_MINOR_VERSION}.${OPENJPEG_BUILD_VERSION}")

#-----------------------------------------------------------------------------
# OpenJPEG build configuration options.
OPTION(BUILD_SHARED_LIBS "Build OpenJPEG with shared libraries." OFF)

#-----------------------------------------------------------------------------
# For the codec...
OPTION(BUILD_EXAMPLES "Build the Examples (codec...)." OFF)


# configure name mangling to allow multiple libraries to coexist
# peacefully
IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in)
SET(MANGLE_PREFIX ${OPENJPEG_LIBRARY_NAME})
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/openjpeg_mangle.h
               @ONLY IMMEDIATE)
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in)

#-----------------------------------------------------------------------------
# Always build the library
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
SUBDIRS(
  libopenjpeg
  )
#-----------------------------------------------------------------------------
# Build example only if requested
IF(BUILD_EXAMPLES)
  SUBDIRS(codec)
ENDIF(BUILD_EXAMPLES)

#-----------------------------------------------------------------------------
# For openjpeg team if they ever want Dart+CMake
IF(OPJ_STANDALONE)
  INCLUDE(Dart)
  MARK_AS_ADVANCED(BUILD_TESTING DART_ROOT TCL_TCLSH)
  IF(BUILD_TESTING)
    ENABLE_TESTING()
    SET(BUILDNAME "OpenJPEG-${CMAKE_SYSTEM}-${CMAKE_C_COMPILER}" CACHE STRING "Name of build on the dashboard")
    MARK_AS_ADVANCED(BUILDNAME)
  ENDIF(BUILD_TESTING)
ENDIF(OPJ_STANDALONE)

# TODO, technically we should add tests, e.g:
# http://www.crc.ricoh.com/~gormish/jpeg2000conformance/

