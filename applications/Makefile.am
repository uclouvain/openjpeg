MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = codec

if WANT_MJ2
SUBDIRS += mj2
endif

if WANT_JPIP_CODE
SUBDIRS += jpip
endif

EXTRA_DIST = \
CMakeLists.txt \
common/color.c \
common/color.h \
common/format_defs.h \
common/opj_getopt.c \
common/opj_getopt.h
