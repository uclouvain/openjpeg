# Import library
#________________
	
	# MESSAGE(STATUS "IMPORT : FreeImagelibrary")

	# Include directories
	INCLUDE_DIRECTORIES(${OPENJPEG_SOURCE_DIR}/libs/FreeImage)
	
	# Link libraries
	
IF(WIN32)
	LINK_DIRECTORIES(${OPENJPEG_SOURCE_DIR}/libs/FreeImage)
	LINK_LIBRARIES(freeimage.s)
ELSE(WIN32)
  LINK_LIBRARIES(freeimage)
ENDIF(WIN32)

	ADD_DEFINITIONS ( -DFREEIMAGE_LIB )
