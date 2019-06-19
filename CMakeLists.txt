cmake_minimum_required(VERSION 3.9)

set(OPENJPEG_VERSION_MAJOR 2)
set(OPENJPEG_VERSION_MINOR 3)
set(OPENJPEG_VERSION_BUILD 1)
set(OPENJPEG_VERSION
  "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}")
set(PACKAGE_VERSION
  "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}")

if(NOT OPENJPEG_SOVERSION)
  set(OPENJPEG_SOVERSION 7)
endif(NOT OPENJPEG_SOVERSION)

set(OPENJPEG_LIBRARY_PROPERTIES
  VERSION   "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}"
  SOVERSION "${OPENJPEG_SOVERSION}"
)

if(NOT OPENJPEG_NAMESPACE)
  set(OPENJPEG_NAMESPACE "OPENJPEG")
  set(OPENJPEG_STANDALONE 1)
endif()
# In all cases:
#string(TOLOWER ${OPENJPEG_NAMESPACE} OPENJPEG_LIBRARY_NAME)
set(OPENJPEG_LIBRARY_NAME openjp2)

project(${OPENJPEG_NAMESPACE})

# Path to additional CMake modules
set(CMAKE_MODULE_PATH
    ${${OPENJPEG_NAMESPACE}_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

string(TOLOWER ${PROJECT_NAME} projectname)
set(OPENJPEG_INSTALL_SUBDIR "${projectname}-${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}")

if(NOT OPENJPEG_INSTALL_BIN_DIR)
  set(OPENJPEG_INSTALL_BIN_DIR "bin")
endif()

if(NOT OPENJPEG_INSTALL_LIB_DIR)
  set(OPENJPEG_INSTALL_LIB_DIR "lib")
endif()

if(NOT OPENJPEG_INSTALL_SHARE_DIR)
  set(OPENJPEG_INSTALL_SHARE_DIR "share")
endif()

if(NOT OPENJPEG_INSTALL_DATA_DIR)
  set(OPENJPEG_INSTALL_DATA_DIR "${OPENJPEG_INSTALL_SHARE_DIR}/${OPENJPEG_INSTALL_SUBDIR}")
endif()

if(NOT OPENJPEG_INSTALL_INCLUDE_DIR)
  set(OPENJPEG_INSTALL_INCLUDE_DIR "include/${OPENJPEG_INSTALL_SUBDIR}")
endif()

if(BUILD_DOC)
if(NOT OPENJPEG_INSTALL_MAN_DIR)
  set(OPENJPEG_INSTALL_MAN_DIR "share/man/")
endif()

if(NOT OPENJPEG_INSTALL_DOC_DIR)
  set(OPENJPEG_INSTALL_DOC_DIR "share/doc/${OPENJPEG_INSTALL_SUBDIR}")
endif()
endif()

if(NOT OPENJPEG_INSTALL_PACKAGE_DIR)
  set(OPENJPEG_INSTALL_PACKAGE_DIR "${OPENJPEG_INSTALL_LIB_DIR}/${OPENJPEG_INSTALL_SUBDIR}")
endif()


configure_file(
  ${${OPENJPEG_NAMESPACE}_SOURCE_DIR}/cmake/CTestCustom.cmake.in
  ${${OPENJPEG_NAMESPACE}_SOURCE_DIR}/CTestCustom.cmake
  @ONLY
  )

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in)
  set(MANGLE_PREFIX ${OPENJPEG_LIBRARY_NAME})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h
               @ONLY)
endif()

include(EnsureFileInclude)

ensure_file_include("string.h"   HAVE_STRING_H YES)
ensure_file_include("memory.h"   HAVE_MEMORY_H YES)
ensure_file_include("stdlib.h"   HAVE_STDLIB_H YES)
ensure_file_include("stdio.h"    HAVE_STDIO_H  YES)
ensure_file_include("math.h"     HAVE_MATH_H   YES)
ensure_file_include("float.h"    HAVE_FLOAT_H  YES)
ensure_file_include("time.h"     HAVE_TIME_H   YES)
ensure_file_include("stdarg.h"   HAVE_STDARG_H YES)
ensure_file_include("ctype.h"    HAVE_CTYPE_H  YES)
ensure_file_include("assert.h"   HAVE_ASSERT_H YES)

# For the following files, we provide an alternative, they are not mandatory
ensure_file_include("stdint.h"   OPJ_HAVE_STDINT_H   NO)
ensure_file_include("inttypes.h" OPJ_HAVE_INTTYPES_H NO)

# why check this one ? for openjpip ?
include (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
CHECK_INCLUDE_FILE("strings.h"      HAVE_STRINGS_H)
CHECK_INCLUDE_FILE("sys/stat.h"     HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/types.h"    HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE("unistd.h"       HAVE_UNISTD_H)

add_subdirectory(src/lib)
option(BUILD_LUTS_GENERATOR "Build utility to generate t1_luts.h" OFF)
option(BUILD_UNIT_TESTS "Build unit tests (bench_dwt, test_sparse_array, etc..)" OFF)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/openjp2/opj_config.h.cmake.in
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/openjp2/opj_config.h
  @ONLY
 )

 configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/openjp2/opj_config_private.h.cmake.in
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/openjp2/opj_config_private.h
  @ONLY
 )
