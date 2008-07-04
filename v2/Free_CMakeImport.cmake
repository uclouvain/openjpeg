# Import library
#________________
	
	# MESSAGE(STATUS "IMPORT : FreeImagelibrary")

	# Include directories
	INCLUDE_DIRECTORIES(${OPENJPEG_SOURCE_DIR}/libs/FreeImage)
	
	# Link libraries
	LINK_DIRECTORIES(${OPENJPEG_SOURCE_DIR}/libs/FreeImage)
	
	LINK_LIBRARIES(freeimage.s)
	ADD_DEFINITIONS ( -DFREEIMAGE_LIB )

