#mj2 Makefile.nix
include ../config.nix

CFLAGS = -Wall

INSTALL_BIN = $(prefix)/bin
USERLIBS = -lm
INCLUDE = -I.. -I. -I../libopenjpeg -I../common

ifeq ($(WITH_LCMS2),yes)
INCLUDE += $(LCMS2_INCLUDE)
USERLIBS += $(LCMS2_LIB)
endif

ifeq ($(WITH_LCMS1),yes)
INCLUDE += $(LCMS1_INCLUDE)
USERLIBS += $(LCMS1_LIB)
endif

CFLAGS += $(INCLUDE) -lstdc++ # -g -p -pg

ifeq ($(ENABLE_SHARED),yes)
ELIB = ../libopenjpeg.so.$(MAJOR).$(MINOR).$(BUILD)
else
ELIB = ../libopenjpeg.a
endif

all: frames_to_mj2 mj2_to_frames extract_j2k_from_mj2 wrap_j2k_in_mj2
	install -d ../bin
	install frames_to_mj2 mj2_to_frames extract_j2k_from_mj2 \
	wrap_j2k_in_mj2 ../bin

frames_to_mj2: frames_to_mj2.c $(ELIB)
	$(CC) $(CFLAGS) ../common/getopt.c mj2_convert.c mj2.c frames_to_mj2.c \
	-o frames_to_mj2 $(ELIB) $(USERLIBS)

mj2_to_frames: mj2_to_frames.c $(ELIB)
	$(CC) $(CFLAGS) ../common/getopt.c mj2_convert.c mj2.c \
	../common/color.c mj2_to_frames.c \
	-o mj2_to_frames $(ELIB) $(USERLIBS)

extract_j2k_from_mj2: extract_j2k_from_mj2.c $(ELIB)
	$(CC) $(CFLAGS) mj2.c extract_j2k_from_mj2.c \
	-o extract_j2k_from_mj2 $(ELIB) $(USERLIBS)

wrap_j2k_in_mj2: wrap_j2k_in_mj2.c $(ELIB)
	$(CC) $(CFLAGS) mj2.c wrap_j2k_in_mj2.c \
	-o wrap_j2k_in_mj2 $(ELIB) $(USERLIBS)

clean:
	rm -f frames_to_mj2 mj2_to_frames extract_j2k_from_mj2 wrap_j2k_in_mj2

install: all
	install -d $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root frames_to_mj2 $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root mj2_to_frames $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root extract_j2k_from_mj2 $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root wrap_j2k_in_mj2 $(DESTDIR)$(INSTALL_BIN)

uninstall:
	rm -f $(DESTDIR)$(INSTALL_BIN)/frames_to_mj2
	rm -f $(DESTDIR)$(INSTALL_BIN)/mj2_to_frames
	rm -f $(DESTDIR)$(INSTALL_BIN)/extract_j2k_from_mj2
	rm -f $(DESTDIR)$(INSTALL_BIN)/wrap_j2k_in_mj2
