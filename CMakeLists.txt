# Main CMakeLists.txt to build the OpenJPEG project using CMake (www.cmake.org)
# Written by Mathieu Malaterre

# This CMake project will by default create a library called openjpeg
# But if you want to use this project within your own (CMake) project
# you will eventually like to prefix the library to avoid linking confusion
# For this purpose you can define a CMake var: OPENJPEG_NAMESPACE to whatever you like
# e.g.:
# SET(OPENJPEG_NAMESPACE "GDCMOPENJPEG")
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(NOT OPENJPEG_NAMESPACE)
  SET(OPENJPEG_NAMESPACE "OPENJPEG")
  SET(OPENJPEG_STANDALONE 1)
ENDIF(NOT OPENJPEG_NAMESPACE)
# In all cases:
STRING(TOLOWER ${OPENJPEG_NAMESPACE} OPENJPEG_LIBRARY_NAME)

PROJECT(${OPENJPEG_NAMESPACE} C)

# Do full dependency headers.
INCLUDE_REGULAR_EXPRESSION("^.*$")

#-----------------------------------------------------------------------------
# OPENJPEG version number, useful for packaging and doxygen doc:
SET(OPENJPEG_VERSION_MAJOR 1)
SET(OPENJPEG_VERSION_MINOR 4)
SET(OPENJPEG_VERSION_BUILD 0)
SET(OPENJPEG_VERSION
  "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}")
SET(PACKAGE_VERSION
  "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}")
# This setting of SOVERSION assumes that any API change
# will increment either the minor or major version number of openjpeg
SET(OPENJPEG_LIBRARY_PROPERTIES
  VERSION   "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}"
  SOVERSION "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}"
)
# You will also need to define a value for the following variables:
# OPENJPEG_INSTALL_BIN_DIR          - binary dir (executables)
# OPENJPEG_INSTALL_LIB_DIR          - library dir (libs)
# OPENJPEG_INSTALL_DATA_DIR         - share dir (say, examples, data, etc)
# OPENJPEG_INSTALL_INCLUDE_DIR      - include dir (headers)


# On Visual Studio 8 MS deprecated C. This removes all 1.276E1265 security
# warnings
IF(WIN32)
  IF(NOT BORLAND)
    IF(NOT CYGWIN)
      IF(NOT MINGW)
        IF(NOT ITK_ENABLE_VISUAL_STUDIO_DEPRECATED_C_WARNINGS)
          ADD_DEFINITIONS(
            -D_CRT_FAR_MAPPINGS_NO_DEPRECATE
            -D_CRT_IS_WCTYPE_NO_DEPRECATE
            -D_CRT_MANAGED_FP_NO_DEPRECATE
            -D_CRT_NONSTDC_NO_DEPRECATE
            -D_CRT_SECURE_NO_DEPRECATE
            -D_CRT_SECURE_NO_DEPRECATE_GLOBALS
            -D_CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE
            -D_CRT_TIME_FUNCTIONS_NO_DEPRECATE
            -D_CRT_VCCLRIT_NO_DEPRECATE
            -D_SCL_SECURE_NO_DEPRECATE
            )
        ENDIF(NOT ITK_ENABLE_VISUAL_STUDIO_DEPRECATED_C_WARNINGS)
      ENDIF(NOT MINGW)
    ENDIF(NOT CYGWIN)
  ENDIF(NOT BORLAND)
ENDIF(WIN32)


# --------------------------------------------------------------------------
# Install directories

STRING(TOLOWER ${PROJECT_NAME} projectname)
SET(subdir "${projectname}-${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}")

IF(NOT OPENJPEG_INSTALL_BIN_DIR)
  SET(OPENJPEG_INSTALL_BIN_DIR "bin")
ENDIF(NOT OPENJPEG_INSTALL_BIN_DIR)

IF(NOT OPENJPEG_INSTALL_LIB_DIR)
  SET(OPENJPEG_INSTALL_LIB_DIR "lib")
ENDIF(NOT OPENJPEG_INSTALL_LIB_DIR)

IF(NOT OPENJPEG_INSTALL_DATA_DIR)
  SET(OPENJPEG_INSTALL_DATA_DIR "share/${subdir}")
ENDIF(NOT OPENJPEG_INSTALL_DATA_DIR)

IF(NOT OPENJPEG_INSTALL_INCLUDE_DIR)
  SET(OPENJPEG_INSTALL_INCLUDE_DIR "include/${subdir}")
ENDIF(NOT OPENJPEG_INSTALL_INCLUDE_DIR)

IF(NOT OPENJPEG_INSTALL_MAN_DIR)
  SET(OPENJPEG_INSTALL_MAN_DIR "share/man/")
ENDIF(NOT OPENJPEG_INSTALL_MAN_DIR)

IF(NOT OPENJPEG_INSTALL_DOC_DIR)
  SET(OPENJPEG_INSTALL_DOC_DIR "share/doc/${subdir}")
ENDIF(NOT OPENJPEG_INSTALL_DOC_DIR)

IF(NOT OPENJPEG_INSTALL_PACKAGE_DIR)
  SET(OPENJPEG_INSTALL_PACKAGE_DIR ${OPENJPEG_INSTALL_LIB_DIR}/${subdir}
    CACHE INTERNAL "")
ENDIF(NOT OPENJPEG_INSTALL_PACKAGE_DIR)

#-----------------------------------------------------------------------------
# Test for some required system information.
INCLUDE (${CMAKE_ROOT}/Modules/CMakeBackwardCompatibilityC.cmake)

#-----------------------------------------------------------------------------
# Test for getopt being available in this system
INCLUDE (${PROJECT_SOURCE_DIR}/CMake/CheckHaveGetopt.cmake )

#-----------------------------------------------------------------------------
# Setup file for setting custom ctest vars
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/CTestCustom.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake
  @ONLY
  )

#-----------------------------------------------------------------------------
# OpenJPEG build configuration options.
OPTION(BUILD_SHARED_LIBS "Build OpenJPEG with shared libraries." ON)

#-----------------------------------------------------------------------------
SET (EXECUTABLE_OUTPUT_PATH ${OPENJPEG_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")
SET (LIBRARY_OUTPUT_PATH ${OPENJPEG_BINARY_DIR}/bin CACHE PATH "Single output directory for building all libraries.")
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)


# configure name mangling to allow multiple libraries to coexist
# peacefully
IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in)
SET(MANGLE_PREFIX ${OPENJPEG_LIBRARY_NAME})
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/openjpeg_mangle.h
               @ONLY IMMEDIATE)
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in)

#-----------------------------------------------------------------------------
# Translate CMake configuration options into #defines that can be queried from
# the code.
#CONFIGURE_FILE(
#   ${PROJECT_SOURCE_DIR}/opj_configh.cmake.in
#   ${PROJECT_BINARY_DIR}/opj_config.h)

#-----------------------------------------------------------------------------
# Always build the library
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
SUBDIRS(
  libopenjpeg
  mj2
  )

IF(NOT UNIX)
SUBDIRS(
  indexer_JPIP
  )
ENDIF(NOT UNIX)

#-----------------------------------------------------------------------------
# Build example codec ?
OPTION(BUILD_EXAMPLES "Build the Examples (codec...)." ON)
IF(BUILD_EXAMPLES)
  SUBDIRS(codec)
ENDIF(BUILD_EXAMPLES)

#-----------------------------------------------------------------------------
# Build JPWL binaries ?
OPTION(BUILD_JPWL "Build the JPWL binaries" OFF)
IF(BUILD_JPWL)
  SUBDIRS(jpwl)
ENDIF(BUILD_JPWL)

#-----------------------------------------------------------------------------
# Build JP3D binaries ?
OPTION(BUILD_JP3D "Build the JP3D binaries" OFF)
IF(BUILD_JP3D)
  SUBDIRS(jp3d)
ENDIF(BUILD_JP3D)

#-----------------------------------------------------------------------------
# Build documentation ?
OPTION(BUILD_DOCUMENTATION "Build the doxygen documentation" OFF)
IF(BUILD_DOCUMENTATION)
  SUBDIRS(doc)
ENDIF(BUILD_DOCUMENTATION)

#-----------------------------------------------------------------------------
# For openjpeg team if they ever want CDash+CMake
OPTION(BUILD_TESTING "Build the tests." OFF)
IF(BUILD_TESTING)
  ENABLE_TESTING()
  INCLUDE(CTest)
ENDIF(BUILD_TESTING)

# Adding test with dataset from:
# http://www.crc.ricoh.com/~gormish/jpeg2000conformance/
# -> wget http://www.crc.ricoh.com/~gormish/jpeg2000conformance/j2kp4files_v1_5.zip
# http://www.jpeg.org/jpeg2000guide/testimages/testimages.html
#-----------------------------------------------------------------------------
# Adding JPEG2000_CONFORMANCE_DATA_ROOT
FIND_PATH(JPEG2000_CONFORMANCE_DATA_ROOT testimages.html
  ${OPENJPEG_SOURCE_DIR}/../jpeg2000testimages
  $ENV{JPEG2000_CONFORMANCE_DATA_ROOT}
)

#-----------------------------------------------------------------------------
# Compiler specific flags:
IF(CMAKE_COMPILER_IS_GNUCC)
  # For all builds, make sure openjpeg is std99 compliant:
  # SET(CMAKE_C_FLAGS "-Wall -std=c99 ${CMAKE_C_FLAGS}") # FIXME: this setting prevented us from setting a coverage build.
  # Do not use ffast-math for all build, it would produce incorrect results, only set for release:
  SET(CMAKE_C_FLAGS_RELEASE "-ffast-math ${CMAKE_C_FLAGS_RELEASE}")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# install all targets referenced as OPENJPEGTargets
install(EXPORT OpenJPEGTargets DESTINATION ${OPENJPEG_INSTALL_PACKAGE_DIR})
CONFIGURE_FILE( ${OPENJPEG_SOURCE_DIR}/CMake/OpenJPEGConfig.cmake.in
  ${OPENJPEG_BINARY_DIR}/OpenJPEGConfig.cmake
  @ONLY
)
INSTALL( FILES ${OPENJPEG_BINARY_DIR}/OpenJPEGConfig.cmake
  DESTINATION ${OPENJPEG_INSTALL_PACKAGE_DIR}
)
#
SET(CMAKE_PREFIX_PATH /usr /usr/local /opt /opt/local)
#
FIND_FILE(HAVE_STRINGS_H strings.h)
FIND_FILE(HAVE_INTTYPES_H inttypes.h)
FIND_FILE(HAVE_MEMORY_H memory.h)
FIND_FILE(HAVE_STDLIB_H stdlib.h)
FIND_FILE(HAVE_STRING_H string.h)
FIND_FILE(HAVE_SYS_STAT_H sys/stat.h)
FIND_FILE(HAVE_SYS_TYPES_H sys/types.h)
FIND_FILE(HAVE_UNISTD_H unistd.h)
#
# Does the system have png library installed ?
# FIND_PACKAGE not used because not robust enough
#
FIND_PATH(PNG_INCLUDE_DIR png.h PATHS /usr/include /usr/include/libpng14 /usr/include/libpng12 /usr/include/libpng /usr/local/include /usr/local/include/libpng14 /usr/local/include/libpng12 /usr/local/include/libpng /opt/include /opt/include/libpng14 /opt/include/libpng12 /opt/include/libpng /opt/local/include /opt/local/include/libpng14 /opt/local/include/libpng12 /opt/local/include/libpng)
#
IF(PNG_INCLUDE_DIR STREQUAL "PNG_INCLUDE_DIR-NOTFOUND")
  SET(HAVE_PNG_H 0)
ELSE()
  SET(HAVE_PNG_H 1)
ENDIF()
FIND_LIBRARY(PNG_LIBRARIES NAMES png14 png12 png )
IF(PNG_LIBRARIES STREQUAL "PNG_LIBRARIES-NOTFOUND")
  SET(HAVE_LIBPNG 0)
ELSE()
  SET(HAVE_LIBPNG 1)
ENDIF()
IF(HAVE_LIBPNG AND HAVE_PNG_H)
  SET(PNG_FOUND 1)
ELSE()
  SET(PNG_FOUND 0)
ENDIF()
#
# Does the system have tiff library installed ?
# FIND_PACKAGE not used because not robust enough
#
FIND_PATH(TIFF_INCLUDE_DIR tiff.h)
IF(TIFF_INCLUDE_DIR STREQUAL "TIFF_INCLUDE_DIR-NOTFOUND")
  SET(HAVE_TIFF_H 0)
ELSE()
  SET(HAVE_TIFF_H 1)
ENDIF()
FIND_LIBRARY(TIFF_LIBRARIES tiff)
IF(TIFF_LIBRARIES STREQUAL "TIFF_LIBRARIES-NOTFOUND")
  SET(HAVE_LIBTIFF 0)
ELSE()
  SET(HAVE_LIBTIFF 1)
ENDIF()
IF(HAVE_LIBTIFF AND HAVE_TIFF_H)
  SET(TIFF_FOUND 1)
ELSE()
  SET(TIFF_FOUND 0)
ENDIF()
#
# Does the system have lcms library installed ?
#
SET(LCMS_LIB "")
FIND_FILE(LCMS2_HEADER_FOUND lcms2.h)
#
IF(LCMS2_HEADER_FOUND STREQUAL "LCMS2_HEADER_FOUND-NOTFOUND")
 SET(LCMS2_HEADER_FOUND "")
ENDIF()
IF(LCMS2_HEADER_FOUND)
 FIND_PATH(LCMS_INCLUDE_DIR  lcms2.h)
 FIND_LIBRARY(HAVE_LIBLCMS2 lcms2)
 IF(HAVE_LIBLCMS2 STREQUAL "HAVE_LIBLCMS2-NOTFOUND")
  SET(HAVE_LIBLCMS2 "")
 ENDIF()
 IF(HAVE_LIBLCMS2)
  SET(LCMS_LIB "${HAVE_LIBLCMS2}")
  SET(HAVE_LCMS2_LIB 1)
  SET(HAVE_LCMS2_H 1)
 ENDIF()
ENDIF()
IF(NOT LCMS2_HEADER_FOUND)
 FIND_FILE(LCMS1_HEADER_FOUND lcms.h)
 IF(LCMS1_HEADER_FOUND STREQUAL "LCMS1_HEADER_FOUND-NOTFOUND")
  SET(LCMS1_HEADER_FOUND "")
 ENDIF()
 IF(LCMS1_HEADER_FOUND)
  FIND_PATH(LCMS_INCLUDE_DIR  lcms.h)
  FIND_LIBRARY(HAVE_LIBLCMS1 lcms)
  IF(HAVE_LIBLCMS1 STREQUAL "HAVE_LIBLCMS1-NOTFOUND")
   SET(HAVE_LIBLCMS1 "")
  ENDIF()
  IF(HAVE_LIBLCMS1)
   SET(LCMS_LIB "${HAVE_LIBLCMS1}")
   SET(HAVE_LCMS1_LIB 1)
   SET(HAVE_LCMS1_H 1)
  ENDIF()
 ENDIF()
ENDIF()
#
# generate opj_config.h
CONFIGURE_FILE("${OPENJPEG_SOURCE_DIR}/opj_configh.cmake.in"
  "${OPENJPEG_BINARY_DIR}/opj_config.h" 
)

