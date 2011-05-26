DISTCHECK_CONFIGURE_FLAGS = --enable-jpip=yes --enable-jpwl=yes --enable-mj2=yes

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = libopenjpeg applications doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libopenjpeg1.pc


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
CMake/CheckHaveGetopt.cmake \
CMake/OpenJPEGConfig.cmake.in \
testing/CTestCustom.cmake.in \
testing/CTestConfig.cmake \
CMakeLists.txt \
LICENSE \
CHANGES \
README \
INSTALL \
bootstrap.sh \
libopenjpeg1.pc.cmake \
libopenjpeg1.pc.in \
opj_configh.cmake.in \
opj_config.h.in.user \
m4/opj_check_lib.m4 \
m4/opj_doxygen.m4 \
m4/pkg.m4

.PHONY: doc

# Documentation

doc:
	@echo "entering doc/"
	@cd doc && make doc

install-data-hook:
	$(LN_S) -nf libopenjpeg1.pc $(DESTDIR)$(pkgconfigdir)/libopenjpeg.pc
	@echo -e " (PC)\t$(pkgconfigdir)/libopenjpeg1.pc" >> $(top_builddir)/report.txt
	@echo -e " (LN)\t$(pkgconfigdir)/libopenjpeg.pc" >> $(top_builddir)/report.txt
	@cat $(top_builddir)/report.txt
	@rm $(top_builddir)/report.txt

uninstall-hook:
	rm -f $(DESTDIR)$(pkgconfigdir)/libopenjpeg.pc
