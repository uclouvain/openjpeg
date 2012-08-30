# Makefile for the MJ2 codecs of the OpenJPEG library: frames_to_mj2, mj2_to_frames, extract_j2k_from_mj2 and wrap_j2k_in_mj2

set(common_SRCS ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c)

# While mj2 executables do not use the API correctly, we do not link with the library but rather compile the sources files.
set(OPJ_SRCS
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
${OPENJPEG_SOURCE_DIR}/libopenjpeg/cidx_manager.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/phix_manager.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/ppix_manager.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/thix_manager.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/tpix_manager.c
${OPENJPEG_SOURCE_DIR}/libopenjpeg/function_list.c
)

set(MJ2_SRCS mj2.c mj2_convert.c)

if(WIN32)
  add_definitions(-DOPJ_STATIC)
endif(WIN32)

# Headers file are located here:
include_directories(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  ${OPENJPEG_SOURCE_DIR}/applications/common
  ${LCMS_INCLUDE_DIRNAME}
  )

add_executable(frames_to_mj2
  frames_to_mj2.c
  ${common_SRCS}
  ${OPJ_SRCS}
  ${MJ2_SRCS}
  )
target_link_libraries(frames_to_mj2 ${LCMS_LIBNAME})

if(UNIX)
  target_link_libraries(frames_to_mj2 m)
endif(UNIX)

add_executable(mj2_to_frames
    mj2_to_frames.c
    ${common_SRCS}
    ${OPJ_SRCS}
    ${MJ2_SRCS}
    ${OPENJPEG_SOURCE_DIR}/applications/common/color.c
    )
target_link_libraries(mj2_to_frames ${LCMS_LIBNAME})

if(UNIX)
  target_link_libraries(mj2_to_frames m)
endif(UNIX)

add_executable(extract_j2k_from_mj2
    extract_j2k_from_mj2.c
    ${OPJ_SRCS}
    ${MJ2_SRCS}
    )
target_link_libraries(extract_j2k_from_mj2 ${LCMS_LIBNAME})

if(UNIX)
  target_link_libraries(extract_j2k_from_mj2 m)
endif(UNIX)

add_executable(wrap_j2k_in_mj2
    wrap_j2k_in_mj2.c
    ${OPJ_SRCS}
    ${MJ2_SRCS}
    )
target_link_libraries(wrap_j2k_in_mj2 ${LCMS_LIBNAME})

if(UNIX)
  target_link_libraries(wrap_j2k_in_mj2 m)
endif(UNIX)

install(TARGETS frames_to_mj2 mj2_to_frames extract_j2k_from_mj2 wrap_j2k_in_mj2
	DESTINATION ${OPENJPEG_INSTALL_BIN_DIR})
