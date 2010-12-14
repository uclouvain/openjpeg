MAJOR = @MAJOR_NR@
MINOR = @MINOR_NR@
BUILD = @BUILD_NR@
CURRENT=`expr $(MAJOR) + $(MINOR)`

lib_LTLIBRARIES = libopenjpeg_JPWL.la

libopenjpeg_JPWL_la_LDFLAGS = -no-undefined \
  -version-info $(CURRENT):$(BUILD):$(MINOR)

JPWL_SRCS = crc.c jpwl.c jpwl_lib.c rs.c
JPWL_INCLS = jpwl.h crc.h rs.h

SRCS = ../libopenjpeg/bio.c ../libopenjpeg/cio.c ../libopenjpeg/dwt.c \
  ../libopenjpeg/event.c ../libopenjpeg/image.c ../libopenjpeg/j2k.c \
  ../libopenjpeg/j2k_lib.c ../libopenjpeg/jp2.c ../libopenjpeg/jpt.c \
  ../libopenjpeg/mct.c ../libopenjpeg/mqc.c ../libopenjpeg/openjpeg.c \
  ../libopenjpeg/pi.c ../libopenjpeg/raw.c ../libopenjpeg/t1.c \
  ../libopenjpeg/t2.c ../libopenjpeg/tcd.c ../libopenjpeg/tgt.c \
  $(JPWL_SRCS)

INCLS = ../libopenjpeg/bio.h ../libopenjpeg/cio.h ../libopenjpeg/dwt.h \
  ../libopenjpeg/event.h ../libopenjpeg/fix.h ../libopenjpeg/image.h \
  ../libopenjpeg/int.h ../libopenjpeg/j2k.h ../libopenjpeg/j2k_lib.h \
  ../libopenjpeg/jp2.h ../libopenjpeg/jpt.h ../libopenjpeg/mct.h \
  ../libopenjpeg/mqc.h ../libopenjpeg/openjpeg.h ../libopenjpeg/pi.h \
  ../libopenjpeg/raw.h ../libopenjpeg/t1.h ../libopenjpeg/t2.h \
  ../libopenjpeg/tcd.h ../libopenjpeg/tgt.h ../libopenjpeg/opj_malloc.h \
  ../libopenjpeg/opj_includes.h \
  $(JPWL_INCLS)

libopenjpeg_JPWL_la_SOURCES = $(SRCS) $(INCLS)

# Converts cr/lf to just lf
DOS2UNIX = dos2unix

dos2unix:
	@$(DOS2UNIX) $(SRCS) $(INCLS)

COMPILERFLAGS = -Wall -ffast-math -std=c99

USERLIBS =
INCLUDES = -I.. -I. -I../libopenjpeg -I../common

if with_libtiff
INCLUDES += @tiffincludes@
USERLIBS += @tifflibs@
endif

if with_libpng
INCLUDES += @pngincludes@
USERLIBS += @pnglibs@
endif

if with_liblcms2
INCLUDES += @lcms2includes@
USERLIBS += @lcms2libs@
endif

if with_liblcms1
INCLUDES += @lcms1includes@
USERLIBS += @lcms1libs@
endif

USERLIBS += -lm

bin_PROGRAMS = JPWL_j2k_to_image JPWL_image_to_j2k

JPWL_j2k_to_image_CFLAGS = $(COMPILERFLAGS) $(INCLUDES) -DUSE_JPWL
JPWL_image_to_j2k_CFLAGS = $(COMPILERFLAGS) $(INCLUDES) -DUSE_JPWL

if with_sharedlibs
COMPILERFLAGS += -DOPJ_EXPORTS
else
COMPILERFLAGS += -DOPJ_STATIC
libopenjpeg_JPWL_la_LDFLAGS += -static
endif

CFLAGS = $(COMPILERFLAGS) $(INCLUDES) -DUSE_JPWL
LDADD = ./libopenjpeg_JPWL.la $(USERLIBS)

JPWL_j2k_to_image_SOURCES = ../common/getopt.c ../codec/index.c \
	../codec/convert.c ../common/color.c ../codec/j2k_to_image.c

JPWL_image_to_j2k_SOURCES = ../common/getopt.c ../codec/index.c \
	../codec/convert.c ../codec/image_to_j2k.c 

REPBIN=$(bin_PROGRAMS)

all-local:
	$(INSTALL) -d ../bin
	$(INSTALL) $(bin_PROGRAMS) ../bin
	cp -P .libs/libopenjpeg_JPWL.* ../bin
	rm -f ../bin/libopenjpeg_JPWL.la*
	@rm -rf .report_lib
	@$(INSTALL) -d .report_lib
	@cp -P .libs/libopenjpeg_JPWL.* .report_lib
	@rm -f .report_lib/libopenjpeg_JPWL.lai
	echo "" > .report.txt
	@(for f in ${REPBIN} ; do \
	echo "Installing: ${prefix}/bin/$$f" >> .report.txt ; \
	done)
	@(cd .report_lib; \
	l=`ls --file-type`; \
	for f in $$l ; do \
	echo "Installing: ${prefix}/lib/$$f" >> ../.report.txt ; \
	done)
	@rm -rf .report_lib

