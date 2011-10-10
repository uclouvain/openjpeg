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
cidx_manager.c \
cidx_manager.h \
cio_ext.c \
cio_ext.h \
j2k_to_idxjp2.c \
j2k_to_idxjp2.h \
event_mgr_handler.c \
event_mgr_handler.h \
idxjp2_manager.c \
j2k_decoder.c \
indexbox_manager.h \
phix_manager.c \
ppix_manager.c \
thix_manager.c \
tpix_manager.c

install-data-hook:
	@echo -e " (B)\t$(bindir)/j2k_to_idxjp2$(EXEEXT)" >> $(top_builddir)/report.txt

