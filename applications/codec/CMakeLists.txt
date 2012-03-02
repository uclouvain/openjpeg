# Build the demo app, small examples

# First thing define the common source:
SET(common_SRCS
  convert.c
  index.c
  ${OPENJPEG_SOURCE_DIR}/applications/common/color.c
  ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c
  )

# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  ${LCMS_INCLUDE_DIRNAME}
  ${OPENJPEG_SOURCE_DIR}/applications/common
  ${Z_INCLUDE_DIRNAME}
  ${PNG_INCLUDE_DIRNAME}
  ${TIFF_INCLUDE_DIRNAME}
  )

IF(WIN32)
  IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_EXPORTS)
  ELSE(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_STATIC)
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(WIN32)

# Loop over all executables:
FOREACH(exe j2k_to_image image_to_j2k j2k_dump)
  ADD_EXECUTABLE(${exe} ${exe}.c ${common_SRCS})
  TARGET_LINK_LIBRARIES(${exe} ${OPENJPEG_LIBRARY_NAME} 
    ${PNG_LIBNAME} ${TIFF_LIBNAME} ${LCMS_LIBNAME}
    )
  # To support universal exe:
  IF(ZLIB_FOUND AND APPLE)
    TARGET_LINK_LIBRARIES(${exe} z)
  ELSe(ZLIB_FOUND AND APPLE)
    TARGET_LINK_LIBRARIES(${exe} ${Z_LIBNAME})
  ENDIF(ZLIB_FOUND AND APPLE)

  # On unix you need to link to the math library:
  IF(UNIX)
    TARGET_LINK_LIBRARIES(${exe} m)
  ENDIF(UNIX)
  # Install exe
  INSTALL(TARGETS ${exe}
    EXPORT OpenJPEGTargets
    DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
  )
ENDFOREACH(exe)

# Install man pages
INSTALL(
  FILES       ${OPENJPEG_SOURCE_DIR}/doc/man/man1/image_to_j2k.1
              ${OPENJPEG_SOURCE_DIR}/doc/man/man1/j2k_dump.1
              ${OPENJPEG_SOURCE_DIR}/doc/man/man1/j2k_to_image.1
  DESTINATION ${OPENJPEG_INSTALL_MAN_DIR}/man1)
#

IF(BUILD_JPWL)
  ADD_EXECUTABLE(JPWL_j2k_to_image
  j2k_to_image.c
  ${common_SRCS}
  )
  
  SET_PROPERTY(
     TARGET JPWL_j2k_to_image
     APPEND PROPERTY COMPILE_DEFINITIONS USE_JPWL
  )

  TARGET_LINK_LIBRARIES(JPWL_j2k_to_image ${OPENJPEG_LIBRARY_NAME}_JPWL
    ${LCMS_LIBNAME} ${PNG_LIBNAME} ${TIFF_LIBNAME})
    
  # To support universal exe:
  IF(ZLIB_FOUND AND APPLE)
    TARGET_LINK_LIBRARIES(JPWL_j2k_to_image z)
  ELSe(ZLIB_FOUND AND APPLE)
    TARGET_LINK_LIBRARIES(JPWL_j2k_to_image ${Z_LIBNAME})
  ENDIF(ZLIB_FOUND AND APPLE)
  
  IF(UNIX)
    TARGET_LINK_LIBRARIES(JPWL_j2k_to_image m)
  ENDIF(UNIX)

  ADD_EXECUTABLE(JPWL_image_to_j2k
  image_to_j2k.c
  ${common_SRCS}
  )
  
  SET_PROPERTY(
     TARGET JPWL_image_to_j2k
     APPEND PROPERTY COMPILE_DEFINITIONS USE_JPWL
  )

  TARGET_LINK_LIBRARIES(JPWL_image_to_j2k ${OPENJPEG_LIBRARY_NAME}_JPWL 
  	${LCMS_LIBNAME} ${PNG_LIBNAME} ${TIFF_LIBNAME})
  	
  # To support universal exe:
  IF(ZLIB_FOUND AND APPLE)
    TARGET_LINK_LIBRARIES(JPWL_image_to_j2k z)
  ELSe(ZLIB_FOUND AND APPLE)
    TARGET_LINK_LIBRARIES(JPWL_image_to_j2k ${Z_LIBNAME})
  ENDIF(ZLIB_FOUND AND APPLE)

  IF(UNIX)
    TARGET_LINK_LIBRARIES(JPWL_image_to_j2k m)
  ENDIF(UNIX)

  INSTALL(TARGETS JPWL_image_to_j2k JPWL_j2k_to_image
    DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
  )
ENDIF(BUILD_JPWL)
