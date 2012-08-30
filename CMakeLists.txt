# Main CMakeLists.txt to build the OpenJPEG project using CMake (www.cmake.org)
# Written by Mathieu Malaterre

# This CMake project will by default create a library called openjpeg
# But if you want to use this project within your own (CMake) project
# you will eventually like to prefix the library to avoid linking confusion
# For this purpose you can define a CMake var: OPENJPEG_NAMESPACE to whatever you like
# e.g.:
# set(OPENJPEG_NAMESPACE "GDCMOPENJPEG")
cmake_minimum_required(VERSION 2.6)

if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)

if(NOT OPENJPEG_NAMESPACE)
  set(OPENJPEG_NAMESPACE "OPENJPEG")
  set(OPENJPEG_STANDALONE 1)
endif(NOT OPENJPEG_NAMESPACE)
# In all cases:
string(TOLOWER ${OPENJPEG_NAMESPACE} OPENJPEG_LIBRARY_NAME)

project(${OPENJPEG_NAMESPACE} C)

# Do full dependency headers.
include_regular_expression("^.*$")

#-----------------------------------------------------------------------------
# OPENJPEG version number, useful for packaging and doxygen doc:
set(OPENJPEG_VERSION_MAJOR 1)
set(OPENJPEG_VERSION_MINOR 99)
set(OPENJPEG_VERSION_BUILD 0)
set(OPENJPEG_VERSION
  "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}")
set(PACKAGE_VERSION
  "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}")
set(OPENJPEG_LIBRARY_PROPERTIES
  VERSION   "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}"
  SOVERSION "${OPENJPEG_VERSION_MAJOR}"
)

# --------------------------------------------------------------------------
# Path to additional CMake modules
set(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/CMake
    ${CMAKE_MODULE_PATH})

# --------------------------------------------------------------------------
# On Visual Studio 8 MS deprecated C. This removes all 1.276E1265 security
# warnings
if(WIN32)
  if(NOT BORLAND)
    if(NOT CYGWIN)
      if(NOT MINGW)
        if(NOT ITK_ENABLE_VISUAL_STUDIO_DEPRECATED_C_WARNINGS)
          add_definitions(
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
        endif(NOT ITK_ENABLE_VISUAL_STUDIO_DEPRECATED_C_WARNINGS)
      endif(NOT MINGW)
    endif(NOT CYGWIN)
  endif(NOT BORLAND)
endif(WIN32)


# --------------------------------------------------------------------------
# Install directories

string(TOLOWER ${PROJECT_NAME} projectname)
set(OPENJPEG_INSTALL_SUBDIR "${projectname}-${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}")

if(NOT OPENJPEG_INSTALL_BIN_DIR)
  set(OPENJPEG_INSTALL_BIN_DIR "bin")
endif(NOT OPENJPEG_INSTALL_BIN_DIR)

if(NOT OPENJPEG_INSTALL_LIB_DIR)
  set(OPENJPEG_INSTALL_LIB_DIR "lib")
endif(NOT OPENJPEG_INSTALL_LIB_DIR)

if(NOT OPENJPEG_INSTALL_SHARE_DIR)
  set(OPENJPEG_INSTALL_SHARE_DIR "share")
endif(NOT OPENJPEG_INSTALL_SHARE_DIR)

if(NOT OPENJPEG_INSTALL_DATA_DIR)
  set(OPENJPEG_INSTALL_DATA_DIR "${OPENJPEG_INSTALL_SHARE_DIR}/${OPENJPEG_INSTALL_SUBDIR}")
endif(NOT OPENJPEG_INSTALL_DATA_DIR)

if(NOT OPENJPEG_INSTALL_INCLUDE_DIR)
  set(OPENJPEG_INSTALL_INCLUDE_DIR "include/${OPENJPEG_INSTALL_SUBDIR}")
endif(NOT OPENJPEG_INSTALL_INCLUDE_DIR)

if(NOT OPENJPEG_INSTALL_MAN_DIR)
  set(OPENJPEG_INSTALL_MAN_DIR "share/man/")
endif(NOT OPENJPEG_INSTALL_MAN_DIR)

if(NOT OPENJPEG_INSTALL_DOC_DIR)
  set(OPENJPEG_INSTALL_DOC_DIR "share/doc/${OPENJPEG_INSTALL_SUBDIR}")
endif(NOT OPENJPEG_INSTALL_DOC_DIR)

if(NOT OPENJPEG_INSTALL_PACKAGE_DIR)
  # We could install *.cmake files in share/ however those files contains
  # hardcoded path to libraries on a multi-arch system (fedora/debian) those
  # path will be different (lib/i386-linux-gnu vs lib/x86_64-linux-gnu)
  set(OPENJPEG_INSTALL_PACKAGE_DIR "${OPENJPEG_INSTALL_LIB_DIR}/${OPENJPEG_INSTALL_SUBDIR}")
endif(NOT OPENJPEG_INSTALL_PACKAGE_DIR)

#-----------------------------------------------------------------------------
# Big endian test:
include (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(OPJ_BIG_ENDIAN)

#-----------------------------------------------------------------------------
# Setup file for setting custom ctest vars
configure_file(
  ${CMAKE_SOURCE_DIR}/CMake/CTestCustom.cmake.in
  ${CMAKE_BINARY_DIR}/CTestCustom.cmake
  @ONLY
  )

#-----------------------------------------------------------------------------
# OpenJPEG build configuration options.
option(BUILD_SHARED_LIBS "Build OpenJPEG shared library and link executables against it." ON)
set (EXECUTABLE_OUTPUT_PATH ${OPENJPEG_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")
set (LIBRARY_OUTPUT_PATH ${OPENJPEG_BINARY_DIR}/bin CACHE PATH "Single output directory for building all libraries.")
mark_as_advanced(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

#-----------------------------------------------------------------------------
# configure name mangling to allow multiple libraries to coexist
# peacefully
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in)
set(MANGLE_PREFIX ${OPENJPEG_LIBRARY_NAME})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/openjpeg_mangle.h
               @ONLY)
endif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in)

#-----------------------------------------------------------------------------
# pkgconfig support
if(UNIX)
  # install in lib and not share (see multi-arch note above)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libopenjpeg1.pc.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libopenjpeg1.pc @ONLY)
  install( FILES  ${CMAKE_CURRENT_BINARY_DIR}/libopenjpeg1.pc DESTINATION
    ${OPENJPEG_INSTALL_LIB_DIR}/pkgconfig )
  install( CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  \"libopenjpeg1.pc\"
  \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${OPENJPEG_INSTALL_SHARE_DIR}/pkgconfig/libopenjpeg.pc\")")
endif(UNIX)

#-----------------------------------------------------------------------------
# Compiler specific flags:
if(CMAKE_COMPILER_IS_GNUCC)
  # For all builds, make sure openjpeg is std99 compliant:
  # set(CMAKE_C_FLAGS "-Wall -std=c99 ${CMAKE_C_FLAGS}") # FIXME: this setting prevented us from setting a coverage build.
  # Do not use ffast-math for all build, it would produce incorrect results, only set for release:
  set(CMAKE_C_FLAGS_RELEASE "-ffast-math ${CMAKE_C_FLAGS_RELEASE}")
endif(CMAKE_COMPILER_IS_GNUCC)

#-----------------------------------------------------------------------------
# opj_config.h generation (1/2)
include (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
CHECK_INCLUDE_FILE("strings.h"      HAVE_STRINGS_H)
CHECK_INCLUDE_FILE("inttypes.h"     HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("memory.h"       HAVE_MEMORY_H)
CHECK_INCLUDE_FILE("stdint.h"       HAVE_STDINT_H)
CHECK_INCLUDE_FILE("stdlib.h"       HAVE_STDLIB_H)
CHECK_INCLUDE_FILE("string.h"       HAVE_STRING_H)
CHECK_INCLUDE_FILE("sys/stat.h"     HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/types.h"    HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE("unistd.h"       HAVE_UNISTD_H)

# Enable Large file support
include(TestLargeFiles)
OPJ_TEST_LARGE_FILES(OPJ_HAVE_LARGEFILES)

#-----------------------------------------------------------------------------
# Build Library
include_directories(BEFORE ${OPENJPEG_BINARY_DIR})
add_subdirectory(libopenjpeg)

#-----------------------------------------------------------------------------
# Build Applications
option(BUILD_CODEC "Build the CODEC executables" ON)
option(BUILD_MJ2 "Build the MJ2 executables." OFF)
option(BUILD_JPWL "Build the JPWL library and executables" OFF)
option(BUILD_JPIP "Build the JPIP library and executables." OFF)
if(BUILD_JPIP)
  option(BUILD_JPIP_SERVER "Build the JPIP server." OFF)
endif(BUILD_JPIP)
option(BUILD_VIEWER "Build the OPJViewer executable (C++)" OFF)
option(BUILD_JAVA "Build the openjpeg jar (Java)" OFF)
mark_as_advanced(BUILD_VIEWER)
mark_as_advanced(BUILD_JAVA)

if(BUILD_CODEC OR BUILD_MJ2)
  # OFF: It will only build 3rd party libs if they are not found on the system
  # ON: 3rd party libs will ALWAYS be build, and used
  option(BUILD_THIRDPARTY "Build the thirdparty executables if it is needed" OFF)
  add_subdirectory(thirdparty)
  add_subdirectory(applications)
endif (BUILD_CODEC OR BUILD_MJ2)

include(CheckTypeSize)
CHECK_TYPE_SIZE(ssize_t     SSIZE_T)

#-----------------------------------------------------------------------------
# opj_config.h generation (2/2)
configure_file("${OPENJPEG_SOURCE_DIR}/opj_config.h.cmake.in"
 "${OPENJPEG_BINARY_DIR}/opj_config.h"
 @ONLY
 )
#
install( FILES  ${CMAKE_CURRENT_BINARY_DIR}/opj_config.h
 DESTINATION ${OPENJPEG_INSTALL_INCLUDE_DIR} COMPONENT Headers)
#-----------------------------------------------------------------------------
# Build DOCUMENTATION (not in ALL target and only if Doxygen is found)
option(BUILD_DOC "Build the HTML documentation (with doxygen if available)." OFF)
if(BUILD_DOC)
    add_subdirectory(doc)
endif(BUILD_DOC)

#-----------------------------------------------------------------------------
# Buld Testing
option(BUILD_TESTING "Build the tests." OFF)
if(BUILD_TESTING)
  if(BUILD_CODEC)
    enable_testing()
    include(CTest)
  
    # Search openjpeg data needed for the tests
    # They could be found via svn on the OpenJPEG google code project
    # svn checkout http://openjpeg.googlecode.com/svn/data (about 70 Mo) 
    find_path(OPJ_DATA_ROOT README-OPJ-Data
      PATHS
      $ENV{OPJ_DATA_ROOT}
      ${CMAKE_SOURCE_DIR}/../data
      )

    # Add repository where to find tests
    add_subdirectory(tests)
    
  else(BUILD_CODEC)
    message(FATAL_ERROR "You need build codec to run the tests")
  endif(BUILD_CODEC)
endif(BUILD_TESTING)

#-----------------------------------------------------------------------------
# install all targets referenced as OPENJPEGTargets
install(EXPORT OpenJPEGTargets DESTINATION ${OPENJPEG_INSTALL_PACKAGE_DIR})
configure_file( ${OPENJPEG_SOURCE_DIR}/CMake/OpenJPEGConfig.cmake.in
  ${OPENJPEG_BINARY_DIR}/OpenJPEGConfig.cmake
  @ONLY
)
install( FILES ${OPENJPEG_BINARY_DIR}/OpenJPEGConfig.cmake
  DESTINATION ${OPENJPEG_INSTALL_PACKAGE_DIR}
)

#-----------------------------------------------------------------------------
# install CHANGES and LICENSE
if(EXISTS ${OPENJPEG_SOURCE_DIR}/CHANGES)
  install(FILES CHANGES DESTINATION ${OPENJPEG_INSTALL_DOC_DIR})
endif(EXISTS ${OPENJPEG_SOURCE_DIR}/CHANGES)
install(FILES LICENSE DESTINATION ${OPENJPEG_INSTALL_DOC_DIR})

include (CMake/OpenJPEGCPack.cmake)
