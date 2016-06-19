#-----------------------------------------------------------------------------
#
# OPENJPEGConfig.cmake - CMake configuration file for external projects.
#
# This file is configured by OPENJPEG and used by the UseOPENJPEG.cmake
# module to load OPENJPEG's settings for an external project.

# The OPENJPEG version number.
SET(OPENJPEG_MAJOR_VERSION "1")
SET(OPENJPEG_MINOR_VERSION "5")
SET(OPENJPEG_BUILD_VERSION "2")

# The libraries.
SET(OPENJPEG_LIBRARIES "openjpeg")

# The CMake macros dir.
SET(OPENJPEG_CMAKE_DIR "lib/openjpeg-1.5")

# The configuration options.
SET(OPENJPEG_BUILD_SHARED_LIBS "ON")

# The "use" file.
SET(OPENJPEG_USE_FILE "")

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
# The following is inspired from:
# http://www.cmake.org/Wiki/CMake/Tutorials/Packaging#Packaging_and_Exporting
# However the following is difficult to handle:
# get_filename_component(myproj_INCLUDE_DIRS "${SELF_DIR}/../../include/myproj" ABSOLUTE)
# it asssumes a non multi-arch system, where 'include' is located '../include' from lib
# therefore we need to take into account the number of subdirs in OPENJPEG_INSTALL_LIB_DIR
if(EXISTS ${SELF_DIR}/OpenJPEGTargets.cmake)
  # This is an install tree
  include(${SELF_DIR}/OpenJPEGTargets.cmake)
  get_filename_component(OPENJPEG_INCLUDE_ROOT "${SELF_DIR}/../../include/openjpeg-1.5" ABSOLUTE)
  set(OPENJPEG_INCLUDE_DIRS ${OPENJPEG_INCLUDE_ROOT})

else(EXISTS ${SELF_DIR}/OpenJPEGTargets.cmake)
  if(EXISTS ${SELF_DIR}/OpenJPEGExports.cmake)
    # This is a build tree
    SET( OPENJPEG_INCLUDE_DIRS )

    include(${SELF_DIR}/OpenJPEGExports.cmake)

  else(EXISTS ${SELF_DIR}/OpenJPEGExports.cmake)
    message(FATAL_ERROR "ooops")
  endif(EXISTS ${SELF_DIR}/OpenJPEGExports.cmake)
endif(EXISTS ${SELF_DIR}/OpenJPEGTargets.cmake)

set(OPENJPEG_USE_FILE ${SELF_DIR}/UseOPENJPEG.cmake)

# Backward compatible part:
SET(OPENJPEG_FOUND       TRUE)

