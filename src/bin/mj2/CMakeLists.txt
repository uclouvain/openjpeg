# Makefile for the MJ2 codecs of the OpenJPEG library: frames_to_mj2, mj2_to_frames, extract_j2k_from_mj2 and wrap_j2k_in_mj2

set(common_SRCS ${OPENJPEG_SOURCE_DIR}/src/bin/common/opj_getopt.c)

# While mj2 executables do not use the API correctly, we do not link with the library but rather compile the sources files.
set(OPJ_SRCS
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/bio.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/cio.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/dwt.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/event.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/image.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/j2k.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/j2k_lib.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/jp2.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/jpt.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/mct.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/mqc.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/openjpeg.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/pi.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/raw.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/t1.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/t2.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/tcd.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/tgt.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/cidx_manager.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/phix_manager.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/ppix_manager.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/thix_manager.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/tpix_manager.c
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/function_list.c
)

set(MJ2_SRCS mj2.c mj2_convert.c)

if(WIN32)
  add_definitions(-DOPJ_STATIC)
endif()

# Headers file are located here:
include_directories(
  ${OPENJPEG_SOURCE_DIR}/src/lib/openjp2
  ${OPENJPEG_SOURCE_DIR}/src/bin/common
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
endif()

add_executable(mj2_to_frames
    mj2_to_frames.c
    ${common_SRCS}
    ${OPJ_SRCS}
    ${MJ2_SRCS}
    ${OPENJPEG_SOURCE_DIR}/src/bin/common/color.c
    )
target_link_libraries(mj2_to_frames ${LCMS_LIBNAME})

if(UNIX)
  target_link_libraries(mj2_to_frames m)
endif()

add_executable(extract_j2k_from_mj2
    extract_j2k_from_mj2.c
    ${OPJ_SRCS}
    ${MJ2_SRCS}
    )
target_link_libraries(extract_j2k_from_mj2 ${LCMS_LIBNAME})

if(UNIX)
  target_link_libraries(extract_j2k_from_mj2 m)
endif()

add_executable(wrap_j2k_in_mj2
    wrap_j2k_in_mj2.c
    ${OPJ_SRCS}
    ${MJ2_SRCS}
    )
target_link_libraries(wrap_j2k_in_mj2 ${LCMS_LIBNAME})

if(UNIX)
  target_link_libraries(wrap_j2k_in_mj2 m)
endif()

install(TARGETS frames_to_mj2 mj2_to_frames extract_j2k_from_mj2 wrap_j2k_in_mj2
	DESTINATION ${OPENJPEG_INSTALL_BIN_DIR})
