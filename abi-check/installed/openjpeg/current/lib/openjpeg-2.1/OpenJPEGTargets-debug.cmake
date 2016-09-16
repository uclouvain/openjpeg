#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
SET(CMAKE_IMPORT_FILE_VERSION 1)

# Compute the installation prefix relative to this file.
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)

# Import target "openjp2" for configuration "Debug"
SET_PROPERTY(TARGET openjp2 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
SET_TARGET_PROPERTIES(openjp2 PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "m;-lpthread"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libopenjp2.so.2.1.1"
  IMPORTED_SONAME_DEBUG "libopenjp2.so.7"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS openjp2 )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_openjp2 "${_IMPORT_PREFIX}/lib/libopenjp2.so.2.1.1" )

# Import target "opj_decompress" for configuration "Debug"
SET_PROPERTY(TARGET opj_decompress APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
SET_TARGET_PROPERTIES(opj_decompress PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/opj_decompress"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS opj_decompress )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_opj_decompress "${_IMPORT_PREFIX}/bin/opj_decompress" )

# Import target "opj_compress" for configuration "Debug"
SET_PROPERTY(TARGET opj_compress APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
SET_TARGET_PROPERTIES(opj_compress PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/opj_compress"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS opj_compress )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_opj_compress "${_IMPORT_PREFIX}/bin/opj_compress" )

# Import target "opj_dump" for configuration "Debug"
SET_PROPERTY(TARGET opj_dump APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
SET_TARGET_PROPERTIES(opj_dump PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/opj_dump"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS opj_dump )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_opj_dump "${_IMPORT_PREFIX}/bin/opj_dump" )

# Loop over all imported files and verify that they actually exist
FOREACH(target ${_IMPORT_CHECK_TARGETS} )
  FOREACH(file ${_IMPORT_CHECK_FILES_FOR_${target}} )
    IF(NOT EXISTS "${file}" )
      MESSAGE(FATAL_ERROR "The imported target \"${target}\" references the file
   \"${file}\"
but this file does not exist.  Possible reasons include:
* The file was deleted, renamed, or moved to another location.
* An install or uninstall procedure did not complete successfully.
* The installation package was faulty and contained
   \"${CMAKE_CURRENT_LIST_FILE}\"
but not all the files it references.
")
    ENDIF()
  ENDFOREACH()
  UNSET(_IMPORT_CHECK_FILES_FOR_${target})
ENDFOREACH()
UNSET(_IMPORT_CHECK_TARGETS)

# Cleanup temporary variables.
SET(_IMPORT_PREFIX)

# Commands beyond this point should not need to know the version.
SET(CMAKE_IMPORT_FILE_VERSION)
