MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = opj_server

opj_server_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/opj_server \
-I$(top_builddir)/applications/jpip/opj_server \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip \
@FCGI_CFLAGS@ \
-DSERVER \
-DQUIT_SIGNAL=\"quitJPIP\"
opj_server_CFLAGS = 
opj_server_LDADD = $(top_builddir)/applications/jpip/libopenjpip/libopenjpip_server.la @FCGI_LIBS@ -lm
opj_server_SOURCES = \
opj_server.c

install-data-hook:
	@echo -e " (B)\t$(bindir)/opj_server$(EXEEXT)" >> $(top_builddir)/report.txt
