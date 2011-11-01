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
channel_manager.c \
opj_server.c \
query_parser.c \
session_manager.c \
jpip_parser.c \
channel_manager.h \
query_parser.h \
session_manager.h \
jpip_parser.h

install-data-hook:
	@echo -e " (B)\t$(bindir)/opj_server$(EXEEXT)" >> $(top_builddir)/report.txt
