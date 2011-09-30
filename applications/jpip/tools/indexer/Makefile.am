MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = j2k_to_idxjp2

j2k_to_idxjp2_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/libopenjpip/tools/indexer \
-I$(top_builddir)/applications/libopenjpip/tools/indexer \
-I$(top_srcdir)/libopenjpeg \
-I$(top_builddir)/libopenjpeg
j2k_to_idxjp2_CFLAGS = 
j2k_to_idxjp2_LDADD = $(top_builddir)/libopenjpeg/libopenjpeg.la -lm
j2k_to_idxjp2_SOURCES = \
ext_libopenjpeg/cidx_manager.c \
ext_libopenjpeg/cidx_manager.h \
ext_libopenjpeg/cio_ext.c \
ext_libopenjpeg/cio_ext.h \
ext_libopenjpeg/ext_j2k.c \
ext_libopenjpeg/ext_jp2.c \
ext_libopenjpeg/ext_openjpeg.h \
ext_libopenjpeg/indexbox_manager.h \
j2k_to_idxjp2.c \
j2k_to_idxjp2.h \
event_mgr_handler.c \
event_mgr_handler.h \
idxjp2_manager.c \
j2k_decoder.c \
indexbox_manager.h \
ext_libopenjpeg/phix_manager.c \
ext_libopenjpeg/ppix_manager.c \
ext_libopenjpeg/thix_manager.c \
ext_libopenjpeg/tpix_manager.c

install-data-hook:
	@echo -e " (B)\t$(bindir)/j2k_to_idxjp2$(EXEEXT)" >> $(top_builddir)/report.txt

