PROJECT(OPENJPEG C)

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
  ENDIF(BUILD_TESTING)
ENDIF(OPJ_STANDALONE)
# TODO, technically we should add tests, e.g:
# http://www.crc.ricoh.com/~gormish/jpeg2000conformance/

