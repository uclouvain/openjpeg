# Makefile for the MJ2 codecs of the OpenJPEG library: frames_to_mj2, mj2_to_frames, extract_j2k_from_mj2 and wrap_j2k_in_mj2

SET(common_SRCS "")
IF(DONT_HAVE_GETOPT)
  SET(common_SRCS ${OPENJPEG_SOURCE_DIR}/applications/common/getopt.c)
ENDIF(DONT_HAVE_GETOPT)

# While mj2 executables do not use the API correctly, we do not link with the library but rather compile the sources files.
SET(OPJ_SRCS
${OPENJPEG_SOURCE_DIR}/libopenjpeg/bio.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/cio.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/dwt.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/event.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/image.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/j2k.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/j2k_lib.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/jp2.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/jpt.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/mct.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/mqc.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/openjpeg.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/pi.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/raw.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/t1.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/t2.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/tcd.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/tgt.c
)

SET(MJ2_SRCS mj2.c mj2_convert.c)

IF(WIN32)
  ADD_DEFINITIONS(-DOPJ_STATIC)
ENDIF(WIN32)

# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  ${OPENJPEG_SOURCE_DIR}/applications/common
  ${LCMS_INCLUDE_DIRNAME}
  )

ADD_EXECUTABLE(frames_to_mj2
  frames_to_mj2.c
  ${common_SRCS}
  ${OPJ_SRCS}
  ${MJ2_SRCS}
  )
IF(LCMS_FOUND OR LCMS2_FOUND)
  TARGET_LINK_LIBRARIES(frames_to_mj2 ${LCMS_LIBNAME})
ENDIF(LCMS_FOUND OR LCMS2_FOUND)
IF(UNIX)
  TARGET_LINK_LIBRARIES(frames_to_mj2 m)
ENDIF(UNIX)

ADD_EXECUTABLE(mj2_to_frames
    mj2_to_frames.c
    ${common_SRCS}
    ${OPJ_SRCS}
    ${MJ2_SRCS}
    ${OPENJPEG_SOURCE_DIR}/applications/common/color.c
    )
IF(LCMS_FOUND OR LCMS2_FOUND)
  TARGET_LINK_LIBRARIES(mj2_to_frames ${LCMS_LIBNAME})
ENDIF(LCMS_FOUND OR LCMS2_FOUND)
IF(UNIX)
  TARGET_LINK_LIBRARIES(mj2_to_frames m)
ENDIF(UNIX)

ADD_EXECUTABLE(extract_j2k_from_mj2
    extract_j2k_from_mj2.c
    ${OPJ_SRCS}
    ${MJ2_SRCS}
    )
IF(LCMS_FOUND OR LCMS2_FOUND)
  TARGET_LINK_LIBRARIES(extract_j2k_from_mj2 ${LCMS_LIBNAME})
ENDIF(LCMS_FOUND OR LCMS2_FOUND)
IF(UNIX)
  TARGET_LINK_LIBRARIES(extract_j2k_from_mj2 m)
ENDIF(UNIX)

ADD_EXECUTABLE(wrap_j2k_in_mj2
    wrap_j2k_in_mj2.c
    ${OPJ_SRCS}
    ${MJ2_SRCS}
    )
IF(LCMS_FOUND OR LCMS2_FOUND)
  TARGET_LINK_LIBRARIES(wrap_j2k_in_mj2 ${LCMS_LIBNAME})
ENDIF(LCMS_FOUND OR LCMS2_FOUND)
IF(UNIX)
  TARGET_LINK_LIBRARIES(wrap_j2k_in_mj2 m)
ENDIF(UNIX)

INSTALL(TARGETS frames_to_mj2 mj2_to_frames extract_j2k_from_mj2 wrap_j2k_in_mj2
	DESTINATION ${OPENJPEG_INSTALL_BIN_DIR})
