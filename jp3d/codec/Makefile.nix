#jp3d/codec/Makefile

include ../../config.nix

INSTALL_BIN = $(prefix)/bin
CFLAGS = -Wall -I. -I../libjp3dvm -lstdc++ # -g -p -pg
#USERLIBS = -lm

all: jp3d_to_volume volume_to_jp3d
	install -d ../../bin
	install jp3d_to_volume volume_to_jp3d ../../bin

jp3d_to_volume: jp3d_to_volume.c ../libopenjp3dvm.a
	$(CC) $(CFLAGS) getopt.c convert.c jp3d_to_volume.c \
	-o jp3d_to_volume ../libopenjp3dvm.a $(USERLIBS)

volume_to_jp3d: volume_to_jp3d.c ../libopenjp3dvm.a
	$(CC) $(CFLAGS) getopt.c convert.c volume_to_jp3d.c \
	-o volume_to_jp3d ../libopenjp3dvm.a $(USERLIBS)

clean:
	rm -f jp3d_to_volume volume_to_jp3d

install: all
	install -d $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root jp3d_to_volume $(DESTDIR)$(INSTALL_BIN)
	install -m 755 -o root -g root volume_to_jp3d $(DESTDIR)$(INSTALL_BIN)

uninstall:
	rm -f $(DESTDIR)$(INSTALL_BIN)/jp3d_to_volume
	rm -f $(DESTDIR)$(INSTALL_BIN)/volume_to_jp3d
