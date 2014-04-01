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
SET(OPENJPEG_VERSION_MINOR 5)
SET(OPENJPEG_VERSION_BUILD 3)
SET(OPENJPEG_VERSION
  "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}")
SET(PACKAGE_VERSION
  "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}")

# Because autotools does not support X.Y notation for SOVERSION, we have to use
# two numerorations, one for the openjpeg version and one for openjpeg soversion
# version | soversion
#   1.0   |  0
#   1.1   |  1
#   1.2   |  2
#   1.3   |  3
#   1.4   |  4
#   1.5   |  5
#   1.5.1 |  5
#   2.0   |  6
# above is the recommendation by the OPJ team. If you really need to override this default,
# you can specify your own OPENJPEG_SOVERSION at cmake configuration time:
# cmake -DOPENJPEG_SOVERSION:STRING=42 /path/to/openjpeg
if(NOT OPENJPEG_SOVERSION)
  SET(OPENJPEG_SOVERSION 5)
endif(NOT OPENJPEG_SOVERSION)
SET(OPENJPEG_LIBRARY_PROPERTIES
  VERSION   "${OPENJPEG_VERSION_MAJOR}.${OPENJPEG_VERSION_MINOR}.${OPENJPEG_VERSION_BUILD}"
  SOVERSION "${OPENJPEG_SOVERSION}"
)

# --------------------------------------------------------------------------
# Path to additional CMake modules
SET(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/CMake
    ${CMAKE_MODULE_PATH})

# --------------------------------------------------------------------------
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

# The following will compute the amount of parent dir to go
# from include to lib. it works nicely with 
# OPENJPEG_INSTALL_LIB_DIR=lib
# OPENJPEG_INSTALL_LIB_DIR=lib/
# OPENJPEG_INSTALL_LIB_DIR=/lib
# OPENJPEG_INSTALL_LIB_DIR=lib/gnu-linux-x64
STRING(REPLACE "/" ";" relative_to_lib ${OPENJPEG_INSTALL_LIB_DIR})
set(relative_parent "..")
foreach( elem ${relative_to_lib})
  set( relative_parent "${relative_parent}/.." )
endforeach()

IF(NOT OPENJPEG_INSTALL_SHARE_DIR)
  SET(OPENJPEG_INSTALL_SHARE_DIR "share")
ENDIF(NOT OPENJPEG_INSTALL_SHARE_DIR)

IF(NOT OPENJPEG_INSTALL_DATA_DIR)
  SET(OPENJPEG_INSTALL_DATA_DIR "${OPENJPEG_INSTALL_SHARE_DIR}/${OPENJPEG_INSTALL_SUBDIR}")
ENDIF(NOT OPENJPEG_INSTALL_DATA_DIR)

IF(NOT OPENJPEG_INSTALL_INCLUDE_DIR)
  SET(OPENJPEG_INSTALL_INCLUDE_DIR "include/${OPENJPEG_INSTALL_SUBDIR}")
ENDIF(NOT OPENJPEG_INSTALL_INCLUDE_DIR)

IF(NOT OPENJPEG_INSTALL_MAN_DIR)
  SET(OPENJPEG_INSTALL_MAN_DIR "share/man/")
ENDIF(NOT OPENJPEG_INSTALL_MAN_DIR)

IF(NOT OPENJPEG_INSTALL_DOC_DIR)
  SET(OPENJPEG_INSTALL_DOC_DIR "share/doc/${OPENJPEG_INSTALL_SUBDIR}")
ENDIF(NOT OPENJPEG_INSTALL_DOC_DIR)

if(NOT OPENJPEG_INSTALL_JNI_DIR)
  if(WIN32)
    set(OPENJPEG_INSTALL_JNI_DIR ${OPENJPEG_INSTALL_BIN_DIR})
  else()
    set(OPENJPEG_INSTALL_JNI_DIR ${OPENJPEG_INSTALL_LIB_DIR})
  endif()
endif()

IF(NOT OPENJPEG_INSTALL_PACKAGE_DIR)
  # We could install *.cmake files in share/ however those files contains
  # hardcoded path to libraries on a multi-arch system (fedora/debian) those
  # path will be different (lib/i386-linux-gnu vs lib/x86_64-linux-gnu)
  SET(OPENJPEG_INSTALL_PACKAGE_DIR "${OPENJPEG_INSTALL_LIB_DIR}/${OPENJPEG_INSTALL_SUBDIR}")
ENDIF(NOT OPENJPEG_INSTALL_PACKAGE_DIR)

#-----------------------------------------------------------------------------
# Big endian test:
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(OPJ_BIG_ENDIAN)

#-----------------------------------------------------------------------------
# Setup file for setting custom ctest vars
CONFIGURE_FILE(
  ${CMAKE_SOURCE_DIR}/CMake/CTestCustom.cmake.in
  ${CMAKE_BINARY_DIR}/CTestCustom.cmake
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
               @ONLY)
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg_mangle.h.in)

#-----------------------------------------------------------------------------
# pkgconfig support
IF(UNIX)
  # install in lib and not share (see multi-arch note above)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libopenjpeg1.pc.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libopenjpeg1.pc @ONLY)
  INSTALL( FILES  ${CMAKE_CURRENT_BINARY_DIR}/libopenjpeg1.pc DESTINATION
    ${OPENJPEG_INSTALL_LIB_DIR}/pkgconfig )
  INSTALL( CODE "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink
  \"libopenjpeg1.pc\"
  \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${OPENJPEG_INSTALL_SHARE_DIR}/pkgconfig/libopenjpeg.pc\")")
ENDIF(UNIX)

#-----------------------------------------------------------------------------
# Compiler specific flags:
IF(CMAKE_COMPILER_IS_GNUCC)
  # For all builds, make sure openjpeg is std99 compliant:
  # SET(CMAKE_C_FLAGS "-Wall -std=c99 ${CMAKE_C_FLAGS}") # FIXME: this setting prevented us from setting a coverage build.
  # Do not use ffast-math for all build, it would produce incorrect results, only set for release:
  SET(CMAKE_C_FLAGS_RELEASE "-ffast-math ${CMAKE_C_FLAGS_RELEASE}")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

#-----------------------------------------------------------------------------
# opj_config.h generation (1/2)
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
CHECK_INCLUDE_FILE("strings.h"      HAVE_STRINGS_H)
CHECK_INCLUDE_FILE("inttypes.h"     HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("memory.h"       HAVE_MEMORY_H)
CHECK_INCLUDE_FILE("stdint.h"       HAVE_STDINT_H)
CHECK_INCLUDE_FILE("stdlib.h"       HAVE_STDLIB_H)
CHECK_INCLUDE_FILE("string.h"       HAVE_STRING_H)
CHECK_INCLUDE_FILE("sys/stat.h"     HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/types.h"    HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE("unistd.h"       HAVE_UNISTD_H)


#-----------------------------------------------------------------------------
# Build Library
INCLUDE_DIRECTORIES(BEFORE ${OPENJPEG_BINARY_DIR})
ADD_SUBDIRECTORY(libopenjpeg)

#-----------------------------------------------------------------------------
# Build Applications
OPTION(BUILD_CODEC "Build the CODEC executables" ON)
OPTION(BUILD_MJ2 "Build the MJ2 executables." OFF)
OPTION(BUILD_JPWL "Build the JPWL library and executables" OFF)
OPTION(BUILD_JPIP "Build the JPIP library and executables." OFF)
IF(BUILD_JPIP)
  OPTION(BUILD_JPIP_SERVER "Build the JPIP server." OFF)
ENDIF(BUILD_JPIP)
OPTION(BUILD_VIEWER "Build the OPJViewer executable (C++)" OFF)
OPTION(BUILD_JAVA "Build the openjpeg jar (Java)" OFF)
OPTION(USE_SYSTEM_GETOPT "Prefer system installed getopt()" OFF)
MARK_AS_ADVANCED(USE_SYSTEM_GETOPT)
MARK_AS_ADVANCED(BUILD_VIEWER)
MARK_AS_ADVANCED(BUILD_JAVA)

IF(BUILD_CODEC OR BUILD_MJ2)
  # OFF: It will only build 3rd party libs if they are not found on the system
  # ON: 3rd party libs will ALWAYS be build, and used
  OPTION(BUILD_THIRDPARTY "Build the thirdparty executables if it is needed" OFF)
  ADD_SUBDIRECTORY(thirdparty)
  ADD_SUBDIRECTORY(applications)
ENDIF (BUILD_CODEC OR BUILD_MJ2)

#-----------------------------------------------------------------------------
# opj_config.h generation (2/2)
CONFIGURE_FILE("${OPENJPEG_SOURCE_DIR}/opj_config.h.cmake.in"
 "${OPENJPEG_BINARY_DIR}/opj_config.h"
 @ONLY
 )

#-----------------------------------------------------------------------------
# Build DOCUMENTATION (not in ALL target and only if Doxygen is found)
OPTION(BUILD_DOC "Build the HTML documentation (with doxygen if available)." OFF)
IF(BUILD_DOC)
    ADD_SUBDIRECTORY(doc)
ENDIF(BUILD_DOC)

#-----------------------------------------------------------------------------
# Buld Testing
OPTION(BUILD_TESTING "Build the tests." OFF)
IF(BUILD_TESTING)
  IF(BUILD_CODEC)
    ENABLE_TESTING()
    INCLUDE(CTest)
  
    # Search openjpeg data needed for the tests
    # They could be found via svn on the OpenJPEG google code project
    # svn checkout http://openjpeg.googlecode.com/svn/data (about 70 Mo) 
    FIND_PATH(OPJ_DATA_ROOT README-OPJ-Data
      PATHS $ENV{OPJ_DATA_ROOT} ${CMAKE_SOURCE_DIR}/../data
        ${CMAKE_SOURCE_DIR}/../../data
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
      )

    # Add repository where to find tests
    ADD_SUBDIRECTORY(tests)
    
  ELSE(BUILD_CODEC)
    message(FATAL_ERROR "You need build codec to run the tests")
  ENDIF(BUILD_CODEC)
ENDIF(BUILD_TESTING)

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
IF(EXISTS ${OPENJPEG_SOURCE_DIR}/CHANGES)
  INSTALL(FILES CHANGES DESTINATION ${OPENJPEG_INSTALL_DOC_DIR})
ENDIF(EXISTS ${OPENJPEG_SOURCE_DIR}/CHANGES)
INSTALL(FILES LICENSE DESTINATION ${OPENJPEG_INSTALL_DOC_DIR})

INCLUDE (CMake/OpenJPEGCPack.cmake)
