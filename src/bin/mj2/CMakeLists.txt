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
${OPENJPEG_SOURCE_DIR}/src/lib/openjp2/function_list.c
)

#set(MJ2_SRCS mj2.c mj2_convert.c)

if(WIN32)
  add_definitions(-DOPJ_STATIC)
endif()

# Headers file are located here:
include_directories(
  ${OPENJPEG_BINARY_DIR}/src/lib/openjp2 # opj_config.h
  ${OPENJPEG_SOURCE_DIR}/src/lib/openjp2
  ${OPENJPEG_SOURCE_DIR}/src/lib/openmj2
  ${OPENJPEG_SOURCE_DIR}/src/bin/common
  ${LCMS_INCLUDE_DIRNAME}
  )

foreach(exe
  opj_mj2_wrap
  opj_mj2_extract
  opj_mj2_decompress
  opj_mj2_compress
)
  add_executable(${exe}
    ${exe}.c
    ${common_SRCS}
    ${OPJ_SRCS}
    ${MJ2_SRCS}
    ${OPENJPEG_SOURCE_DIR}/src/bin/common/color.c
    )
  target_link_libraries(${exe} ${LCMS_LIBNAME} openmj2)
  
  if(UNIX)
    target_link_libraries(${exe} m)
  endif()
  
  install(TARGETS ${exe}
  	DESTINATION ${OPENJPEG_INSTALL_BIN_DIR})
endforeach()
