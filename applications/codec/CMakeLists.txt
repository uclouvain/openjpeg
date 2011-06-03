  # Build the demo app, small examples

# First thing define the common source:
SET(common_SRCS
  convert.c
  index.c
  ${OPENJPEG_SOURCE_DIR}/applications/common/color.c
)

# If not getopt was found then add it to the lib:
IF(DONT_HAVE_GETOPT)
  SET(common_SRCS
    ${common_SRCS}
    ${OPENJPEG_SOURCE_DIR}/applications/common/getopt.c
  )
ENDIF(DONT_HAVE_GETOPT)

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
${Z_LIBNAME} ${PNG_LIBNAME} ${TIFF_LIBNAME})

  IF(LCMS_FOUND OR LCMS2_FOUND)
    TARGET_LINK_LIBRARIES(${exe} ${LCMS_LIBNAME})
  ENDIF(LCMS_FOUND OR LCMS2_FOUND)

  ADD_TEST(${exe} ${EXECUTABLE_OUTPUT_PATH}/${exe})
  # calling those exe without option will make them fail always:
  SET_TESTS_PROPERTIES(${exe} PROPERTIES WILL_FAIL TRUE)
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
    ${LCMS_LIBNAME} ${Z_LIBNAME} ${PNG_LIBNAME} ${TIFF_LIBNAME})
  
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
  	${LCMS_LIBNAME} ${Z_LIBNAME} ${PNG_LIBNAME} ${TIFF_LIBNAME})

  IF(UNIX)
    TARGET_LINK_LIBRARIES(JPWL_image_to_j2k m)
  ENDIF(UNIX)

  INSTALL(TARGETS JPWL_image_to_j2k JPWL_j2k_to_image
    DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
  )
ENDIF(BUILD_JPWL)

if(BUILD_TESTING)
# Do testing here, once we know the examples are being built:
FILE(GLOB_RECURSE OPENJPEG_DATA_IMAGES_GLOB
  "${JPEG2000_CONFORMANCE_DATA_ROOT}/*.j2k"
  "${JPEG2000_CONFORMANCE_DATA_ROOT}/*.j2c"
  "${JPEG2000_CONFORMANCE_DATA_ROOT}/*.jp2"
  )

foreach(filename ${OPENJPEG_DATA_IMAGES_GLOB})
  get_filename_component(filename_temp ${filename} NAME)
  get_filename_component(filename_ext ${filename} EXT)
  execute_process(COMMAND ${EXECUTABLE_OUTPUT_PATH}/j2k_dump -i ${filename}
    OUTPUT_VARIABLE dump_success
    OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${filename_temp}.dump
    ERROR_QUIET
  )
  if(dump_success)
  file(READ ${CMAKE_CURRENT_BINARY_DIR}/${filename_temp}.dump numcomp_file)
  string(REGEX REPLACE ".*numcomps=([0-9]+).*" "\\1"
    numcomps "${numcomp_file}")
  #message( "found:${output_variable} for ${filename_temp}" )
  endif()
  ADD_TEST(dump-${filename_temp} ${EXECUTABLE_OUTPUT_PATH}/j2k_dump -i ${filename})
  foreach(codec_type ppm pgx bmp tif raw tga png)
    ADD_TEST(j2i-${filename_temp}-${codec_type} ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image -i ${filename} -o ${filename_temp}.${codec_type})
  endforeach(codec_type)
  foreach(codec_type ppm bmp tif tga png)
    ADD_TEST(i2j-${filename_temp}-${codec_type} ${EXECUTABLE_OUTPUT_PATH}/image_to_j2k -i ${filename_temp}.${codec_type} -o ${filename_temp}.${codec_type}${filename_ext})
    #if(UNIX)
    #  ADD_TEST(cmp-${filename_temp}-${codec_type} cmp ${filename} ${filename_temp}.${codec_type}${filename_ext})
    #endif(UNIX)
  endforeach(codec_type)
endforeach(filename)
endif(BUILD_TESTING)
