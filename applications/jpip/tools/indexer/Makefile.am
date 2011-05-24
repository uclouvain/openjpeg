MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = index_create

index_create_CPPFLAGS = \
-I. \
-I$(top_srcdir)/applications/libopenjpip/tools/indexer \
-I$(top_builddir)/applications/libopenjpip/tools/indexer
index_create_CFLAGS = 
index_create_LDADD = -lm
index_create_SOURCES = \
bio.c \
bio.h \
cio.c \
cio.h \
index_create.c \
int.c \
int.h \
j2k.h \
jp2.c \
jp2.h \
jpip.c \
jpip.h \
pi.c \
pi.h \
t2.c \
t2.h \
tcd.c \
tcd.h \
tgt.c \
tgt.h

install-data-hook:
	@echo -e " (B)\t$(bindir)/index_create$(EXEEXT)" >> $(top_builddir)/report.txt
