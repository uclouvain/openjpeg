MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = opj_dec_server

opj_dec_server_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/opj_server \
-I$(top_builddir)/applications/jpip/opj_server \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip \
@FCGI_CFLAGS@ \
-DSERVER
opj_dec_server_CFLAGS = 
opj_dec_server_LDADD = $(top_builddir)/applications/jpip/libopenjpip/libopenjpip_server.la $(top_builddir)/libopenjpeg/libopenjpeg.la @FCGI_LIBS@ -lm
opj_dec_server_SOURCES = \
cache_manager.c \
cache_manager.h \
imgsock_manager.c \
imgsock_manager.h \
jp2k_decoder.c \
jp2k_decoder.h \
jptstream_manager.c \
jptstream_manager.h \
opj_dec_server.c


install-data-hook:
	@echo -e " (B)\t$(bindir)/opj_dec_server$(EXEEXT)" >> $(top_builddir)/report.txt
