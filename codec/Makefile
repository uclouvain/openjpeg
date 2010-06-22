# Makefile for the main OpenJPEG codecs: j2k_to_image and image_to_j2k

CFLAGS = -Wall -O3 -lstdc++ # -g -p -pg

all: j2k_to_image image_to_j2k j2k_dump

j2k_to_image: j2k_to_image.c ../libopenjpeg.a
	gcc $(CFLAGS) compat/getopt.c index.c convert.c j2k_to_image.c -o j2k_to_image -L.. -lopenjpeg -I ../libopenjpeg/ -lm -ltiff -lpng

image_to_j2k: image_to_j2k.c ../libopenjpeg.a
	gcc $(CFLAGS) compat/getopt.c index.c convert.c image_to_j2k.c -o image_to_j2k -L.. -lopenjpeg -I ../libopenjpeg/ -lm -ltiff -lpng

j2k_dump: j2k_dump.c ../libopenjpeg.a
	gcc $(CFLAGS) compat/getopt.c index.c j2k_dump.c -o j2k_dump -L.. -lopenjpeg -I ../libopenjpeg/ -lm

clean:
	rm -f j2k_to_image image_to_j2k j2k_dump
