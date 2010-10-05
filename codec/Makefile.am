COMPILERFLAGS = -Wall
USERLIBS = -lm
INCLUDES = -I.. -I. -I../libopenjpeg

if with_libjpwl
USERLIBS += -L../jpwl -lopenjpeg_JPWL
COMPILERFLAGS += -DUSE_JPWL
endif

if with_libtiff
INCLUDES += @tiffincludes@
USERLIBS += @tifflibs@
endif

if with_libpng
INCLUDES += @pngincludes@
USERLIBS += @pnglibs@
endif

if with_liblcms2
INCLUDES += @lcms2includes@
USERLIBS += @lcms2libs@
endif

if with_liblcms1
INCLUDES += @lcms1includes@
USERLIBS += @lcms1libs@
endif

bin_PROGRAMS = j2k_to_image image_to_j2k j2k_dump

CFLAGS = $(COMPILERFLAGS) $(INCLUDES)
LDADD = $(USERLIBS) ../libopenjpeg/libopenjpeg.la

j2k_to_image_SOURCES = compat/getopt.c index.c convert.c j2k_to_image.c

image_to_j2k_SOURCES = compat/getopt.c index.c convert.c image_to_j2k.c

j2k_dump_SOURCES = compat/getopt.c index.c j2k_dump.c

REPBIN=$(bin_PROGRAMS)

all-local:
	$(INSTALL) -d ../bin
	$(INSTALL) $(bin_PROGRAMS) ../bin
	@echo "" > .report.txt
	@for f in ${REPBIN} ; do \
	echo "Installing: ${prefix}/bin/$$f" >> .report.txt ; \
	done
