# Makefile for the main OpenJPEG codecs: jp3d_to_volume and volume_to_jp3d

CFLAGS = -O3 -lstdc++ # -g -p -pg

all: jp3d_to_volume volume_to_jp3d

jp3d_to_volume: jp3d_to_volume.c ../libjp3dvm.a
	gcc $(CFLAGS) getopt.c convert.c jp3d_to_volume.c -o jp3d_to_volume -L.. -ljp3dvm -I ../libjp3dvm/ -lm -ltiff

volume_to_jp3d: volume_to_jp3d.c ../libjp3dvm.a
	gcc $(CFLAGS) getopt.c convert.c volume_to_jp3d.c -o volume_to_jp3d -L.. -ljp3dvm -I ../libjp3dvm/ -lm -ltiff

clean:
	rm -f jp3d_to_volume volume_to_jp3d
