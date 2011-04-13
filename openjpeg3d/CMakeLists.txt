# Main CMakeLists.txt to build the OpenJPEG3D 3D project using CMake (www.cmake.org)
# Written by Mathieu Malaterre

# This CMake project will by default create a library called openjpeg3d3d
# But if you want to use this project within your own (CMake) project
# you will eventually like to prefix the library to avoid linking confusion
# For this purpose you can define a CMake var: OPENJPEG3D_NAMESPACE to whatever you like
# e.g.:
# SET(OPENJPEG3D_NAMESPACE "GDCMOPENJPEG3D")
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(NOT OPENJPEG3D_NAMESPACE)
  SET(OPENJPEG3D_NAMESPACE "OPENJPEG3D")
  SET(OPENJPEG3D_STANDALONE 1)
ENDIF(NOT OPENJPEG3D_NAMESPACE)
# In all cases:
STRING(TOLOWER ${OPENJPEG3D_NAMESPACE} OPENJPEG3D_LIBRARY_NAME)

PROJECT(${OPENJPEG3D_NAMESPACE} C)

# Do full dependency headers.
INCLUDE_REGULAR_EXPRESSION("^.*$")

#-----------------------------------------------------------------------------
# OPENJPEG3D version number, useful for packaging and doxygen doc:
SET(OPENJPEG3D_VERSION_MAJOR 1)
SET(OPENJPEG3D_VERSION_MINOR 3)
SET(OPENJPEG3D_VERSION_BUILD 0)
SET(OPENJPEG3D_VERSION
  "${OPENJPEG3D_VERSION_MAJOR}.${OPENJPEG3D_VERSION_MINOR}.${OPENJPEG3D_VERSION_BUILD}")
SET(PACKAGE_VERSION
  "${OPENJPEG3D_VERSION_MAJOR}.${OPENJPEG3D_VERSION_MINOR}.${OPENJPEG3D_VERSION_BUILD}")
SET(OPENJPEG3D_LIBRARY_PROPERTIES
  VERSION   "${OPENJPEG3D_VERSION_MAJOR}.${OPENJPEG3D_VERSION_MINOR}.${OPENJPEG3D_VERSION_BUILD}"
  SOVERSION "${OPENJPEG3D_VERSION_MAJOR}"
)
# You will also need to define a value for the following variables:
# OPENJPEG3D_INSTALL_BIN_DIR          - binary dir (executables)
# OPENJPEG3D_INSTALL_LIB_DIR          - library dir (libs)
# OPENJPEG3D_INSTALL_DATA_DIR         - share dir (say, examples, data, etc)
# OPENJPEG3D_INSTALL_INCLUDE_DIR      - include dir (headers)


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
SET(subdir "${projectname}-${OPENJPEG3D_VERSION_MAJOR}.${OPENJPEG3D_VERSION_MINOR}")

IF(NOT OPENJPEG3D_INSTALL_BIN_DIR)
  SET(OPENJPEG3D_INSTALL_BIN_DIR "bin")
ENDIF(NOT OPENJPEG3D_INSTALL_BIN_DIR)

IF(NOT OPENJPEG3D_INSTALL_LIB_DIR)
  SET(OPENJPEG3D_INSTALL_LIB_DIR "lib")
ENDIF(NOT OPENJPEG3D_INSTALL_LIB_DIR)

IF(NOT OPENJPEG3D_INSTALL_DATA_DIR)
  SET(OPENJPEG3D_INSTALL_DATA_DIR "share/${subdir}")
ENDIF(NOT OPENJPEG3D_INSTALL_DATA_DIR)

IF(NOT OPENJPEG3D_INSTALL_INCLUDE_DIR)
  SET(OPENJPEG3D_INSTALL_INCLUDE_DIR "include/")
ENDIF(NOT OPENJPEG3D_INSTALL_INCLUDE_DIR)

IF(NOT OPENJPEG3D_INSTALL_MAN_DIR)
  SET(OPENJPEG3D_INSTALL_MAN_DIR "share/man/")
ENDIF(NOT OPENJPEG3D_INSTALL_MAN_DIR)

IF(NOT OPENJPEG3D_INSTALL_DOC_DIR)
  SET(OPENJPEG3D_INSTALL_DOC_DIR "share/doc/${subdir}")
ENDIF(NOT OPENJPEG3D_INSTALL_DOC_DIR)

IF(NOT OPENJPEG3D_INSTALL_PACKAGE_DIR)
  SET(OPENJPEG3D_INSTALL_PACKAGE_DIR ${OPENJPEG3D_INSTALL_LIB_DIR}/${subdir}
    CACHE INTERNAL "")
ENDIF(NOT OPENJPEG3D_INSTALL_PACKAGE_DIR)

#-----------------------------------------------------------------------------
# Test for some required system information.
INCLUDE (${CMAKE_ROOT}/Modules/CMakeBackwardCompatibilityC.cmake)

#-----------------------------------------------------------------------------
# Test for getopt being available in this system
INCLUDE (${PROJECT_SOURCE_DIR}/CMake/CheckHaveGetopt.cmake )

#-----------------------------------------------------------------------------
# OpenJPEG3D build configuration options.
OPTION(BUILD_SHARED_LIBS "Build OpenJPEG3D shared library and link executables against it." ON)

#-----------------------------------------------------------------------------
SET (EXECUTABLE_OUTPUT_PATH ${OPENJPEG3D_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")
SET (LIBRARY_OUTPUT_PATH ${OPENJPEG3D_BINARY_DIR}/bin CACHE PATH "Single output directory for building all libraries.")
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)


# configure name mangling to allow multiple libraries to coexist
# peacefully
IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg3d_mangle.h.in)
SET(MANGLE_PREFIX ${OPENJPEG3D_LIBRARY_NAME})
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/openjpeg3d_mangle.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/openjpeg3d_mangle.h
               @ONLY IMMEDIATE)
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg3d_mangle.h.in)

#-----------------------------------------------------------------------------
# pkgconfig support
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libopenjpeg3d.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libopenjpeg3d.pc @ONLY)
INSTALL( FILES  ${CMAKE_CURRENT_BINARY_DIR}/libopenjpeg3d.pc DESTINATION ${OPENJPEG3D_INSTALL_LIB_DIR}/pkgconfig )

#-----------------------------------------------------------------------------
# Always build the library
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
SUBDIRS(libopenjpeg3d)

#-----------------------------------------------------------------------------
# Compiler specific flags:
IF(CMAKE_COMPILER_IS_GNUCC)
  # For all builds, make sure openjpeg3d is std99 compliant:
  # SET(CMAKE_C_FLAGS "-Wall -std=c99 ${CMAKE_C_FLAGS}") # FIXME: this setting prevented us from setting a coverage build.
  # Do not use ffast-math for all build, it would produce incorrect results, only set for release:
  SET(CMAKE_C_FLAGS_RELEASE "-ffast-math ${CMAKE_C_FLAGS_RELEASE}")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# install all targets referenced as OPENJPEG3DTargets
install(EXPORT OpenJPEG3DTargets DESTINATION ${OPENJPEG3D_INSTALL_PACKAGE_DIR})
CONFIGURE_FILE( ${OPENJPEG3D_SOURCE_DIR}/CMake/OpenJPEG3DConfig.cmake.in
  ${OPENJPEG3D_BINARY_DIR}/OpenJPEG3DConfig.cmake
  @ONLY
)
INSTALL( FILES ${OPENJPEG3D_BINARY_DIR}/OpenJPEG3DConfig.cmake
  DESTINATION ${OPENJPEG3D_INSTALL_PACKAGE_DIR}
)
# install CHANGES and LICENSE
INSTALL(
  FILES       LICENSE
  DESTINATION ${OPENJPEG3D_INSTALL_DOC_DIR})
#
FIND_FILE(HAVE_STRINGS_H_FOUND strings.h)
IF(NOT HAVE_STRINGS_H_FOUND STREQUAL "HAVE_STRINGS_H_FOUND-NOTFOUND")
  FIND_FILE(HAVE_STRINGS_H strings.h)
  SET(HAS_STRINGS_H 1)
ENDIF()
FIND_FILE(HAVE_INTTYPES_H_FOUND inttypes.h)
IF(NOT HAVE_INTTYPES_H_FOUND STREQUAL "HAVE_INTTYPES_H_FOUND-NOTFOUND")
  FIND_FILE(HAVE_INTTYPES_H inttypes.h)
  SET(HAS_INTTYPES_H 1)
ENDIF()
FIND_FILE(HAVE_MEMORY_H_FOUND memory.h)
IF(NOT HAVE_MEMORY_H_FOUND STREQUAL "HAVE_MEMORY_H_FOUND-NOTFOUND")
  FIND_FILE(HAVE_MEMORY_H memory.h)
  SET(HAS_MEMORY_H 1)
ENDIF()
FIND_FILE(HAVE_STDINT_H_FOUND stdint.h)
IF(NOT HAVE_STDINT_H_FOUND STREQUAL "HAVE_STDINT_H_FOUND-NOTFOUND")
  FIND_FILE(HAVE_STDINT_H stdint.h)
  SET(HAS_STDINT_H 1)
ENDIF()
FIND_FILE(HAVE_STDLIB_H_FOUND stdlib.h)
IF(NOT HAVE_STDLIB_H_FOUND STREQUAL "HAVE_STDLIB_H_FOUND-NOTFOUND")
  FIND_FILE(HAVE_STDLIB_H stdlib.h)
  SET(HAS_STDLIB_H 1)
ENDIF()
FIND_FILE(HAVE_STRING_H_FOUND string.h)
IF(NOT HAVE_STRING_H_FOUND STREQUAL "HAVE_STRING_H_FOUND-NOTFOUND")
  FIND_FILE(HAVE_STRING_H string.h)
  SET(HAS_STRING_H 1)
ENDIF()
FIND_FILE(HAVE_SYS_STAT_H_FOUND sys/stat.h)
IF(NOT HAVE_SYS_STAT_H_FOUND STREQUAL "HAVE_SYS_STAT_H_FOUND-NOTFOUND")
  FIND_FILE(HAVE_SYS_STAT_H sys/stat.h)
  SET(HAS_SYS_STAT_H 1)
ENDIF()
FIND_FILE(HAVE_SYS_TYPES_H_FOUND sys/types.h)
IF(NOT HAVE_SYS_TYPES_H_FOUND STREQUAL "HAVE_SYS_TYPES_H_FOUND-NOTFOUND")
  FIND_FILE(HAVE_SYS_TYPES_H sys/types.h)
  SET(HAS_SYS_TYPES_H 1)
ENDIF()
FIND_FILE(HAVE_UNISTD_H_FOUND unistd.h)
IF(NOT HAVE_UNISTD_H_FOUND STREQUAL "HAVE_UNISTD_H_FOUND-NOTFOUND")
  FIND_FILE(HAVE_UNISTD_H unistd.h)
  SET(HAS_UNISTD_H 1)
ENDIF()


# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/codec
  ${CMAKE_CURRENT_SOURCE_DIR}/libopenjpeg3d
  )


SUBDIRS(codec)
