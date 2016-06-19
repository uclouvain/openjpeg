#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "openjpeg" for configuration "Debug"
set_property(TARGET openjpeg APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(openjpeg PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "m"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libopenjpeg.so.1.5.1"
  IMPORTED_SONAME_DEBUG "libopenjpeg.so.5"
  )

list(APPEND _IMPORT_CHECK_TARGETS openjpeg )
list(APPEND _IMPORT_CHECK_FILES_FOR_openjpeg "${_IMPORT_PREFIX}/lib/libopenjpeg.so.1.5.1" )

# Import target "j2k_to_image" for configuration "Debug"
set_property(TARGET j2k_to_image APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(j2k_to_image PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/j2k_to_image"
  )

list(APPEND _IMPORT_CHECK_TARGETS j2k_to_image )
list(APPEND _IMPORT_CHECK_FILES_FOR_j2k_to_image "${_IMPORT_PREFIX}/bin/j2k_to_image" )

# Import target "image_to_j2k" for configuration "Debug"
set_property(TARGET image_to_j2k APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(image_to_j2k PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/image_to_j2k"
  )

list(APPEND _IMPORT_CHECK_TARGETS image_to_j2k )
list(APPEND _IMPORT_CHECK_FILES_FOR_image_to_j2k "${_IMPORT_PREFIX}/bin/image_to_j2k" )

# Import target "j2k_dump" for configuration "Debug"
set_property(TARGET j2k_dump APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(j2k_dump PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/j2k_dump"
  )

list(APPEND _IMPORT_CHECK_TARGETS j2k_dump )
list(APPEND _IMPORT_CHECK_FILES_FOR_j2k_dump "${_IMPORT_PREFIX}/bin/j2k_dump" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
