# Main CMakeLists.txt to build the OpenJPEG project using CMake (www.cmake.org)
# Written by Mathieu Malaterre

# This CMake project will by default create a library called openjpeg
# But if you want to use this project within your own (CMake) project
# you will eventually like to prefix the library to avoid linking confusion
# For this purpose you can define a CMake var: OPJ_PREFIX to whatever you like
# e.g.:
# SET(OPJ_PREFIX "gdcm")
# Also if you want to test (CTest driven) you need to define :
# OPJ_STANDALONE to 1, e.g
# OPJ_STANDALONE:BOOL=1
PROJECT(OPENJPEG C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.0)

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

#-----------------------------------------------------------------------------
# Always build the library
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

