#codec Makefile
include ../config.nix

CFLAGS = -Wall

INSTALL_BIN = $(prefix)/bin

INCLUDE = -I.. -I. -I../libopenjpeg -I../common
USERLIBS = -lm

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

CFLAGS += $(INCLUDE) -lstdc++ # -g -p -pg

all: j2k_to_image image_to_j2k j2k_dump
	install -d ../bin
	install j2k_to_image image_to_j2k j2k_dump ../bin

j2k_to_image: j2k_to_image.c ../libopenjpeg.a
	$(CC)  $(CFLAGS) ../common/getopt.c index.c convert.c \
	../common/color.c j2k_to_image.c \
	-o j2k_to_image ../libopenjpeg.a $(USERLIBS)

image_to_j2k: image_to_j2k.c ../libopenjpeg.a
	$(CC) $(CFLAGS) ../common/getopt.c index.c convert.c image_to_j2k.c \
	-o image_to_j2k ../libopenjpeg.a $(USERLIBS)

j2k_dump: j2k_dump.c ../libopenjpeg.a
	$(CC) $(CFLAGS) ../common/getopt.c index.c j2k_dump.c \
	-o j2k_dump ../libopenjpeg.a $(USERLIBS)

clean:
	rm -f j2k_to_image image_to_j2k j2k_dump

install: all
	install -d $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root j2k_to_image $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root image_to_j2k $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root j2k_dump $(DESTDIR)$(INSTALL_BIN)

uninstall:
	rm -f $(DESTDIR)$(INSTALL_BIN)/j2k_to_image
	rm -f $(DESTDIR)$(INSTALL_BIN)/image_to_j2k
	rm -f $(DESTDIR)$(INSTALL_BIN)/j2k_dump
