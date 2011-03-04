
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = jp3d_to_volume volume_to_jp3d
LDADD = $(USERLIBS) ../libjp3dvm/libopenjp3dvm.la

jp3d_to_volume_CPPFLAGS =
jp3d_to_volume_CFLAGS =
jp3d_to_volume_LDADD = $(top_builddir)/jp3d/libjp3dvm/libopenjp3dvm.la -lm
jp3d_to_volume_SOURCES = getopt.c convert.c jp3d_to_volume.c convert.h

volume_to_jp3d_CPPFLAGS =
volume_to_jp3d_CFLAGS =
volume_to_jp3d_LDADD = $(top_builddir)/jp3d/libjp3dvm/libopenjp3dvm.la
volume_to_jp3d_SOURCES = getopt.c convert.c volume_to_jp3d.c convert.h

EXTRA_DIST = \
CMakeLists.txt \
windirent.h

install-data-hook:
	@echo -e " (B)\t$(bindir)/jp3d_to_volume$(EXEEXT)" >> $(top_builddir)/report.txt
	@echo -e " (B)\t$(bindir)/volume_to_jp3d$(EXEEXT)" >> $(top_builddir)/report.txt
