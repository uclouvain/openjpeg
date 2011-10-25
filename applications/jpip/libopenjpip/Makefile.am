MAINTAINERCLEANFILES = Makefile.in

includesdir = $(includedir)/openjpeg-$(MAJOR_NR).$(MINOR_NR)
includes_HEADERS = 

lib_LTLIBRARIES = libopenjpip_server.la libopenjpip_local.la

JPIP_SRC = \
openjpip.c \
query_parser.c \
channel_manager.c \
session_manager.c \
jpip_parser.c \
boxheader_manager.c \
codestream_manager.c \
imgreg_manager.c \
marker_manager.c \
msgqueue_manager.c \
box_manager.c \
faixbox_manager.c \
index_manager.c \
metadata_manager.c \
placeholder_manager.c \
byte_manager.c \
ihdrbox_manager.c \
manfbox_manager.c \
mhixbox_manager.c \
target_manager.c \
cachemodel_manager.c \
j2kheader_manager.c \
jp2k_encoder.c \
bool.h \
boxheader_manager.h \
box_manager.h \
byte_manager.h \
codestream_manager.h \
faixbox_manager.h \
ihdrbox_manager.h \
imgreg_manager.h \
index_manager.h \
manfbox_manager.h \
marker_manager.h \
metadata_manager.h \
mhixbox_manager.h \
msgqueue_manager.h \
placeholder_manager.h \
target_manager.h \
cachemodel_manager.h \
j2kheader_manager.h \
jp2k_encoder.h \
query_parser.h \
channel_manager.h \
session_manager.h \
jpip_parser.h

LOCAL_SRC = jp2k_decoder.c \
imgsock_manager.c \
jpipstream_manager.c \
cache_manager.c \
dec_clientmsg_handler.c \
imgsock_manager.h \
jpipstream_manager.h \
cache_manager.h \
dec_clientmsg_handler.h

libopenjpip_server_la_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip \
@FCGI_CFLAGS@ \
-DSERVER
libopenjpip_server_la_CFLAGS =
libopenjpip_server_la_LIBADD = @FCGI_LIBS@ -lm
libopenjpip_server_la_LDFLAGS = -no-undefined -version-info @lt_version@
libopenjpip_server_la_SOURCES = $(JPIP_SRC)

libopenjpip_local_la_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/jpip/libopenjpip \
-I$(top_builddir)/applications/jpip/libopenjpip
libopenjpip_local_la_CFLAGS =
libopenjpip_local_la_LIBADD = -lm
libopenjpip_local_la_LDFLAGS = -no-undefined -version-info @lt_version@
libopenjpip_local_la_SOURCES = $(JPIP_SRC) $(LOCAL_SRC)

install-data-hook:
	@echo -e " (LA)\t$(libdir)/libopenjpip_server.la" >> $(top_builddir)/report.txt
if BUILD_SHARED
	@( $(call solist_s) ) >> $(top_builddir)/report.txt
endif
if BUILD_STATIC
	@echo -e " (A)\t$(base_s)/$(a_s)" >> $(top_builddir)/report.txt
endif
	@echo -e " (LA)\t$(libdir)/libopenjpip_local.la" >> $(top_builddir)/report.txt
if BUILD_SHARED
	@( $(call solist_c) ) >> $(top_builddir)/report.txt
endif
if BUILD_STATIC
	@echo -e " (A)\t$(base_c)/$(a_c)" >> $(top_builddir)/report.txt
endif

solist_s    = $(foreach f, $(dll_s) $(so_s), echo -e ' $(SO_PREFIX)\t$(base_s)/$(f)' ;)
get_tok_s   = $(shell grep -E "^$(1)=" libopenjpip_server.la | cut -d "'" -f 2)
base_s      = $(call get_tok_s,libdir)
so_s        = $(call get_tok_s,library_names)
a_s         = $(call get_tok_s,old_library)

solist_c    = $(foreach f, $(dll_c) $(so_c), echo -e ' $(SO_PREFIX)\t$(base_c)/$(f)' ;)
get_tok_c   = $(shell grep -E "^$(1)=" libopenjpip_local.la | cut -d "'" -f 2)
base_c      = $(call get_tok_c,libdir)
so_c        = $(call get_tok_c,library_names)
a_c         = $(call get_tok_c,old_library)

if HAVE_WIN32
SO_PREFIX = (DLL)
dll_s     = $(call get_tok_s,dlname)
dll_c     = $(call get_tok_c,dlname)
else
if HAVE_DARWIN
SO_PREFIX = (DY)
dll_s     =
dll_c     =
else
SO_PREFIX = (SO)
dll_s     =
dll_c     =
endif
endif
