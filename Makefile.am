DISTCHECK_CONFIGURE_FLAGS = --enable-jpip=yes --enable-jpip-server=yes --enable-jpwl=yes --enable-mj2=yes

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = libopenjpeg applications doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libopenjpeg1.pc

if WANT_JPWL
pkgconfig_DATA += libopenjpeg-jpwl.pc
endif


MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
config.guess \
opj_config.h.in \
opj_config.h.in~ \
config.sub \
configure \
depcomp \
install-sh \
ltmain.sh \
missing

EXTRA_DIST = \
CMake/CTestCustom.cmake.in \
CMake/FindFCGI.cmake \
CMake/FindLCMS2.cmake \
CMake/FindLCMS.cmake \
CMake/OpenJPEGConfig.cmake.in \
CMakeLists.txt \
CTestConfig.cmake \
AUTHORS \
CHANGES \
INSTALL \
LICENSE \
NEWS \
README \
THANKS \
bootstrap.sh \
libopenjpeg1.pc.cmake \
libopenjpeg1.pc.in \
libopenjpeg-jpwl.pc.in \
opj_config.h.cmake.in \
m4/opj_check_lib.m4 \
m4/opj_doxygen.m4 \
m4/pkg.m4

.PHONY: doc

# Documentation

doc:
	@echo "entering doc/"
	@cd doc && make doc

install-data-hook:
if HAVE_WIN32
	$(LN_S) -f libopenjpeg1.pc $(DESTDIR)$(pkgconfigdir)/libopenjpeg.pc
else
	$(LN_S) -nf libopenjpeg1.pc $(DESTDIR)$(pkgconfigdir)/libopenjpeg.pc
endif
	@echo -e " (PC)\t$(pkgconfigdir)/libopenjpeg1.pc" >> $(top_builddir)/report.txt
	@echo -e " (LN)\t$(pkgconfigdir)/libopenjpeg.pc" >> $(top_builddir)/report.txt
if WANT_JPWL
	@echo -e " (PC)\t$(pkgconfigdir)/libopenjpeg-jpwl.pc" >> $(top_builddir)/report.txt
endif
	@cat $(top_builddir)/report.txt
	@rm $(top_builddir)/report.txt

uninstall-hook:
	rm -f $(DESTDIR)$(pkgconfigdir)/libopenjpeg.pc
