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
SET(OPENJPEG_LIBRARY_PROPERTIES
  VERSION   "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}"
  SOVERSION "${OPENJPEG_VERSION_MAJOR}"
)

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
SET(OPENJPEG_INSTALL_SUBDIR "${projectname}-${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}")

IF(NOT OPENJPEG_INSTALL_BIN_DIR)
  SET(OPENJPEG_INSTALL_BIN_DIR "bin")
ENDIF(NOT OPENJPEG_INSTALL_BIN_DIR)

IF(NOT OPENJPEG_INSTALL_LIB_DIR)
  SET(OPENJPEG_INSTALL_LIB_DIR "lib")
ENDIF(NOT OPENJPEG_INSTALL_LIB_DIR)

IF(NOT OPENJPEG_INSTALL_DATA_DIR)
  SET(OPENJPEG_INSTALL_DATA_DIR "share/${OPENJPEG_INSTALL_SUBDIR}")
ENDIF(NOT OPENJPEG_INSTALL_DATA_DIR)

IF(NOT OPENJPEG_INSTALL_INCLUDE_DIR)
  SET(OPENJPEG_INSTALL_INCLUDE_DIR "include/")
ENDIF(NOT OPENJPEG_INSTALL_INCLUDE_DIR)

IF(NOT OPENJPEG_INSTALL_MAN_DIR)
  SET(OPENJPEG_INSTALL_MAN_DIR "share/man/")
ENDIF(NOT OPENJPEG_INSTALL_MAN_DIR)

IF(NOT OPENJPEG_INSTALL_DOC_DIR)
  SET(OPENJPEG_INSTALL_DOC_DIR "share/doc/${OPENJPEG_INSTALL_SUBDIR}")
ENDIF(NOT OPENJPEG_INSTALL_DOC_DIR)

IF(NOT OPENJPEG_INSTALL_PACKAGE_DIR)
  SET(OPENJPEG_INSTALL_PACKAGE_DIR ${OPENJPEG_INSTALL_LIB_DIR}/${OPENJPEG_INSTALL_SUBDIR}
    CACHE INTERNAL "")
ENDIF(NOT OPENJPEG_INSTALL_PACKAGE_DIR)

#-----------------------------------------------------------------------------
# Test for some required system information.
INCLUDE (${CMAKE_ROOT}/Modules/CMakeBackwardCompatibilityC.cmake)

#-----------------------------------------------------------------------------
# Test for getopt being available in this system
INCLUDE (${OPENJPEG_SOURCE_DIR}/CMake/CheckHaveGetopt.cmake )

#-----------------------------------------------------------------------------
# Setup file for setting custom ctest vars
CONFIGURE_FILE(
  ${CMAKE_SOURCE_DIR}/testing/CTestCustom.cmake.in
  ${CMAKE_BINARY_DIR}/testing/CTestCustom.cmake
  @ONLY
  )

#-----------------------------------------------------------------------------
# OpenJPEG build configuration options.
OPTION(BUILD_SHARED_LIBS "Build OpenJPEG shared library and link executables against it." ON)
SET (EXECUTABLE_OUTPUT_PATH ${OPENJPEG_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")
SET (LIBRARY_OUTPUT_PATH ${OPENJPEG_BINARY_DIR}/bin CACHE PATH "Single output directory for building all libraries.")
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

#-----------------------------------------------------------------------------
# configure name mangling to allow multiple libraries to coexist
# peacefully
IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in)
SET(MANGLE_PREFIX ${OPENJPEG_LIBRARY_NAME})
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/openjpeg_mangle.h
               @ONLY IMMEDIATE)
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in)

#-----------------------------------------------------------------------------
# pkgconfig support
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libopenjpeg1.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libopenjpeg1.pc @ONLY)
INSTALL( FILES  ${CMAKE_CURRENT_BINARY_DIR}/libopenjpeg1.pc DESTINATION ${OPENJPEG_INSTALL_LIB_DIR}/pkgconfig )
INSTALL( CODE
  "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink \"libopenjpeg1.pc\" \"\$ENV{DESTDIR}${OPENJPEG_INSTALL_LIB_DIR}/pkgconfig/libopenjpeg.pc\")")

#-----------------------------------------------------------------------------
# Compiler specific flags:
IF(CMAKE_COMPILER_IS_GNUCC)
  # For all builds, make sure openjpeg is std99 compliant:
  # SET(CMAKE_C_FLAGS "-Wall -std=c99 ${CMAKE_C_FLAGS}") # FIXME: this setting prevented us from setting a coverage build.
  # Do not use ffast-math for all build, it would produce incorrect results, only set for release:
  SET(CMAKE_C_FLAGS_RELEASE "-ffast-math ${CMAKE_C_FLAGS_RELEASE}")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

#-----------------------------------------------------------------------------
# opj_config.h generation
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
CONFIGURE_FILE("${OPENJPEG_SOURCE_DIR}/opj_configh.cmake.in"
 "${OPENJPEG_BINARY_DIR}/opj_config.h"
 @ONLY
 )

#-----------------------------------------------------------------------------
# Build Library
INCLUDE_DIRECTORIES(BEFORE ${OPENJPEG_BINARY_DIR})
ADD_SUBDIRECTORY(libopenjpeg)

#-----------------------------------------------------------------------------
# Build Applications
OPTION(BUILD_CODEC "Build the CODEC executables" ON)
OPTION(BUILD_MJ2 "Build the MJ2 executables." OFF)
ADD_SUBDIRECTORY(applications)

#-----------------------------------------------------------------------------
# Build JPWL-flavoured library and executables
OPTION(BUILD_JPWL "Build the JPWL library and executables" OFF)

#-----------------------------------------------------------------------------
# Build DOCUMENTATION (not in ALL target and only if Doxygen is found)
ADD_SUBDIRECTORY(doc)

#-----------------------------------------------------------------------------
# CDash+CMake : to be updated
OPTION(BUILD_TESTING "Build the tests." OFF)
IF(BUILD_TESTING)
  ENABLE_TESTING()
  INCLUDE(CTest)
ENDIF(BUILD_TESTING)
# Adding test with dataset from:
# http://www.crc.ricoh.com/~gormish/jpeg2000conformance/
# -> wget http://www.crc.ricoh.com/~gormish/jpeg2000conformance/j2kp4files_v1_5.zip
# http://www.jpeg.org/jpeg2000guide/testimages/testimages.html
# Adding JPEG2000_CONFORMANCE_DATA_ROOT
FIND_PATH(JPEG2000_CONFORMANCE_DATA_ROOT testimages.html
  ${OPENJPEG_SOURCE_DIR}/../jpeg2000testimages
  $ENV{JPEG2000_CONFORMANCE_DATA_ROOT}
)

#-----------------------------------------------------------------------------
# install all targets referenced as OPENJPEGTargets
INSTALL(EXPORT OpenJPEGTargets DESTINATION ${OPENJPEG_INSTALL_PACKAGE_DIR})
CONFIGURE_FILE( ${OPENJPEG_SOURCE_DIR}/CMake/OpenJPEGConfig.cmake.in
  ${OPENJPEG_BINARY_DIR}/OpenJPEGConfig.cmake
  @ONLY
)
INSTALL( FILES ${OPENJPEG_BINARY_DIR}/OpenJPEGConfig.cmake
  DESTINATION ${OPENJPEG_INSTALL_PACKAGE_DIR}
)

#-----------------------------------------------------------------------------
# install CHANGES and LICENSE
INSTALL(
  FILES       CHANGES 
              LICENSE
  DESTINATION ${OPENJPEG_INSTALL_DOC_DIR})
