# Makefile for the MJ2 codecs of the OpenJPEG library: frames_to_mj2, mj2_to_frames, extract_j2k_from_mj2 and wrap_j2k_in_mj2

# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  ${OPENJPEG_SOURCE_DIR}/codec/compat
  ${LCMS_INCLUDE_DIR}
  )

# Do the proper thing when building static...if only there was configured
# headers or def files instead
ADD_DEFINITIONS(-DOPJ_STATIC)

ADD_EXECUTABLE(frames_to_mj2
  frames_to_mj2.c
  ${PROJECT_SOURCE_DIR}/codec/compat/getopt.c
  mj2_convert.c
  mj2.c
  )
TARGET_LINK_LIBRARIES(frames_to_mj2 ${OPENJPEG_LIBRARY_NAME}.static ${LCMS_LIB})
IF(UNIX)
  TARGET_LINK_LIBRARIES(frames_to_mj2 m)
ENDIF(UNIX)

ADD_EXECUTABLE(mj2_to_frames
    mj2_to_frames.c
    ${PROJECT_SOURCE_DIR}/codec/compat/getopt.c
    mj2_convert.c
    mj2.c
    )
TARGET_LINK_LIBRARIES(mj2_to_frames ${OPENJPEG_LIBRARY_NAME}.static ${LCMS_LIB})
IF(UNIX)
  TARGET_LINK_LIBRARIES(mj2_to_frames m)
ENDIF(UNIX)

ADD_EXECUTABLE(extract_j2k_from_mj2
    extract_j2k_from_mj2.c
    mj2.c )
TARGET_LINK_LIBRARIES(extract_j2k_from_mj2 ${OPENJPEG_LIBRARY_NAME}.static ${LCMS_LIB})
IF(UNIX)
  TARGET_LINK_LIBRARIES(extract_j2k_from_mj2 m)
ENDIF(UNIX)

ADD_EXECUTABLE(wrap_j2k_in_mj2
    wrap_j2k_in_mj2.c
    mj2.c )
TARGET_LINK_LIBRARIES(wrap_j2k_in_mj2 ${OPENJPEG_LIBRARY_NAME}.static ${LCMS_LIB})
IF(UNIX)
  TARGET_LINK_LIBRARIES(wrap_j2k_in_mj2 m)
ENDIF(UNIX)

INSTALL(TARGETS frames_to_mj2 mj2_to_frames extract_j2k_from_mj2 wrap_j2k_in_mj2
	DESTINATION bin)
