#libjp3dvm
#
MAJOR = @JP3D_MAJOR_NR@
MINOR = @JP3D_MINOR_NR@
BUILD = @JP3D_BUILD_NR@
CURRENT=`expr $(MAJOR) + $(MINOR)`

lib_LTLIBRARIES = libopenjp3dvm.la

libopenjp3dvm_la_LDFLAGS = -no-undefined \
  -version-info $(CURRENT):$(BUILD):$(MINOR)

SRCS = bio.c cio.c dwt.c event.c jp3d.c jp3d_lib.c volume.c mct.c mqc.c \
  openjpeg.c pi.c raw.c t1.c t1_3d.c t2.c tcd.c tgt.c

INCLS = t1_3d.h bio.h cio.h  dwt.h event.h fix.h int.h jp3d.h jp3d_lib.h \
  volume.h mct.h mqc.h pi.h raw.h t1.h  t2.h tcd.h tgt.h opj_includes.h

libopenjp3dvm_la_SOURCES = $(SRCS) $(INCLS)

includedir = ${prefix}/include/openjpeg3d-$(MAJOR).$(MINOR)
include_HEADERS = openjpeg3d.h

# Converts cr/lf to just lf
DOS2UNIX = dos2unix

dos2unix:
	@$(DOS2UNIX) $(SRCS) $(INCLS)

INCLUDES = -I../.. -I.
COMPILERFLAGS = -Wall -O3 -ffast-math -std=c99

if with_sharedlibs
COMPILERFLAGS += -DOPJ_EXPORTS
else
COMPILERFLAGS += -DOPJ_STATIC
libopenjp3dvm_la_LDFLAGS += -static
endif

CFLAGS = $(COMPILERFLAGS) $(INCLUDES) 
#CFLAGS += -DJP3D_VERSION="\"$(MAJOR).$(MINOR).$(BUILD)\""

headerdir = openjpeg3d-$(MAJOR).$(MINOR)

all-local:
	@$(INSTALL) -d ../../bin
	@cp -P .libs/libopenjp3dvm.* ../../bin
	@rm -f ../../bin/libopenjp3dvm.la*
	@rm -rf .report_lib
	@$(INSTALL) -d .report_lib
	@cp -P .libs/libopenjp3dvm.* .report_lib
	@rm -f .report_lib/libopenjp3dvm.lai
	@echo "" > ../.report.txt
	@echo "Installing: ${prefix}/include/${headerdir}/" >> ../.report.txt
	@echo "Installing: ${prefix}/include/${headerdir}/openjpeg3d.h" >> ../.report.txt
	@(cd .report_lib; \
	l=`ls --file-type`; \
	for f in $$l ; do \
	echo "Installing: ${prefix}/lib/$$f" >> ../../.report.txt ; \
	done)
	@rm -rf .report_lib

install-data-hook:
	(cd $(DESTDIR)${prefix}/include && rm -f openjpeg3d.h)
	(cd $(DESTDIR)${prefix}/include && \
	$(LN_S) ${headerdir}/openjpeg3d.h openjpeg3d.h)

uninstall-hook:
	(cd $(DESTDIR)${prefix}/include && rm -rf openjpeg3d.h ${headerdir})
