MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libopenjpeg_JPWL.la

OPJ_SRC = \
../libopenjpeg/bio.c \
../libopenjpeg/cio.c \
../libopenjpeg/dwt.c \
../libopenjpeg/event.c \
../libopenjpeg/image.c \
../libopenjpeg/j2k.c \
../libopenjpeg/j2k_lib.c \
../libopenjpeg/jp2.c \
../libopenjpeg/jpt.c \
../libopenjpeg/mct.c \
../libopenjpeg/mqc.c \
../libopenjpeg/openjpeg.c \
../libopenjpeg/pi.c \
../libopenjpeg/raw.c \
../libopenjpeg/t1.c \
../libopenjpeg/t2.c \
../libopenjpeg/tcd.c \
../libopenjpeg/tgt.c

libopenjpeg_JPWL_la_CPPFLAGS = \
-I. \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
-I$(top_srcdir)/jpwl \
-I$(top_builddir)/jpwl \
-DUSE_JPWL
libopenjpeg_JPWL_la_CFLAGS =
libopenjpeg_JPWL_la_LIBADD = -lm
libopenjpeg_JPWL_la_LDFLAGS = -no-undefined -version-info @lt_version@
libopenjpeg_JPWL_la_SOURCES = \
$(OPJ_SRC) \
crc.c \
jpwl.c \
jpwl_lib.c \
rs.c \
jpwl.h \
crc.h \
rs.h

bin_PROGRAMS = JPWL_j2k_to_image JPWL_image_to_j2k

JPWL_j2k_to_image_CPPFLAGS = \
-I. \
-I$(top_srcdir)/common \
-I$(top_builddir)/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
-I$(top_srcdir)/jpwl \
-I$(top_builddir)/jpwl \
-DUSE_JPWL \
@TIFF_CFLAGS@ \
@PNG_CFLAGS@ \
@LCMS1_CFLAGS@ \
@LCMS2_CFLAGS@
JPWL_j2k_to_image_CFLAGS =
JPWL_j2k_to_image_LDADD = $(top_builddir)/jpwl/libopenjpeg_JPWL.la  @LCMS1_LIBS@ @LCMS2_LIBS@ @TIFF_LIBS@ @PNG_LIBS@
JPWL_j2k_to_image_SOURCES = \
$(top_builddir)/common/color.c \
$(top_builddir)/common/getopt.c \
$(top_builddir)/codec/index.c \
$(top_builddir)/codec/convert.c \
$(top_builddir)/codec/j2k_to_image.c

JPWL_image_to_j2k_CPPFLAGS = \
-I. \
-I$(top_srcdir)/common \
-I$(top_builddir)/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
-I$(top_srcdir)/jpwl \
-I$(top_builddir)/jpwl \
-DUSE_JPWL \
@TIFF_CFLAGS@ \
@PNG_CFLAGS@
JPWL_image_to_j2k_CFLAGS =
JPWL_image_to_j2k_LDADD = $(top_builddir)/jpwl/libopenjpeg_JPWL.la @TIFF_LIBS@ @PNG_LIBS@

JPWL_image_to_j2k_SOURCES = \
$(top_builddir)/common/getopt.c \
$(top_builddir)/codec/index.c \
$(top_builddir)/codec/convert.c \
$(top_builddir)/codec/image_to_j2k.c

EXTRA_DIST = \
CMakeLists.txt

install-data-hook:
	@echo -e " (B)\t\t$(bindir)/JPWL_j2k_to_image$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t\t$(bindir)/JPWL_image_to_j2k$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (LA)\t$(libdir)/libopenjpeg_JPWL.la" >> $(top_builddir)/report.txt
	@( $(call solist) ) >> $(top_builddir)/report.txt
	@echo -e " (A)\t\t$(base)/$(a)" >> $(top_builddir)/report.txt

solist    = $(foreach f, $(dll) $(so), echo -e ' $(SO_PREFIX)\t$(base)/$(f)' ;)
get_tok   = $(shell grep -E "^$(1)=" $(lib_LTLIBRARIES) | cut -d "'" -f 2)
base      = $(call get_tok,libdir)
so        = $(call get_tok,library_names)
a         = $(call get_tok,old_library)

if HAVE_WIN32
SO_PREFIX = (DLL)
dll       = $(call get_tok,dlname)
else
if HAVE_DARWIN
SO_PREFIX = (DY)
dll       =
else
SO_PREFIX = (SO)
dll       =
endif
endif
