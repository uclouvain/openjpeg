# - Find LCMS library
# Find the native LCMS includes and library
# This module defines
#  LCMS_INCLUDE_DIR, where to find tiff.h, etc.
#  LCMS_LIBRARIES, libraries to link against to use LCMS.
#  LCMS_FOUND, If false, do not try to use LCMS.
# also defined, but not for general use are
#  LCMS_LIBRARY, where to find the LCMS library.
#
FIND_PATH(LCMS_INCLUDE_DIR lcms.h  PATHS /usr/include /usr/local/include /opt/include /opt/local/include)
#
SET(LCMS_NAMES ${LCMS_NAMES} lcms liblcms liblcms_static)
FIND_LIBRARY(LCMS_LIBRARY NAMES ${LCMS_NAMES} )
#
# handle the QUIETLY and REQUIRED arguments and set LCMS_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LCMS  DEFAULT_MSG  LCMS_LIBRARY  LCMS_INCLUDE_DIR)
#
IF(LCMS_FOUND)
  SET( LCMS_LIBRARIES ${LCMS_LIBRARY} )
ENDIF(LCMS_FOUND)

MARK_AS_ADVANCED(LCMS_INCLUDE_DIR LCMS_LIBRARY)
