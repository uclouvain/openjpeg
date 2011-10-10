MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = jpip_to_jp2 jpip_to_j2k test_index addXMLinJP2

jpip_to_jp2_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip \
-I$(top_srcdir)/applications/jpip/libopenjpip/tools \
-I$(top_builddir)/applications/jpip/libopenjpip/tools
jpip_to_jp2_CFLAGS = 
jpip_to_jp2_LDADD = $(top_builddir)/applications/jpip/libopenjpip/libopenjpip_local.la
jpip_to_jp2_SOURCES = jpip_to_jp2.c

jpip_to_j2k_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip \
-I$(top_srcdir)/applications/jpip/libopenjpip/tools \
-I$(top_builddir)/applications/jpip/libopenjpip/tools
jpip_to_j2k_CFLAGS = 
jpip_to_j2k_LDADD = $(top_builddir)/applications/jpip/libopenjpip/libopenjpip_local.la
jpip_to_j2k_SOURCES = jpip_to_j2k.c

test_index_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip \
-I$(top_srcdir)/applications/jpip/libopenjpip/tools \
-I$(top_builddir)/applications/jpip/libopenjpip/tools
test_index_CFLAGS = 
test_index_LDADD = $(top_builddir)/applications/jpip/libopenjpip/libopenjpip_local.la
test_index_SOURCES = test_index.c

addXMLinJP2_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/libopenjpip/tools \
-I$(top_builddir)/applications/jpip/libopenjpip/tools
addXMLinJP2_CFLAGS = 
addXMLinJP2_LDADD = $(top_builddir)/applications/jpip/libopenjpip/libopenjpip_local.la
addXMLinJP2_SOURCES = addXMLinJP2.c

install-data-hook:
	@echo -e " (B)\t$(bindir)/jpip_to_jp2$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/jpip_to_j2k$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/test_index$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/addXMLinJP2$(EXEEXT)" >> $(top_builddir)/report.txt
