# - Find NPTH library
#
#  NPTH_INCLUDE_DIR
#  NPTH_LIBRARIES
#  NPTH_FOUND
#
# also defined, but not for general use are
#  NPTH_LIBRARY

FIND_PATH(NPTH_INCLUDE_DIR npth.h
  PATHS /usr/include /usr/local/include /opt/include /opt/local/include)

SET(NPTH_NAMES ${NPTH_NAMES} npth libnpth libnpth_static)

FIND_LIBRARY(NPTH_LIBRARY NAMES ${NPTH_NAMES} )

MARK_AS_ADVANCED(NPTH_INCLUDE_DIR NPTH_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set NPTH_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(NPTH  DEFAULT_MSG  NPTH_LIBRARY  NPTH_INCLUDE_DIR)

IF(NPTH_FOUND)
  SET(NPTH_INCLUDE_DIRS ${NPTH_INCLUDE_DIR})
  SET(NPTH_LIBRARIES ${NPTH_LIBRARY} )
ENDIF(NPTH_FOUND)
