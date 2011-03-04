MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = j2k_to_image image_to_j2k j2k_dump

j2k_to_image_CPPFLAGS = \
-I. \
-I$(top_srcdir)/codec \
-I$(top_builddir)/codec \
-I$(top_srcdir)/common \
-I$(top_builddir)/common \
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
../common/getopt.c \
convert.c \
index.c \
j2k_to_image.c \
../common/color.h

image_to_j2k_CPPFLAGS = \
-I. \
-I$(top_srcdir)/codec \
-I$(top_builddir)/codec \
-I$(top_srcdir)/common \
-I$(top_builddir)/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
@PNG_CFLAGS@ \
@TIFF_CFLAGS@
image_to_j2k_CFLAGS = 
image_to_j2k_LDADD = $(top_builddir)/libopenjpeg/libopenjpeg.la @TIFF_LIBS@ @PNG_LIBS@ -lm
image_to_j2k_SOURCES = \
../common/getopt.c \
convert.c \
index.c \
image_to_j2k.c \
convert.h \
../common/format_defs.h \
index.h

j2k_dump_CPPFLAGS = \
-I. \
-I$(top_srcdir)/codec \
-I$(top_builddir)/codec \
-I$(top_srcdir)/common \
-I$(top_builddir)/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg
j2k_dump_CFLAGS = 
j2k_dump_LDADD = $(top_builddir)/libopenjpeg/libopenjpeg.la -lm
j2k_dump_SOURCES = \
../common/getopt.c \
index.c \
j2k_dump.c \
../common/getopt.h \
index.h

EXTRA_DIST = \
CMakeLists.txt \
windirent.h

install-data-hook:
	@echo -e " (B)\t\t$(bindir)/j2k_to_image$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t\t$(bindir)/image_to_j2k$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t\t$(bindir)/j2k_dump$(EXEEXT)" >> $(top_builddir)/report.txt
