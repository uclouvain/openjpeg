MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libopenjp3dvm.la

includesdir = ${includedir}/openjpeg3d-@JP3D_MAJOR_NR@.@JP3D_MINOR_NR@
includes_HEADERS = openjpeg3d.h

libopenjp3dvm_la_LDFLAGS = -no-undefined -version-info @lt_version@

libopenjp3dvm_la_CPPFLAGS = \
-I. \
-I$(top_srcdir)/jp3d/libjp3dvm \
-I$(top_builddir)/jp3d/libjp3dvm
libopenjp3dvm_la_CFLAGS =
libopenjp3dvm_la_LIBADD = -lm
libopenjp3dvm_la_SOURCES = \
bio.c \
cio.c \
dwt.c \
event.c \
jp3d.c \
jp3d_lib.c \
volume.c \
mct.c \
mqc.c \
openjpeg.c \
pi.c \
raw.c \
t1.c \
t1_3d.c \
t2.c \
tcd.c \
tgt.c \
bio.h \
cio.h \
dwt.h \
event.h \
fix.h \
int.h \
jp3d.h \
jp3d_lib.h \
mct.h \
mqc.h \
opj_includes.h \
pi.h \
raw.h \
t1.h \
t1_3d.h \
t2.h \
tcd.h \
tgt.h \
volume.h

EXTRA_DIST = \
CMakeLists.txt

install-data-hook:
	cd $(DESTDIR)$(includedir) && \
	  rm -f openjpeg3d.h && \
	  $(LN_S) openjpeg3d-@JP3D_MAJOR_NR@.@JP3D_MINOR_NR@/openjpeg3d.h \
	          openjpeg3d.h
	@echo -e " (LA)\t$(libdir)/libopenjp3dvm.la" >> $(top_builddir)/report.txt
	@( $(call solist) ) >> $(top_builddir)/report.txt
	@echo -e " (A)\t\t$(base)/$(a)" >> $(top_builddir)/report.txt
	@echo -e " (H)\t\t$(includedir)/openjpeg3d-$(MAJOR_NR).$(MINOR_NR)/openjpeg3d.h" >> $(top_builddir)/report.txt
	@echo -e " (LN)\t$(includedir)/openjpeg3d.h" >> $(top_builddir)/report.txt

uninstall-hook:
	rm -f $(DESTDIR)$(includedir)/openjpeg3d.h

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
