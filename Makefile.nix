#top Makefile.nix
include  config.nix

TARGET  = openjpeg
COMPILERFLAGS = -Wall -O3 -ffast-math -std=c99 -fPIC

INCLUDE = -I. -Ilibopenjpeg
LIBRARIES = -lstdc++

SRCS = ./libopenjpeg/bio.c ./libopenjpeg/cio.c ./libopenjpeg/dwt.c \
 ./libopenjpeg/event.c ./libopenjpeg/image.c ./libopenjpeg/j2k.c \
 ./libopenjpeg/j2k_lib.c ./libopenjpeg/jp2.c ./libopenjpeg/jpt.c \
 ./libopenjpeg/mct.c ./libopenjpeg/mqc.c ./libopenjpeg/openjpeg.c \
 ./libopenjpeg/pi.c ./libopenjpeg/raw.c ./libopenjpeg/t1.c \
 ./libopenjpeg/t2.c ./libopenjpeg/tcd.c ./libopenjpeg/tgt.c \
 ./libopenjpeg/opj_convert.c

INCLS = ./libopenjpeg/bio.h ./libopenjpeg/cio.h ./libopenjpeg/dwt.h \
 ./libopenjpeg/event.h ./libopenjpeg/fix.h ./libopenjpeg/image.h \
 ./libopenjpeg/int.h ./libopenjpeg/j2k.h ./libopenjpeg/j2k_lib.h \
 ./libopenjpeg/jp2.h ./libopenjpeg/jpt.h ./libopenjpeg/mct.h \
 ./libopenjpeg/mqc.h ./libopenjpeg/openjpeg.h ./libopenjpeg/pi.h \
 ./libopenjpeg/raw.h ./libopenjpeg/t1.h ./libopenjpeg/t2.h \
 ./libopenjpeg/tcd.h ./libopenjpeg/tgt.h ./libopenjpeg/opj_malloc.h \
 ./libopenjpeg/opj_convert.h ./libopenjpeg/opj_includes.h

AR = ar

INSTALL_LIBDIR = $(prefix)/lib
headerdir = openjpeg-$(MAJOR).$(MINOR)
INSTALL_INCLUDE = $(prefix)/include/$(headerdir)

# Converts cr/lf to just lf
DOS2UNIX = dos2unix

ifeq ($(WITH_LCMS1),yes)
INCLUDE += $(LCMS1_INCLUDE)
LIBRARIES += $(LCMS1_LIB)
endif

ifeq ($(WITH_LCMS2),yes)
INCLUDE += $(LCMS2_INCLUDE)
LIBRARIES += $(LCMS2_LIB)
endif

MODULES = $(SRCS:.c=.o)

CFLAGS = $(COMPILERFLAGS) $(INCLUDE)

LIBNAME = lib$(TARGET)
STATICLIB = $(LIBNAME).a

ifeq ($(ENABLE_SHARED),yes)
SHAREDLIB = $(LIBNAME).so.$(MAJOR).$(MINOR).$(BUILD)
endif

default: all

all: OpenJPEG
	make -C codec -f Makefile.nix all
	make -C mj2 -f Makefile.nix all
ifeq ($(WITH_JPWL),yes)
	make -C jpwl -f Makefile.nix all
endif
ifeq ($(WITH_JP3D),yes)
	make -C jp3d -f Makefile.nix all
endif

dos2unix:
	@$(DOS2UNIX) $(SRCS) $(INCLS)

OpenJPEG: $(STATICLIB) $(SHAREDLIB)
	install -d bin
	install -m 644 $(STATICLIB) bin
ifeq ($(ENABLE_SHARED),yes)
	install -m 755 $(SHAREDLIB) bin
	(cd bin && ln -sf $(SHAREDLIB) $(LIBNAME).so.$(MAJOR).$(MINOR))
	(cd bin && ln -sf $(LIBNAME).so.$(MAJOR).$(MINOR) $(LIBNAME).so)
endif

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(STATICLIB): $(MODULES)
	rm -f $(STATICLIB)
	$(AR) r $@ $(MODULES)

ifeq ($(ENABLE_SHARED),yes)
$(SHAREDLIB): $(MODULES)
	$(CC) -s -shared -Wl,-soname,$(LIBNAME) -o $@ $(MODULES) $(LIBRARIES)
endif

install: OpenJPEG
	install -d $(DESTDIR)$(INSTALL_LIBDIR) 
	install -m 644 -o root -g root $(STATICLIB) $(DESTDIR)$(INSTALL_LIBDIR)
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && ranlib $(STATICLIB) )
ifeq ($(ENABLE_SHARED),yes)
	install -m 755 -o root -g root $(SHAREDLIB) $(DESTDIR)$(INSTALL_LIBDIR)
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && \
	ln -sf $(SHAREDLIB) $(LIBNAME).so.$(MAJOR).$(MINOR) )
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && \
	ln -sf $(LIBNAME).so.$(MAJOR).$(MINOR) $(LIBNAME).so )
endif
	install -d $(DESTDIR)$(INSTALL_INCLUDE)
	install -m 644 -o root -g root libopenjpeg/openjpeg.h \
	$(DESTDIR)$(INSTALL_INCLUDE)
	(cd $(DESTDIR)$(prefix)/include && \
	ln -sf $(headerdir)/openjpeg.h openjpeg.h)
	make -C codec -f Makefile.nix install
	make -C mj2 -f Makefile.nix install
ifeq ($(WITH_JPWL),yes)
	make -C jpwl -f Makefile.nix install
endif
ifeq ($(WITH_JP3D),yes)
	make -C jp3d -f Makefile.nix install
endif
	ldconfig
	make -C doc -f Makefile.nix install

ifeq ($(WITH_JPWL),yes)
jpwl-all:
	make -C jpwl -f Makefile.nix all

jpwl-install: jpwl-all
	make -C jpwl -f Makefile.nix install
	ldconfig

jpwl-clean:
	make -C jpwl -f Makefile.nix clean

jpwl-uninstall:
	make -C jpwl -f Makefile.nix uninstall
endif

ifeq ($(WITH_JP3D),yes)
jp3d-all:
	make -C jp3d -f Makefile.nix all

jp3d-install: jp3d-all
	make -C jp3d -f Makefile.nix install
	ldconfig

jp3d-clean:
	make -C jp3d -f Makefile.nix clean

jp3d-uninstall:
	make -C jp3d -f Makefile.nix uninstall
endif

doc-all:
	make -C doc -f Makefile.nix all

doc-install: doc-all
	make -C doc -f Makefile.nix install

clean:
	rm -rf bin
	rm -f core u2dtmp* $(MODULES) $(STATICLIB) $(SHAREDLIB)
	make -C codec -f Makefile.nix clean
	make -C mj2 -f Makefile.nix clean
	make -C doc -f Makefile.nix clean
ifeq ($(WITH_JPWL),yes)
	make -C jpwl -f Makefile.nix clean
endif
ifeq ($(WITH_JP3D),yes)
	make -C jp3d -f Makefile.nix clean
endif

doc-clean:
	make -C doc -f Makefile.nix clean

uninstall:
	rm -f $(DESTDIR)$(INSTALL_LIBDIR)/$(STATICLIB)
ifeq ($(ENABLE_SHARED),yes)
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && \
	rm -f $(LIBNAME).so $(LIBNAME).so.$(MAJOR).$(MINOR) $(SHAREDLIB) )
endif
	ldconfig
	rm -f $(DESTDIR)$(prefix)/include/openjpeg.h
	rm -rf $(DESTDIR)$(INSTALL_INCLUDE)
	make -C codec -f Makefile.nix uninstall
	make -C mj2 -f Makefile.nix uninstall
	make -C doc -f Makefile.nix uninstall
ifeq ($(WITH_JPWL),yes)
	make -C jpwl -f Makefile.nix uninstall
endif
ifeq ($(WITH_JP3D),yes)
	make -C jp3d -f Makefile.nix uninstall
endif
