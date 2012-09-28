# Build the demo app, small examples

# First thing define the common source:
set(common_SRCS
  convert.c
  index.c
  ${OPENJPEG_SOURCE_DIR}/src/bin/common/color.c
  ${OPENJPEG_SOURCE_DIR}/src/bin/common/opj_getopt.c
  )

# Headers file are located here:
include_directories(
  ${OPENJPEG_SOURCE_DIR}/src/lib/openjp2
  ${LCMS_INCLUDE_DIRNAME}
  ${OPENJPEG_SOURCE_DIR}/src/bin/common
  ${Z_INCLUDE_DIRNAME}
  ${PNG_INCLUDE_DIRNAME}
  ${TIFF_INCLUDE_DIRNAME}
  )

if(WIN32)
  if(BUILD_SHARED_LIBS)
    add_definitions(-DOPJ_EXPORTS)
  else()
    add_definitions(-DOPJ_STATIC)
  endif()
endif()

# Loop over all executables:
foreach(exe j2k_to_image image_to_j2k j2k_dump)
  add_executable(${exe} ${exe}.c ${common_SRCS})
  target_link_libraries(${exe} ${OPENJPEG_LIBRARY_NAME}
    ${PNG_LIBNAME} ${TIFF_LIBNAME} ${LCMS_LIBNAME}
    )
  # To support universal exe:
  if(ZLIB_FOUND AND APPLE)
    target_link_libraries(${exe} z)
  ELSe(ZLIB_FOUND AND APPLE)
    target_link_libraries(${exe} ${Z_LIBNAME})
  endif()

  # On unix you need to link to the math library:
  if(UNIX)
    target_link_libraries(${exe} m)
  endif()
  # Install exe
  install(TARGETS ${exe}
    EXPORT OpenJPEGTargets
    DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
  )
endforeach()

# Install man pages
install(
  FILES       ${OPENJPEG_SOURCE_DIR}/doc/man/man1/image_to_j2k.1
              ${OPENJPEG_SOURCE_DIR}/doc/man/man1/j2k_dump.1
              ${OPENJPEG_SOURCE_DIR}/doc/man/man1/j2k_to_image.1
  DESTINATION ${OPENJPEG_INSTALL_MAN_DIR}/man1)
#

if(BUILD_JPWL)
  add_executable(JPWL_j2k_to_image
  j2k_to_image.c
  ${common_SRCS}
  )

  set_property(
     TARGET JPWL_j2k_to_image
     APPEND PROPERTY COMPILE_DEFINITIONS USE_JPWL
  )

  target_link_libraries(JPWL_j2k_to_image ${OPENJPEG_LIBRARY_NAME}_JPWL
    ${LCMS_LIBNAME} ${PNG_LIBNAME} ${TIFF_LIBNAME})

  # To support universal exe:
  if(ZLIB_FOUND AND APPLE)
    target_link_libraries(JPWL_j2k_to_image z)
  ELSe(ZLIB_FOUND AND APPLE)
    target_link_libraries(JPWL_j2k_to_image ${Z_LIBNAME})
  endif()

  if(UNIX)
    target_link_libraries(JPWL_j2k_to_image m)
  endif()

  add_executable(JPWL_image_to_j2k
  image_to_j2k.c
  ${common_SRCS}
  )

  set_property(
     TARGET JPWL_image_to_j2k
     APPEND PROPERTY COMPILE_DEFINITIONS USE_JPWL
  )

  target_link_libraries(JPWL_image_to_j2k ${OPENJPEG_LIBRARY_NAME}_JPWL
  	${LCMS_LIBNAME} ${PNG_LIBNAME} ${TIFF_LIBNAME})
  	
  # To support universal exe:
  if(ZLIB_FOUND AND APPLE)
    target_link_libraries(JPWL_image_to_j2k z)
  ELSe(ZLIB_FOUND AND APPLE)
    target_link_libraries(JPWL_image_to_j2k ${Z_LIBNAME})
  endif()

  if(UNIX)
    target_link_libraries(JPWL_image_to_j2k m)
  endif()

  install(TARGETS JPWL_image_to_j2k JPWL_j2k_to_image
    DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
  )
endif()
