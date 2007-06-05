# MacOSX makefile for OpenJPEG

VER_MAJOR = 2
VER_MINOR = 1.2.0

SRCS = ./libopenjpeg/bio.c ./libopenjpeg/cio.c ./libopenjpeg/dwt.c ./libopenjpeg/event.c ./libopenjpeg/image.c ./libopenjpeg/j2k.c ./libopenjpeg/j2k_lib.c ./libopenjpeg/jp2.c ./libopenjpeg/jpt.c ./libopenjpeg/mct.c ./libopenjpeg/mqc.c ./libopenjpeg/openjpeg.c ./libopenjpeg/pi.c ./libopenjpeg/raw.c ./libopenjpeg/t1.c ./libopenjpeg/t2.c ./libopenjpeg/tcd.c ./libopenjpeg/tgt.c
INCLS = ./libopenjpeg/bio.h ./libopenjpeg/cio.h ./libopenjpeg/dwt.h ./libopenjpeg/event.h ./libopenjpeg/fix.h ./libopenjpeg/image.h ./libopenjpeg/int.h ./libopenjpeg/j2k.h ./libopenjpeg/j2k_lib.h ./libopenjpeg/jp2.h ./libopenjpeg/jpt.h ./libopenjpeg/mct.h ./libopenjpeg/mqc.h ./libopenjpeg/openjpeg.h ./libopenjpeg/pi.h ./libopenjpeg/raw.h ./libopenjpeg/t1.h ./libopenjpeg/t2.h ./libopenjpeg/tcd.h ./libopenjpeg/tgt.h ./libopenjpeg/opj_includes.h
INCLUDE = -Ilibopenjpeg

# General configuration variables:
CC = gcc
LIBTOOLSTAT = libtool
LIBTOOLDYN = gcc

PREFIX = /usr
INSTALL_LIBDIR = $(PREFIX)/lib
INSTALL_INCLUDE = $(PREFIX)/include

COMPILERFLAGS = -O3 -fPIC

MODULES = $(SRCS:.c=.o)
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)

TARGET  = openjpeg
STATICLIB = lib$(TARGET).a
SHAREDLIB = lib$(TARGET)-$(VER_MAJOR).$(VER_MINOR).dylib
LIBNAME = lib$(TARGET).dylib



default: all

all: OpenJPEG

dos2unix:
	@$(DOS2UNIX) $(SRCS) $(INCLS)

dist: OpenJPEG
	install -d dist
	install -m 644 $(STATICLIB) dist
	install -m 755 $(SHAREDLIB) dist
	ln -sf $(SHAREDLIB) dist/$(LIBNAME)
	install libopenjpeg/openjpeg.h dist

OpenJPEG: $(STATICLIB) $(SHAREDLIB)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(STATICLIB): $(MODULES)
	$(LIBTOOLSTAT) -o $@ $(MODULES)

$(SHAREDLIB): $(MODULES)
	$(LIBTOOLDYN) -dynamiclib -o $@ $(MODULES)

install:
	install -d '$(DESTDIR)$(INSTALL_LIBDIR)' '$(DESTDIR)$(INSTALL_INCLUDE)'
	install -m 644 -o root -g wheel $(STATICLIB) '$(DESTDIR)$(INSTALL_LIBDIR)'
	ranlib '$(DESTDIR)$(INSTALL_LIBDIR)/$(STATICLIB)'
	install -m 755 -o root -g wheel $(SHAREDLIB) '$(DESTDIR)$(INSTALL_LIBDIR)'
	ln -sf $(SHAREDLIB) '$(DESTDIR)$(INSTALL_LIBDIR)/$(LIBNAME)'
	install -m 644 -o root -g wheel ./libopenjpeg/openjpeg.h '$(DESTDIR)$(INSTALL_INCLUDE)'

clean:
	rm -rf core dist/ u2dtmp* $(MODULES) $(STATICLIB) $(SHAREDLIB) $(LIBNAME)
	
