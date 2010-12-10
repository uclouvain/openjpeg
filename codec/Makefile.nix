#jp3d/codec/Makefile

include ../../config.nix

INSTALL_BIN = $(prefix)/bin
CFLAGS = -Wall -I. -I../libjp3dvm -lstdc++ # -g -p -pg
#USERLIBS = -lm

ifeq ($(ENABLE_SHARED),yes)
ELIB = ../libopenjp3dvm.so.$(JP3D_MAJOR).$(JP3D_MINOR).$(JP3D_BUILD)
else
ELIB = ../libopenjp3dvm.a
endif

all: jp3d_to_volume volume_to_jp3d
	install -d ../../bin
	install jp3d_to_volume volume_to_jp3d ../../bin

jp3d_to_volume: jp3d_to_volume.c $(ELIB)
	$(CC) $(CFLAGS) getopt.c convert.c jp3d_to_volume.c \
	-o jp3d_to_volume  $(ELIB) $(USERLIBS)

volume_to_jp3d: volume_to_jp3d.c  $(ELIB)
	$(CC) $(CFLAGS) getopt.c convert.c volume_to_jp3d.c \
	-o volume_to_jp3d  $(ELIB) $(USERLIBS)

clean:
	rm -f jp3d_to_volume volume_to_jp3d

install: all
	install -d $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root jp3d_to_volume $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root volume_to_jp3d $(DESTDIR)$(INSTALL_BIN)

uninstall:
	rm -f $(DESTDIR)$(INSTALL_BIN)/jp3d_to_volume
	rm -f $(DESTDIR)$(INSTALL_BIN)/volume_to_jp3d
