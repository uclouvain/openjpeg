#jp3d/codec/

USERLIBS =
INCLUDES = -I. -I../libjp3dvm
CFLAGS = -Wall $(INCLUDES)

if with_sharedlibs
CFLAGS += -DOPJ_EXPORTS
else
CFLAGS += -DOPJ_STATIC
endif

bin_PROGRAMS = jp3d_to_volume volume_to_jp3d
LDADD = $(USERLIBS) ../libjp3dvm/libopenjp3dvm.la

jp3d_to_volume_SOURCES = getopt.c convert.c jp3d_to_volume.c

volume_to_jp3d_SOURCES = getopt.c convert.c volume_to_jp3d.c

REPBIN=$(bin_PROGRAMS)

all-local:
	$(INSTALL) -d ../../bin
	$(INSTALL) $(bin_PROGRAMS) ../../bin
	@(for f in ${REPBIN} ; do \
	echo "Installing: ${prefix}/bin/$$f" >> ../.report.txt ; \
	done)
