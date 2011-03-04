DISTCHECK_CONFIGURE_FLAGS = --enable-jp3d=yes --enable-jpwl=yes --enable-mj2=yes

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = libopenjpeg codec

if WANT_JP3D
SUBDIRS += jp3d
endif

if WANT_JPWL
SUBDIRS += jpwl
endif

if WANT_MJ2
SUBDIRS += mj2
endif

SUBDIRS += doc

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
CMake/CTestCustom.cmake.in \
CMake/OpenJPEGConfig.cmake.in \
CMakeLists.txt \
CTestConfig.cmake \
CTestCustom.cmake.in \
CMakeLists.txt \
JavaOpenJPEG/java-jni/include/jawt.h \
JavaOpenJPEG/java-jni/include/jdwpTransport.h \
JavaOpenJPEG/java-jni/include/jni.h \
JavaOpenJPEG/java-jni/include/jvmdi.h \
JavaOpenJPEG/java-jni/include/jvmpi.h \
JavaOpenJPEG/java-jni/include/jvmti.h \
JavaOpenJPEG/java-jni/include/win32/jawt_md.h \
JavaOpenJPEG/java-jni/include/win32/jni_md.h \
JavaOpenJPEG/java-jni/lib/jvm.lib \
JavaOpenJPEG/JavaOpenJPEG.c \
JavaOpenJPEG/JavaOpenJPEGDecoder.c \
JavaOpenJPEG/java-sources/org/openJpeg/OpenJPEGJavaDecoder.java \
JavaOpenJPEG/java-sources/org/openJpeg/OpenJPEGJavaEncoder.java \
JavaOpenJPEG/org_openJpeg_OpenJPEGJavaDecoder.h \
JavaOpenJPEG/org_openJpeg_OpenJPEGJavaEncoder.h \
LICENSE \
CHANGES \
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
