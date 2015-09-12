# -----------------------------------------------------------------------------
# Travis-ci ctest script for OpenJPEG project
# This will compile/run tests/upload to cdash OpenJPEG
# Results will be available at: http://my.cdash.org/index.php?project=OPENJPEG
# -----------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8)

set( ENV{LANG} en_US.UTF-8)
set( CTEST_DASHBOARD_ROOT  "$ENV{PWD}/build" )
set( CTEST_CMAKE_GENERATOR "Unix Makefiles")   # Always makefile in travis-ci environment

if ("$ENV{OPJ_BUILD_CONFIGURATION}" STREQUAL "")
  set( CTEST_BUILD_CONFIGURATION "Release")
else()
	set( CTEST_BUILD_CONFIGURATION "$ENV{OPJ_BUILD_CONFIGURATION}")
endif()

if ("$ENV{OPJ_SITE}" STREQUAL "")
  set( CTEST_SITE "Unknown")
else()
	set( CTEST_SITE "$ENV{OPJ_SITE}")
endif()

if ("$ENV{OPJ_BUILDNAME}" STREQUAL "")
  set( CTEST_BUILD_NAME "Unknown-${CTEST_BUILD_CONFIGURATION}")
else()
	set( CTEST_BUILD_NAME "$ENV{OPJ_BUILDNAME}")
endif()

if (NOT "$ENV{OPJ_CI_ARCH}" STREQUAL "")
	if (APPLE)
	  set(CCFLAGS_ARCH "-arch $ENV{OPJ_CI_ARCH}")
	else()
		if ("$ENV{OPJ_CI_ARCH}" MATCHES "^i[3-6]86$")
			set(CCFLAGS_ARCH "-m32 -march=$ENV{OPJ_CI_ARCH}")
		elseif ("$ENV{OPJ_CI_ARCH}" STREQUAL "x86_64")
			set(CCFLAGS_ARCH "-m64")
		endif()
	endif()
endif()

if(NOT "$ENV{OPJ_CI_SKIP_TESTS}" STREQUAL "1")
	# To execute part of the encoding test suite, kakadu binaries are needed to decode encoded image and compare 
	# it to the baseline. Kakadu binaries are freely available for non-commercial purposes 
	# at http://www.kakadusoftware.com.
	# Here's the copyright notice from kakadu:
	# Copyright is owned by NewSouth Innovations Pty Limited, commercial arm of the UNSW Australia in Sydney.
	# You are free to trial these executables and even to re-distribute them,
	# so long as such use or re-distribution is accompanied with this copyright notice and is not for commercial gain.
	# Note: Binaries can only be used for non-commercial purposes.
	if ("$ENV{OPJ_NONCOMMERCIAL}" STREQUAL "1" )
		set(KDUPATH $ENV{PWD}/kdu)
		set(ENV{LD_LIBRARY_PATH} ${KDUPATH})
		set(ENV{PATH} $ENV{PATH}:${KDUPATH})
	endif()
	set(BUILD_TESTING "TRUE")
else()
	set(BUILD_TESTING "FALSE")
endif(NOT "$ENV{OPJ_CI_SKIP_TESTS}" STREQUAL "1")

# Options 
set( CACHE_CONTENTS "

# Build kind
CMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}

# Warning level
CMAKE_C_FLAGS:STRING= ${CCFLAGS_ARCH} -Wall -Wextra -Wconversion -Wno-unused-parameter -Wdeclaration-after-statement

# Use to activate the test suite
BUILD_TESTING:BOOL=${BUILD_TESTING}

# Build Thirdparty, useful but not required for test suite 
BUILD_THIRDPARTY:BOOL=TRUE

# JPEG2000 test files are available with git clone https://github.com/uclouvain/openjpeg-data.git
OPJ_DATA_ROOT:PATH=$ENV{PWD}/data

# jpylyzer is available with on GitHub: https://github.com/openpreserve/jpylyzer  
JPYLYZER_EXECUTABLE=$ENV{PWD}/jpylyzer/jpylyzer/jpylyzer.py

" )

#---------------------
#1. openjpeg specific: 
set( CTEST_PROJECT_NAME	"OPENJPEG" )
if(NOT EXISTS $ENV{OPJ_SOURCE_DIR})
	message(FATAL_ERROR "OPJ_SOURCE_DIR not defined or does not exist:$ENV{OPJ_SOURCE_DIR}")
endif()
set( CTEST_SOURCE_DIRECTORY	"$ENV{OPJ_SOURCE_DIR}")
set( CTEST_BINARY_DIRECTORY	"${CTEST_DASHBOARD_ROOT}")

#---------------------
# Files to submit to the dashboard
set (CTEST_NOTES_FILES
${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}
${CTEST_BINARY_DIRECTORY}/CMakeCache.txt )

ctest_empty_binary_directory( "${CTEST_BINARY_DIRECTORY}" )
file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "${CACHE_CONTENTS}")

# Perform a Experimental build
ctest_start(Experimental)
#ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}")
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}")
ctest_read_custom_files(${CTEST_BINARY_DIRECTORY})
ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}")
if(NOT "$ENV{OPJ_CI_SKIP_TESTS}" STREQUAL "1")
	ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" PARALLEL_LEVEL 2)
endif()
if ("$ENV{OPJ_DO_SUBMIT}" STREQUAL "1")
	ctest_submit()
endif()
ctest_empty_binary_directory( "${CTEST_BINARY_DIRECTORY}" )
