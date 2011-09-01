#JavaOpenJPEG/CMakeLists.txt

# First thing define the common source:
SET(common_SRCS
  ../codec/convert.c
  ../codec/index.c
  ../common/opj_getopt.c
  )


# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  ${LCMS_INCLUDE_DIR}
  ${PNG_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIR}
  ${TIFF_INCLUDE_DIR}
  )

# Loop over all executables:
FOREACH(exe j2k_to_image image_to_j2k)
  ADD_EXECUTABLE(${exe} ${exe}.c ${common_SRCS})
  TARGET_LINK_LIBRARIES(${exe} ${OPJ_PREFIX}openjpeg ${TIFF_LIBRARIES}
	${PNG_LIBRARIES} ${ZLIB_LIBRARY} ${LCMS_LIB})
  # On unix you need to link to the math library:
  IF(UNIX)
    TARGET_LINK_LIBRARIES(${exe} -lm)
  ENDIF(UNIX)
  # Install exe
  INSTALL_TARGETS(/bin/ ${exe})
ENDFOREACH(exe)


