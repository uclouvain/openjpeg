MAJOR = @MAJOR_NR@
MINOR = @MINOR_NR@
BUILD = @BUILD_NR@
CURRENT=`expr $(MAJOR) + $(MINOR)`

lib_LTLIBRARIES = libopenjpeg.la

INCLUDES = -I.. -I.

libopenjpeg_la_LDFLAGS = -no-undefined \
  -version-info $(CURRENT):$(BUILD):$(MINOR)

SRCS = bio.c cio.c dwt.c event.c image.c j2k.c j2k_lib.c jp2.c jpt.c \
 mct.c mqc.c openjpeg.c pi.c raw.c t1.c t2.c tcd.c tgt.c

INCLS = bio.h cio.h dwt.h event.h fix.h image.h int.h j2k.h j2k_lib.h \
 jp2.h jpt.h mct.h mqc.h pi.h raw.h t1.h t2.h tcd.h tgt.h opj_malloc.h \
 opj_includes.h

libopenjpeg_la_SOURCES = $(SRCS) $(INCLS)

includedir = ${prefix}/include/openjpeg-$(MAJOR).$(MINOR)
include_HEADERS = openjpeg.h

# Converts cr/lf to just lf
DOS2UNIX = dos2unix

dos2unix:
	@$(DOS2UNIX) $(SRCS) $(INCLS)

COMPILERFLAGS = -Wall -O3 -ffast-math -std=c99

if with_sharedlibs
COMPILERFLAGS += -DOPJ_EXPORTS
else
COMPILERFLAGS += -DOPJ_STATIC
libopenjpeg_la_LDFLAGS += -static
endif

CFLAGS = $(COMPILERFLAGS) $(INCLUDES)

headerdir = openjpeg-$(MAJOR).$(MINOR)

all-local:
	@$(INSTALL) -d ../bin
	@rm -rf .report_dir
	@$(INSTALL) -d .report_dir
if build_so
if with_sharedlibs
	@find .libs -type f -name "*\.so*" -exec cp -P {} .report_dir \;
	@find .libs -type l -name "*\.so*" -exec cp -P {} .report_dir \;
else !with_sharedlibs
	@find .libs -type f -name "*\.a" -exec cp -P {} .report_dir \;
endif !with_sharedlibs
endif
if build_dll
if with_sharedlibs
	@find .libs -type f -name "*\.dll" -exec cp -P {} .report_dir \;
	@find .libs -type f -name "*\.a" -exec cp -P {} .report_dir \;
else !with_sharedlibs
	@find .libs -type f -name "*\.a" -exec cp -P {} .report_dir \;
endif !with_sharedlibs
endif
if build_dylib
if with_sharedlibs
	@find .libs -type f -name "*\.dylib" -exec cp -P {} .report_dir \;
else !with_sharedlibs
	@find .libs -type l -name "*\.a" -exec cp -P {} .report_dir \;
endif !with_sharedlibs
endif
	@cp -a .report_dir/* ../bin
	@echo "" > .report.txt
	@echo "Installing: ${prefix}/include/${headerdir}/" >> .report.txt
	@echo "Installing: ${prefix}/include/${headerdir}/openjpeg.h" >> .report.txt
	@echo "Installing: ${prefix}/lib/libopenjpeg.la" >> .report.txt
	@(cd .report_dir; \
	for file in `ls *.dll 2> /dev/null` ; do \
	echo "Installing: ${prefix}/bin/$${file}" >> ../.report.txt ; \
	done ; \
	for file in `ls *.a 2> /dev/null` ; do \
	echo "Installing: ${prefix}/lib/$${file}" >> ../.report.txt ; \
	done ; \
	for file in `ls *.so* 2> /dev/null` ; do \
	echo "Installing: ${prefix}/lib/$${file}" >> ../.report.txt ; \
	done ; \
	for file in `ls *.dylib 2> /dev/null` ; do \
	echo "Installing: ${prefix}/lib/$${file}" >> ../.report.txt ; \
	done \
	)
	@rm -rf .report_dir

install-data-hook:
	(cd $(DESTDIR)${prefix}/include && rm -f openjpeg.h)
	(cd $(DESTDIR)${prefix}/include && \
	$(LN_S) ${headerdir}/openjpeg.h openjpeg.h)
	
uninstall-hook:
	(cd $(DESTDIR)${prefix}/include && rm -rf openjpeg.h ${headerdir})
