# - Find LCMS2 library
# Find the native LCMS2 includes and library
# This module defines
#  LCMS2_INCLUDE_DIR, where to find tiff.h, etc.
#  LCMS2_LIBRARIES, libraries to link against to use LCMS2.
#  LCMS2_FOUND, If false, do not try to use LCMS2.
# also defined, but not for general use are
#  LCMS2_LIBRARY, where to find the LCMS2 library.
#
FIND_PATH(LCMS2_INCLUDE_DIR lcms2.h PATHS /usr/include /usr/local/include /opt/include /opt/local/include)
SET(LCMS2_NAMES ${LCMS2_NAMES} lcms2 liblcms2 liblcms2_static)
FIND_LIBRARY(LCMS2_LIBRARY NAMES ${LCMS2_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set LCMS2_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LCMS2  DEFAULT_MSG  LCMS2_LIBRARY  LCMS2_INCLUDE_DIR)
#
IF(LCMS2_FOUND)
  SET( LCMS2_LIBRARIES ${LCMS2_LIBRARY} )
ENDIF(LCMS2_FOUND)
#
MARK_AS_ADVANCED(LCMS2_INCLUDE_DIR LCMS2_LIBRARY)
