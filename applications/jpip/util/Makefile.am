MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS =

if WANT_JPIP
bin_PROGRAMS += opj_dec_server test_index jpip_to_j2k jpip_to_jp2
endif

if WANT_JPIP_SERVER
bin_PROGRAMS += opj_server
endif

opj_server_CPPFLAGS = \
@FCGI_CFLAGS@ \
-I. \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip \
-DSERVER \
-DQUIT_SIGNAL=\"quitJPIP\"
#
opj_server_LDADD = $(top_builddir)/applications/jpip/libopenjpip/libopenjpip_server.la @FCGI_LIBS@ -lm
#
opj_server_SOURCES = opj_server.c

#-------------
opj_dec_server_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip
#
opj_dec_server_CFLAGS =
opj_dec_server_LDADD = $(top_builddir)/applications/jpip/libopenjpip/libopenjpip_local.la
opj_dec_server_SOURCES = opj_dec_server.c

#-------------
jpip_to_jp2_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip
#
jpip_to_jp2_CFLAGS =
jpip_to_jp2_LDADD = $(top_builddir)/applications/jpip/libopenjpip/libopenjpip_local.la
jpip_to_jp2_SOURCES = jpip_to_jp2.c

#-------------
jpip_to_j2k_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip
#
jpip_to_j2k_CFLAGS =
jpip_to_j2k_LDADD = $(top_builddir)/applications/jpip/libopenjpip/libopenjpip_local.la
jpip_to_j2k_SOURCES = jpip_to_j2k.c

#-------------
test_index_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip
#
test_index_CFLAGS =
test_index_LDADD = $(top_builddir)/applications/jpip/libopenjpip/libopenjpip_local.la
test_index_SOURCES = test_index.c

#-------------
install-data-hook:
if WANT_JPIP_SERVER
	@echo -e " (B)\t$(bindir)/opj_server$(EXEEXT)" >> $(top_builddir)/report.txt
endif
if WANT_JPIP
	@echo -e " (B)\t$(bindir)/opj_dec_server$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/jpip_to_jp2$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/jpip_to_j2k$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/test_index$(EXEEXT)" >> $(top_builddir)/report.txt
endif
