COMPILERFLAGS = -Wall

if with_sharedlibs
COMPILERFLAGS += -DOPJ_EXPORTS
else
COMPILERFLAGS += -DOPJ_STATIC
endif

USERLIBS =
INCLUDES = -I.. -I. -I../libopenjpeg -I../common

if with_liblcms2
INCLUDES += @lcms2includes@
USERLIBS += @lcms2libs@
endif

if with_liblcms1
INCLUDES += @lcms1includes@
USERLIBS += @lcms1libs@
endif

bin_PROGRAMS = frames_to_mj2 mj2_to_frames extract_j2k_from_mj2 wrap_j2k_in_mj2

CFLAGS = $(COMPILERFLAGS) $(INCLUDES)
LDADD = ../libopenjpeg/libopenjpeg.la $(USERLIBS)

frames_to_mj2_SOURCES = ../common/getopt.c mj2_convert.c mj2.c \
 frames_to_mj2.c

mj2_to_frames_SOURCES = ../common/getopt.c mj2_convert.c mj2.c \
 ../common/color.c mj2_to_frames.c

extract_j2k_from_mj2_SOURCES = mj2.c extract_j2k_from_mj2.c

wrap_j2k_in_mj2_SOURCES = mj2.c wrap_j2k_in_mj2.c

REPBIN=$(bin_PROGRAMS)

all-local:
	$(INSTALL) -d ../bin
	$(INSTALL) $(bin_PROGRAMS) ../bin
	@echo "" > .report.txt
	@(for f in ${REPBIN} ; do \
	echo "Installing: ${prefix}/bin/$$f" >> .report.txt ; \
	done)
