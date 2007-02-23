# Makefile for the main JPWL OpenJPEG codecs: JPWL_ j2k_to_image and JPWL_image_to_j2k

# General configuration variables:
CC = gcc
AR = ar

CFLAGS = -O3 -DUSE_JPWL # -g -p -pg -DUSE_JPWL

OPJ_SRCS = ../libopenjpeg/bio.c ../libopenjpeg/cio.c ../libopenjpeg/dwt.c ../libopenjpeg/event.c ../libopenjpeg/image.c ../libopenjpeg/j2k.c ../libopenjpeg/j2k_lib.c ../libopenjpeg/jp2.c ../libopenjpeg/jpt.c ../libopenjpeg/mct.c ../libopenjpeg/mqc.c ../libopenjpeg/openjpeg.c ../libopenjpeg/pi.c ../libopenjpeg/raw.c ../libopenjpeg/t1.c ../libopenjpeg/t2.c ../libopenjpeg/tcd.c ../libopenjpeg/tgt.c
JPWL_SRCS = crc.c jpwl.c jpwl_lib.c rs.c

TARGET  = openjpeg_JPWL
STATICLIB = lib$(TARGET).a

MODULES = $(OPJ_SRCS:.c=.o) $(JPWL_SRCS:.c=.o)

all: JPWL_image_to_j2k JPWL_j2k_to_image

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(STATICLIB): $(MODULES)
	$(AR) r $@ $(MODULES)

JPWL_j2k_to_image: ../codec/j2k_to_image.c $(STATICLIB)
	gcc $(CFLAGS) ../codec/convert.c ../codec/j2k_to_image.c -o JPWL_j2k_to_image -I ../libopenjpeg/ -L . -lopenjpeg_JPWL -lm -lstdc++ 

JPWL_image_to_j2k: ../codec/image_to_j2k.c $(STATICLIB)
	gcc $(CFLAGS) ../codec/convert.c ../codec/image_to_j2k.c -o JPWL_image_to_j2k -I ../libopenjpeg/ -L . -lopenjpeg_JPWL -lm -lstdc++ 

clean:
	rm -f JPWL_j2k_to_image JPWL_image_to_j2k *.o *.a
	cd ../libopenjpeg; rm -f *.o
