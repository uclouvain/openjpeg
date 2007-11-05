# Linux makefile for OpenJPEG + JPWL

VER_MAJOR = 2
VER_MINOR = 1.2.0

JPWL_SRCS = ./crc.c ./jpwl.c ./jpwl_lib.c ./rs.c
SRCS = ../libopenjpeg/bio.c ../libopenjpeg/cio.c ../libopenjpeg/dwt.c ../libopenjpeg/event.c ../libopenjpeg/image.c ../libopenjpeg/j2k.c ../libopenjpeg/j2k_lib.c ../libopenjpeg/jp2.c ../libopenjpeg/jpt.c ../libopenjpeg/mct.c ../libopenjpeg/mqc.c ../libopenjpeg/openjpeg.c ../libopenjpeg/pi.c ../libopenjpeg/raw.c ../libopenjpeg/t1.c ../libopenjpeg/t2.c ../libopenjpeg/tcd.c ../libopenjpeg/tgt.c $(JPWL_SRCS)
INCLS = ../libopenjpeg/bio.h ../libopenjpeg/cio.h ../libopenjpeg/dwt.h ../libopenjpeg/event.h ../libopenjpeg/fix.h ../libopenjpeg/image.h ../libopenjpeg/int.h ../libopenjpeg/j2k.h ../libopenjpeg/j2k_lib.h ../libopenjpeg/jp2.h ../libopenjpeg/jpt.h ../libopenjpeg/mct.h ../libopenjpeg/mqc.h ../libopenjpeg/openjpeg.h ../libopenjpeg/pi.h ../libopenjpeg/raw.h ../libopenjpeg/t1.h ../libopenjpeg/t2.h ../libopenjpeg/tcd.h ../libopenjpeg/tgt.h ../libopenjpeg/opj_malloc.h ../libopenjpeg/opj_includes.h
INCLUDE = -Ilibopenjpeg_JPWL

# General configuration variables:
CC = gcc
AR = ar

PREFIX = /usr
INSTALL_LIBDIR = $(PREFIX)/lib
INSTALL_INCLUDE = $(PREFIX)/include

# Converts cr/lf to just lf
DOS2UNIX = dos2unix

COMPILERFLAGS = -O3 -fPIC
LIBRARIES = -lstdc++

MODULES = $(SRCS:.c=.o)
CFLAGS = $(COMPILERFLAGS) $(INCLUDE) -DUSE_JPWL

TARGET  = openjpeg_JPWL
STATICLIB = lib$(TARGET).a
SHAREDLIB = lib$(TARGET)-$(VER_MAJOR).$(VER_MINOR).so
LIBNAME = lib$(TARGET).so.$(VER_MAJOR)



default: all

all: OpenJPEG_JPWL JPWL_image_to_j2k JPWL_j2k_to_image

dist: OpenJPEG_JPWL
	install -d dist
	install -m 644 $(STATICLIB) dist
	install -m 755 $(SHAREDLIB) dist
	ln -sf $(SHAREDLIB) dist/$(LIBNAME)
	install libopenjpeg/openjpeg.h dist

dos2unix:
	@$(DOS2UNIX) $(SRCS) $(INCLS)

OpenJPEG_JPWL: $(STATICLIB) $(SHAREDLIB)

JPWL_codec: JPWL_j2k_to_image JPWL_image_to_j2k $(STATICLIB)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(STATICLIB): $(MODULES)
	$(AR) r $@ $(MODULES)

$(SHAREDLIB): $(MODULES)
	$(CC) -s -shared -Wl,-soname,$(LIBNAME) -o $@ $(MODULES) $(LIBRARIES)

JPWL_j2k_to_image: ../codec/j2k_to_image.c
	gcc $(CFLAGS) ../codec/convert.c ../codec/j2k_to_image.c -o JPWL_j2k_to_image -I ../libopenjpeg/ -L . -lopenjpeg_JPWL -lm -ltiff

JPWL_image_to_j2k: ../codec/image_to_j2k.c
	gcc $(CFLAGS) ../codec/convert.c ../codec/image_to_j2k.c -o JPWL_image_to_j2k -I ../libopenjpeg/ -L . -lopenjpeg_JPWL -lm -ltiff

install: OpenJPEG
	install -d '$(DESTDIR)$(INSTALL_LIBDIR)' '$(DESTDIR)$(INSTALL_INCLUDE)'
	install -m 644 -o root -g root $(STATICLIB) '$(DESTDIR)$(INSTALL_LIBDIR)'
	ranlib '$(DESTDIR)$(INSTALL_LIBDIR)/$(STATICLIB)'
	install -m 755 -o root -g root $(SHAREDLIB) '$(DESTDIR)$(INSTALL_LIBDIR)'
	ln -sf $(SHAREDLIB) '$(DESTDIR)$(INSTALL_LIBDIR)/$(LIBNAME)'
	install -m 644 -o root -g root libopenjpeg/openjpeg.h '$(DESTDIR)$(INSTALL_INCLUDE)'
	-ldconfig

cleanlib:
	rm -rf core dist/ u2dtmp* $(MODULES) $(STATICLIB) $(SHAREDLIB) $(LIBNAME)

cleancodec:
	rm -f JPWL_j2k_to_image JPWL_image_to_j2k JPWL_j2k_to_image.o JPWL_image_to_j2k.o

clean: cleanlib cleancodec