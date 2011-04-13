#jpwl Makefile
include ../config.nix

TARGET  = openjpeg_JPWL
COMPILERFLAGS = -Wall -ffast-math -std=c99 -fPIC
USERLIBS =

JPWL_SRCS = ./crc.c ./jpwl.c ./jpwl_lib.c ./rs.c

SRCS = ../libopenjpeg/bio.c ../libopenjpeg/cio.c ../libopenjpeg/dwt.c \
  ../libopenjpeg/event.c ../libopenjpeg/image.c ../libopenjpeg/j2k.c \
  ../libopenjpeg/j2k_lib.c ../libopenjpeg/jp2.c ../libopenjpeg/jpt.c \
  ../libopenjpeg/mct.c ../libopenjpeg/mqc.c ../libopenjpeg/openjpeg.c \
  ../libopenjpeg/pi.c ../libopenjpeg/raw.c ../libopenjpeg/t1.c \
  ../libopenjpeg/t2.c ../libopenjpeg/tcd.c ../libopenjpeg/tgt.c \
  $(JPWL_SRCS)

INCLS = ../libopenjpeg/bio.h ../libopenjpeg/cio.h ../libopenjpeg/dwt.h \
  ../libopenjpeg/event.h ../libopenjpeg/fix.h ../libopenjpeg/image.h \
  ../libopenjpeg/int.h ../libopenjpeg/j2k.h ../libopenjpeg/j2k_lib.h \
  ../libopenjpeg/jp2.h ../libopenjpeg/jpt.h ../libopenjpeg/mct.h \
  ../libopenjpeg/mqc.h ../libopenjpeg/openjpeg.h ../libopenjpeg/pi.h \
  ../libopenjpeg/raw.h ../libopenjpeg/t1.h ../libopenjpeg/t2.h \
  ../libopenjpeg/tcd.h ../libopenjpeg/tgt.h ../libopenjpeg/opj_malloc.h \
  ../libopenjpeg/opj_includes.h

INCLUDE = -I.. -I. -I../libopenjpeg -I../common

INSTALL_LIBDIR = $(prefix)/lib
INSTALL_BIN = $(prefix)/bin

# Converts cr/lf to just lf
DOS2UNIX = dos2unix


LIBRARIES = -lstdc++

ifeq ($(WITH_TIFF),yes)
INCLUDE += $(TIFF_INCLUDE)
USERLIBS += $(TIFF_LIB)
endif

ifeq ($(WITH_PNG),yes)
INCLUDE += $(PNG_INCLUDE)
USERLIBS += $(PNG_LIB)
endif

ifeq ($(WITH_LCMS2),yes)
INCLUDE += $(LCMS2_INCLUDE)
USERLIBS += $(LCMS2_LIB)
endif

ifeq ($(WITH_LCMS1),yes)
INCLUDE += $(LCMS1_INCLUDE)
USERLIBS += $(LCMS1_LIB)
endif

USERLIBS += -lm

MODULES = $(SRCS:.c=.o)

CFLAGS = $(COMPILERFLAGS) $(INCLUDE) -DUSE_JPWL

LIBNAME = lib$(TARGET)

ifeq ($(ENABLE_SHARED),yes)
SHAREDLIB = $(LIBNAME).so.$(MAJOR).$(MINOR).$(BUILD)
else
STATICLIB = $(LIBNAME).a
endif

default: all

all: OpenJPEG_JPWL JPWL_image_to_j2k JPWL_j2k_to_image
	install -d ../bin
ifeq ($(ENABLE_SHARED),yes)
	install -m 755 $(SHAREDLIB) ../bin
	(cd ../bin && ln -sf $(SHAREDLIB) $(LIBNAME).so.$(MAJOR).$(MINOR))
	(cd ../bin && ln -sf $(LIBNAME).so.$(MAJOR).$(MINOR) $(LIBNAME).so)
else
	install -m 644 $(STATICLIB) ../bin
endif
	install JPWL_image_to_j2k JPWL_j2k_to_image ../bin

dos2unix:
	@$(DOS2UNIX) $(SRCS) $(INCLS)

OpenJPEG_JPWL: $(STATICLIB) $(SHAREDLIB)

JPWL_codec: JPWL_j2k_to_image JPWL_image_to_j2k $(STATICLIB)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

ifeq ($(ENABLE_SHARED),yes)
$(SHAREDLIB): $(MODULES)
	$(CC) -shared -Wl,-soname,$(LIBNAME) -o $@ $(MODULES) $(LIBRARIES)
else
$(STATICLIB): $(MODULES)
	$(AR) r $@ $(MODULES)
endif

ifeq ($(ENABLE_SHARED),yes)
ELIB = $(SHAREDLIB)
else
ELIB = $(STATICLIB)
endif

JPWL_j2k_to_image: ../codec/j2k_to_image.c
	$(CC) $(CFLAGS) ../common/getopt.c ../codec/index.c \
	../codec/convert.c ../common/color.c ../codec/j2k_to_image.c \
	-o JPWL_j2k_to_image $(ELIB) $(USERLIBS)

JPWL_image_to_j2k: ../codec/image_to_j2k.c
	$(CC) $(CFLAGS) ../common/getopt.c ../codec/index.c \
	../codec/convert.c ../codec/image_to_j2k.c \
	-o JPWL_image_to_j2k $(ELIB) $(USERLIBS)

install: OpenJPEG_JPWL
	install -d $(DESTDIR)$(INSTALL_LIBDIR)
ifeq ($(ENABLE_SHARED),yes)
	install -m 755 -o root -g root $(SHAREDLIB) $(DESTDIR)$(INSTALL_LIBDIR)
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && \
	ln -sf $(SHAREDLIB) $(LIBNAME).so.$(MAJOR).$(MINOR) )
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && \
	ln -sf $(LIBNAME).so.$(MAJOR).$(MINOR) $(LIBNAME).so )
else
	install -m 644 -o root -g root $(STATICLIB) $(DESTDIR)$(INSTALL_LIBDIR)
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && ranlib $(STATICLIB))
endif
	install -d $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root JPWL_j2k_to_image $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root JPWL_image_to_j2k $(DESTDIR)$(INSTALL_BIN)
	
cleanlib:
	rm -f core u2dtmp* $(MODULES) $(STATICLIB) $(SHAREDLIB)

cleancodec:
	rm -f JPWL_j2k_to_image JPWL_image_to_j2k JPWL_j2k_to_image.o \
	JPWL_image_to_j2k.o

clean: cleanlib cleancodec

uninstall:
ifeq ($(ENABLE_SHARED),yes)
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && \
	rm -f $(LIBNAME).so $(LIBNAME).so.$(MAJOR).$(MINOR) $(SHAREDLIB))
else
	rm -f $(DESTDIR)$(INSTALL_LIBDIR)/$(STATICLIB)
endif
	rm -f $(DESTDIR)$(INSTALL_BIN)/JPWL_j2k_to_image
	rm -f $(DESTDIR)$(INSTALL_BIN)/JPWL_image_to_j2k
