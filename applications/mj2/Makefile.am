MAINTAINERCLEANFILES = Makefile.in

OPJ_SRC = \
../../libopenjpeg/bio.c \
../../libopenjpeg/cio.c \
../../libopenjpeg/dwt.c \
../../libopenjpeg/event.c \
../../libopenjpeg/image.c \
../../libopenjpeg/j2k.c \
../../libopenjpeg/j2k_lib.c \
../../libopenjpeg/jp2.c \
../../libopenjpeg/jpt.c \
../../libopenjpeg/mct.c \
../../libopenjpeg/mqc.c \
../../libopenjpeg/openjpeg.c \
../../libopenjpeg/pi.c \
../../libopenjpeg/raw.c \
../../libopenjpeg/t1.c \
../../libopenjpeg/t2.c \
../../libopenjpeg/tcd.c \
../../libopenjpeg/tgt.c

bin_PROGRAMS = \
frames_to_mj2 \
mj2_to_frames \
extract_j2k_from_mj2 \
wrap_j2k_in_mj2

frames_to_mj2_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/common \
-I$(top_builddir)/applications/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
-DOPJ_STATIC
frames_to_mj2_CFLAGS = 
frames_to_mj2_LDADD = -lm
frames_to_mj2_SOURCES = \
$(OPJ_SRC) \
../common/getopt.c \
mj2_convert.c \
mj2.c \
frames_to_mj2.c \
mj2.h \
mj2_convert.h

mj2_to_frames_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/common \
-I$(top_builddir)/applications/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
@LCMS2_CFLAGS@ \
@LCMS1_CFLAGS@ \
-DOPJ_STATIC
mj2_to_frames_CFLAGS = 
mj2_to_frames_LDADD = @LCMS2_LIBS@ @LCMS1_LIBS@
mj2_to_frames_SOURCES = \
$(OPJ_SRC) \
../common/color.c \
../common/getopt.c \
mj2_convert.c \
mj2.c \
mj2_to_frames.c \
mj2.h \
mj2_convert.h

extract_j2k_from_mj2_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/common \
-I$(top_builddir)/applications/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
-DOPJ_STATIC
extract_j2k_from_mj2_CFLAGS = 
extract_j2k_from_mj2_LDADD = -lm
extract_j2k_from_mj2_SOURCES = \
$(OPJ_SRC) \
mj2.c \
extract_j2k_from_mj2.c \
mj2.h

wrap_j2k_in_mj2_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/common \
-I$(top_builddir)/applications/common \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg \
-DOPJ_STATIC
wrap_j2k_in_mj2_CFLAGS = 
wrap_j2k_in_mj2_LDADD = -lm
wrap_j2k_in_mj2_SOURCES = \
$(OPJ_SRC) \
mj2.c \
wrap_j2k_in_mj2.c \
mj2.h

EXTRA_DIST = \
CMakeLists.txt

install-data-hook:
	@echo -e " (B)\t$(bindir)/frames_to_mj2$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/mj2_to_frames$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/extract_j2k_from_mj2$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/wrap_j2k_in_mj2$(EXEEXT)" >> $(top_builddir)/report.txt
