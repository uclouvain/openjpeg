# Linux makefile for JP3DVM

VER_MAJOR = 2
VER_MINOR = 1.3.0

SRCS = ./libjp3dvm/bio.c ./libjp3dvm/cio.c ./libjp3dvm/dwt.c ./libjp3dvm/event.c ./libjp3dvm/jp3d.c ./libjp3dvm/jp3d_lib.c ./libjp3dvm/volume.c ./libjp3dvm/mct.c ./libjp3dvm/mqc.c ./libjp3dvm/openjpeg.c ./libjp3dvm/pi.c ./libjp3dvm/raw.c ./libjp3dvm/t1.c ./libjp3dvm/t1_3d.c ./libjp3dvm/t2.c ./libjp3dvm/tcd.c ./libjp3dvm/tgt.c
INCLS = ./libjp3dvm/t1_3d.h ./libjp3dvm/bio.h ./libjp3dvm/cio.h ./libjp3dvm/dwt.h ./libjp3dvm/event.h ./libjp3dvm/fix.h ./libjp3dvm/int.h ./libjp3dvm/jp3d.h ./libjp3dvm/jp3d_lib.h ./libjp3dvm/volume.h ./libjp3dvm/mct.h ./libjp3dvm/mqc.h ./libjp3dvm/openjpeg.h ./libjp3dvm/pi.h ./libjp3dvm/raw.h ./libjp3dvm/t1.h  ./libjp3dvm/t2.h ./libjp3dvm/tcd.h ./libjp3dvm/tgt.h ./libjp3dvm/opj_includes.h
INCLUDE = -Ilibjp3dvm

# General configuration variables:
CC = gcc
AR = ar

PREFIX = /usr
INSTALL_LIBDIR = $(PREFIX)/lib
INSTALL_INCLUDE = $(PREFIX)/include

# Converts cr/lf to just lf
DOS2UNIX = dos2unix

COMPILERFLAGS = -Wall -O3 -ffast-math -std=c99 -fPIC
LIBRARIES = -lstdc++

MODULES = $(SRCS:.c=.o)
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)

TARGET  = jp3dvm
STATICLIB = lib$(TARGET).a
SHAREDLIB = lib$(TARGET)-$(VER_MAJOR).$(VER_MINOR).so
LIBNAME = lib$(TARGET).so.$(VER_MAJOR)



default: all

all: Jp3dVM

dist: Jp3dVM
	install -d dist
	install -m 644 $(STATICLIB) dist
	install -m 755 $(SHAREDLIB) dist
	ln -sf $(SHAREDLIB) dist/$(LIBNAME)
	install libjp3dvm/openjpeg.h dist

dos2unix:
	@$(DOS2UNIX) $(SRCS) $(INCLS)

Jp3dVM: $(STATICLIB) $(SHAREDLIB)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(STATICLIB): $(MODULES)
	$(AR) r $@ $(MODULES)

$(SHAREDLIB): $(MODULES)
	$(CC) -s -shared -Wl,-soname,$(LIBNAME) -o $@ $(MODULES) $(LIBRARIES)

install: Jp3dVM
	install -d '$(DESTDIR)$(INSTALL_LIBDIR)' '$(DESTDIR)$(INSTALL_INCLUDE)'
	install -m 644 -o root -g root $(STATICLIB) '$(DESTDIR)$(INSTALL_LIBDIR)'
	ranlib '$(DESTDIR)$(INSTALL_LIBDIR)/$(STATICLIB)'
	install -m 755 -o root -g root $(SHAREDLIB) '$(DESTDIR)$(INSTALL_LIBDIR)'
	ln -sf $(SHAREDLIB) '$(DESTDIR)$(INSTALL_LIBDIR)/$(LIBNAME)'
	install -m 644 -o root -g root libjp3dvm/openjpeg.h '$(DESTDIR)$(INSTALL_INCLUDE)'
	-ldconfig

clean:
	rm -rf core dist/ u2dtmp* $(MODULES) $(STATICLIB) $(SHAREDLIB) $(LIBNAME)

osx:
	make -f Makefile.osx

osxinstall:
	make -f Makefile.osx install

osxclean:
	make -f Makefile.osx clean
