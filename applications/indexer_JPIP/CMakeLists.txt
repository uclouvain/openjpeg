# index_create

ADD_EXECUTABLE(index_create
bio.c cio.c int.c pi.c t2.c tgt.c tcd.c index_create.c jpip.c jp2.c
)
INSTALL(TARGETS index_create
  EXPORT OpenJPEGTargets
  DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
)
