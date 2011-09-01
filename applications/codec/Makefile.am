MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = j2k_to_image image_to_j2k j2k_dump

if WANT_JPWL
bin_PROGRAMS += JPWL_j2k_to_image JPWL_image_to_j2k
endif

j2k_to_image_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/codec \
-I$(top_builddir)/applications/codec \
-I$(top_srcdir)/applications/common \
-I$(top_builddir)/applications/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
@PNG_CFLAGS@ \
@TIFF_CFLAGS@ \
@LCMS1_CFLAGS@ \
@LCMS2_CFLAGS@
j2k_to_image_CFLAGS = 
j2k_to_image_LDADD = $(top_builddir)/libopenjpeg/libopenjpeg.la @LCMS1_LIBS@ @LCMS2_LIBS@ @TIFF_LIBS@ @PNG_LIBS@ -lm
j2k_to_image_SOURCES = \
../common/color.c \
../common/opj_getopt.c \
convert.c \
index.c \
j2k_to_image.c \
../common/color.h

image_to_j2k_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/codec \
-I$(top_builddir)/applications/codec \
-I$(top_srcdir)/applications/common \
-I$(top_builddir)/applications/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
@PNG_CFLAGS@ \
@TIFF_CFLAGS@
image_to_j2k_CFLAGS = 
image_to_j2k_LDADD = $(top_builddir)/libopenjpeg/libopenjpeg.la @TIFF_LIBS@ @PNG_LIBS@ -lm
image_to_j2k_SOURCES = \
../common/opj_getopt.c \
convert.c \
index.c \
image_to_j2k.c \
convert.h \
../common/format_defs.h \
index.h

j2k_dump_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/codec \
-I$(top_builddir)/applications/codec \
-I$(top_srcdir)/applications/common \
-I$(top_builddir)/applications/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg
j2k_dump_CFLAGS = 
j2k_dump_LDADD = $(top_builddir)/libopenjpeg/libopenjpeg.la -lm
j2k_dump_SOURCES = \
../common/opj_getopt.c \
index.c \
j2k_dump.c \
../common/opj_getopt.h \
index.h

JPWL_j2k_to_image_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/common \
-I$(top_builddir)/applications/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
-DUSE_JPWL \
@TIFF_CFLAGS@ \
@PNG_CFLAGS@ \
@LCMS1_CFLAGS@ \
@LCMS2_CFLAGS@
JPWL_j2k_to_image_CFLAGS =
JPWL_j2k_to_image_LDADD = $(top_builddir)/libopenjpeg/jpwl/libopenjpeg_JPWL.la  @LCMS1_LIBS@ @LCMS2_LIBS@ @TIFF_LIBS@ @PNG_LIBS@
JPWL_j2k_to_image_SOURCES = \
../common/color.c \
../common/opj_getopt.c \
index.c \
convert.c \
j2k_to_image.c

JPWL_image_to_j2k_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/common \
-I$(top_builddir)/applications/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
-DUSE_JPWL \
@TIFF_CFLAGS@ \
@PNG_CFLAGS@
JPWL_image_to_j2k_CFLAGS =
JPWL_image_to_j2k_LDADD = $(top_builddir)/libopenjpeg/jpwl/libopenjpeg_JPWL.la @TIFF_LIBS@ @PNG_LIBS@

JPWL_image_to_j2k_SOURCES = \
../common/opj_getopt.c \
index.c \
convert.c \
image_to_j2k.c

EXTRA_DIST = \
CMakeLists.txt \
windirent.h

install-data-hook:
	@echo -e " (B)\t$(bindir)/j2k_to_image$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/image_to_j2k$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/j2k_dump$(EXEEXT)" >> $(top_builddir)/report.txt
if WANT_JPWL
	@echo -e " (B)\t$(bindir)/JPWL_j2k_to_image$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/JPWL_image_to_j2k$(EXEEXT)" >> $(top_builddir)/report.txt
endif
