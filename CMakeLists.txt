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
OPTION(BUILD_SHARED_LIBS "Build OpenJPEG with shared libraries." OFF)

#-----------------------------------------------------------------------------
SET (EXECUTABLE_OUTPUT_PATH ${OPENJPEG_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")
SET (LIBRARY_OUTPUT_PATH ${OPENJPEG_BINARY_DIR}/bin CACHE PATH "Single output directory for building all libraries.")
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)


#-----------------------------------------------------------------------------
# For the codec...
OPTION(BUILD_EXAMPLES "Build the Examples (codec...)." ON)


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
CONFIGURE_FILE(
   ${PROJECT_SOURCE_DIR}/openjpegConfigure.h.in
   ${PROJECT_BINARY_DIR}/openjpegConfigure.h)

#-----------------------------------------------------------------------------
# Always build the library
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
SUBDIRS(
  libopenjpeg
  mj2
  jpwl
  )

IF(NOT UNIX)
SUBDIRS(
  jp3d
  indexer_JPIP
  )
ENDIF(NOT UNIX)

#-----------------------------------------------------------------------------
# Build example only if requested
IF(BUILD_EXAMPLES)
  SUBDIRS(codec)
ENDIF(BUILD_EXAMPLES)

#-----------------------------------------------------------------------------
# For the documentation
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

