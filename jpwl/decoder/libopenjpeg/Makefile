# $Id$
#
# makefile for OpenJPEG library

CFLAGS  = -Wall -O3 -fno-strength-reduce -fomit-frame-pointer

ifndef DEBUG
  LDFLAGS = -s -lm
else
  LDFLAGS = -lm
endif

OBJ_DIR_W32 = obj.w32
LIB_DIR_W32 = lib.w32

ifdef MINGW32
  CC = i386-mingw32-gcc
  AR = i386-mingw32-ar
  OBJ_DIR = $(OBJ_DIR_W32)
  LIB_DIR = $(LIB_DIR_W32)
  all:	$(OBJ_DIR) $(LIB_DIR) \
	$(LIB_DIR)/libopenjpeg.a $(LIB_DIR)/libopenjpeg.dll
else
  CC = gcc
  AR = ar
  OBJ_DIR = obj
  LIB_DIR = lib
  all:	$(OBJ_DIR) $(LIB_DIR) \
	$(LIB_DIR)/libopenjpeg.a $(LIB_DIR)/libopenjpeg.so
endif

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(LIB_DIR):	
	mkdir $(LIB_DIR)

$(OBJ_DIR)/%.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(OBJ_DIR)/bio.o:	bio.c bio.h
$(OBJ_DIR)/cio.o:	cio.c cio.h
$(OBJ_DIR)/dwt.o:	dwt.c dwt.h int.h fix.h tcd.h
$(OBJ_DIR)/fix.o:	fix.c fix.h
$(OBJ_DIR)/int.o:	int.c
$(OBJ_DIR)/j2k.o:	j2k.c j2k.h cio.h tcd.h dwt.h int.h
$(OBJ_DIR)/mct.o:	mct.c mct.h fix.h
$(OBJ_DIR)/mqc.o:	mqc.c mqc.h

$(OBJ_DIR)/pi.o:	pi.c pi.h int.h
$(OBJ_DIR)/raw.o:	raw.c raw.h
$(OBJ_DIR)/t1.o:	t1.c t1.h j2k.h mqc.h raw.h int.h mct.h dwt.h fix.h
$(OBJ_DIR)/t2.o:	t2.c t2.h tcd.h bio.h j2k.h pi.h tgt.h int.h cio.h
$(OBJ_DIR)/tcd.o:	tcd.c tcd.h int.h t1.h t2.h dwt.h mct.h
$(OBJ_DIR)/tgt.o:	tgt.c tgt.h bio.h
$(OBJ_DIR)/jpt.o:       jpt.c jpt.h cio.h
$(OBJ_DIR)/jp2.o:	jp2.c jp2.h

COM_OBJS = $(addprefix $(OBJ_DIR)/, j2k.o bio.o cio.o dwt.o fix.o int.o mct.o \
	   mqc.o pi.o t1.o t2.o  tgt.o tcd.o raw.o jpt.o jp2.o)

$(LIB_DIR)/libopenjpeg.a: ${COM_OBJS}
	$(AR) -sr $@ $^

$(LIB_DIR)/libopenjpeg.dll: ${COM_OBJS}
	${CC} -s -shared -Wl,-soname,libopenjpeg.dll -o $@ $^

$(LIB_DIR)/libopenjpeg.so.1.0: ${COM_OBJS}
	${CC} -s -shared -Wl,-soname,libopenjpeg.so.1 -o $@ $^

$(LIB_DIR)/libopenjpeg.so.1: $(LIB_DIR)/libopenjpeg.so.1.0
	ln -s libopenjpeg.so.1.0 $(LIB_DIR)/libopenjpeg.so.1

$(LIB_DIR)/libopenjpeg.so: $(LIB_DIR)/libopenjpeg.so.1
	ln -s libopenjpeg.so.1 $(LIB_DIR)/libopenjpeg.so

clean:
	rm -f $(OBJ_DIR_W32)/* $(OBJ_DIR)/* $(LIB_DIR_W32)/* $(LIB_DIR)/*
