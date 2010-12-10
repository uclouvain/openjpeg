#jp3d Makefile
include ../config.nix

TARGET  = openjp3dvm
COMPILERFLAGS = -O3 -Wall -ffast-math -std=c99 -fPIC

SRCS = ./libjp3dvm/bio.c ./libjp3dvm/cio.c ./libjp3dvm/dwt.c \
  ./libjp3dvm/event.c ./libjp3dvm/jp3d.c ./libjp3dvm/jp3d_lib.c \
  ./libjp3dvm/volume.c ./libjp3dvm/mct.c ./libjp3dvm/mqc.c \
  ./libjp3dvm/openjpeg.c ./libjp3dvm/pi.c ./libjp3dvm/raw.c \
  ./libjp3dvm/t1.c ./libjp3dvm/t1_3d.c ./libjp3dvm/t2.c \
  ./libjp3dvm/tcd.c ./libjp3dvm/tgt.c

INCLS = ./libjp3dvm/t1_3d.h ./libjp3dvm/bio.h ./libjp3dvm/cio.h \
  ./libjp3dvm/dwt.h ./libjp3dvm/event.h ./libjp3dvm/fix.h \
  ./libjp3dvm/int.h ./libjp3dvm/jp3d.h ./libjp3dvm/jp3d_lib.h \
  ./libjp3dvm/volume.h ./libjp3dvm/mct.h ./libjp3dvm/mqc.h \
  ./libjp3dvm/openjpeg3d.h ./libjp3dvm/pi.h ./libjp3dvm/raw.h \
  ./libjp3dvm/t1.h  ./libjp3dvm/t2.h ./libjp3dvm/tcd.h \
  ./libjp3dvm/tgt.h ./libjp3dvm/opj_includes.h

INCLUDE = -I.. -Ilibjp3dvm

INSTALL_LIBDIR = $(prefix)/lib
headerdir = openjpeg3d-$(JP3D_MAJOR).$(JP3D_MINOR)
INSTALL_INCLUDE = $(prefix)/include/$(headerdir)

# Converts cr/lf to just lf
DOS2UNIX = dos2unix

LIBRARIES = -lstdc++

MODULES = $(SRCS:.c=.o)
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)

LIBNAME = lib$(TARGET)

ifeq ($(ENABLE_SHARED),yes)
SHAREDLIB = $(LIBNAME).so.$(JP3D_MAJOR).$(JP3D_MINOR).$(JP3D_BUILD)
else
STATICLIB = $(LIBNAME).a
endif

default: all

all: Jp3dVM
	make -C codec -f Makefile.nix all 
	install -d ../bin
ifeq ($(ENABLE_SHARED),yes)
	install -m 755 $(SHAREDLIB) ../bin
	(cd ../bin && \
	ln -sf $(SHAREDLIB) $(LIBNAME).so.$(JP3D_MAJOR).$(JP3D_MINOR))
	(cd ../bin && \
	ln -sf $(LIBNAME).so.$(JP3D_MAJOR).$(JP3D_MINOR) $(LIBNAME).so)
else
	install -m 644 $(STATICLIB) ../bin
endif

dos2unix:
	@$(DOS2UNIX) $(SRCS) $(INCLS)

Jp3dVM: $(STATICLIB) $(SHAREDLIB)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

ifeq ($(ENABLE_SHARED),yes)
$(SHAREDLIB): $(MODULES)
	$(CC) -s -shared -Wl,-soname,$(LIBNAME) -o $@ $(MODULES) $(LIBRARIES)
else
$(STATICLIB): $(MODULES)
	$(AR) r $@ $(MODULES)
endif

install: Jp3dVM
	install -d '$(DESTDIR)$(INSTALL_LIBDIR)'
ifeq ($(ENABLE_SHARED),yes)
	install -m 755 -o root -g root $(SHAREDLIB) '$(DESTDIR)$(INSTALL_LIBDIR)'
	(cd $(DESTDIR)$(INSTALL_LIBDIR) &&  \
	ln -sf $(SHAREDLIB) $(LIBNAME).so.$(JP3D_MAJOR).$(JP3D_MINOR) )
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && \
	ln -sf $(LIBNAME).so.$(JP3D_MAJOR).$(JP3D_MINOR) $(LIBNAME).so )
else
	install -m 644 -o root -g root $(STATICLIB) '$(DESTDIR)$(INSTALL_LIBDIR)'
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && ranlib $(STATICLIB))
endif
	install -d $(DESTDIR)$(INSTALL_INCLUDE)
	rm -f $(DESTDIR)$(INSTALL_INCLUDE)/openjpeg3d.h
	install -m 644 -o root -g root libjp3dvm/openjpeg3d.h \
	$(DESTDIR)$(INSTALL_INCLUDE)/openjpeg3d.h
	(cd $(DESTDIR)$(prefix)/include && \
	ln -sf $(headerdir)/openjpeg3d.h openjpeg3d.h)
	make -C codec -f Makefile.nix install

uninstall:
ifeq ($(ENABLE_SHARED),yes)
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && \
	rm -f $(LIBNAME).so $(LIBNAME).so.$(JP3D_MAJOR).$(JP3D_MINOR) $(SHAREDLIB))
else
	rm -f $(DESTDIR)$(INSTALL_LIBDIR)/$(STATICLIB)
endif
	rm -f $(DESTDIR)$(prefix)/include/openjpeg3d.h
	rm -rf $(DESTDIR)$(INSTALL_INCLUDE)
	make -C codec -f Makefile.nix uninstall

clean:
	rm -f core u2dtmp* $(MODULES) $(STATICLIB) $(SHAREDLIB)
	make -C codec -f Makefile.nix clean
