# $Id$
#
# makefile for OpenJPEG codec

OPENJPEG_DIR = ../libopenjpeg

ifndef DEBUG
  LDFLAGS = -s -lm
else
  LDFLAGS = -lm
endif

CFLAGS = -Wall -O3 -fno-strength-reduce -fomit-frame-pointer -I$(OPENJPEG_DIR)

OBJ_DIR_W32 = obj.w32
BIN_DIR_W32 = bin.w32
LIB_DIR_W32 = $(OPENJPEG_DIR)/lib.w32

ifdef MINGW32
  CC = i386-mingw32-gcc
  CFLAGS += -DDONT_HAVE_GETOPT
  OBJ_DIR = $(OBJ_DIR_W32)
  BIN_DIR = $(BIN_DIR_W32)
  LIB_DIR = lib.w32
  all: $(addprefix $(BIN_DIR)/,j2k_to_image.exe image_to_j2k.exe)
else
  CC = gcc
  OBJ_DIR = obj
  BIN_DIR = bin
  LIB_DIR = lib
  all: $(addprefix $(BIN_DIR)/,j2k_to_image image_to_j2k)
endif

ifdef DYNAMIC
  ifdef MINGW32
    LIB_OPENJPEG = $(LIB_DIR)/libopenjpeg.dll
    LDFLAGS += -L$(LIB_DIR) -lopenjpeg
  else
    LIB_OPENJPEG = $(OPENJPEG_DIR)/$(LIB_DIR)/libopenjpeg.so
    LDFLAGS += -L$(OPENJPEG_DIR)/$(LIB_DIR) -lopenjpeg
  endif
else
  LIB_OPENJPEG = $(OPENJPEG_DIR)/$(LIB_DIR)/libopenjpeg.a
  LDFLAGS += $(LIB_OPENJPEG)
endif

$(LIB_DIR)/libopenjpeg.dll: $(OPENJPEG_DIR)/$(LIB_DIR)/libopenjpeg.dll
	ln -sf ../$< $@
$(OPENJPEG_DIR)/$(LIB_DIR)/libopenjpeg.dll:
	cd $(OPENJPEG_DIR) && $(MAKE) $(LIB_DIR)/libopenjpeg.dll
$(OPENJPEG_DIR)/$(LIB_DIR)/libopenjpeg.a:
	cd $(OPENJPEG_DIR) && $(MAKE) $(LIB_DIR)/libopenjpeg.a
$(OPENJPEG_DIR)/$(LIB_DIR)/libopenjpeg.so:
	cd $(OPENJPEG_DIR) && $(MAKE) $(LIB_DIR)/libopenjpeg.so

$(OBJ_DIR)/%.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(OBJ_DIR)/getopt.o:		compat/getopt.c
$(OBJ_DIR)/convert.o:		convert.c
$(OBJ_DIR)/j2k_to_image.o:	j2k_to_image.c
$(OBJ_DIR)/image_to_j2k.o:	image_to_j2k.c convert.h

J2I_OBJS = $(addprefix $(OBJ_DIR)/,j2k_to_image.o)
I2J_OBJS = $(addprefix $(OBJ_DIR)/,image_to_j2k.o convert.o)

$(BIN_DIR)/j2k_to_image:	$(J2I_OBJS) $(LIB_OPENJPEG)
	${CC} -o $@  $(J2I_OBJS) ${LDFLAGS} 

$(BIN_DIR)/image_to_j2k:	$(I2J_OBJS) $(LIB_OPENJPEG)
	${CC} -o $@  $(I2J_OBJS) ${LDFLAGS} 

$(BIN_DIR)/j2k_to_image.exe:	$(J2I_OBJS) $(LIB_OPENJPEG)
	${CC} -o $@  $(J2I_OBJS) ${LDFLAGS} 

$(BIN_DIR)/image_to_j2k.exe:	$(I2J_OBJS) $(LIB_OPENJPEG) $(OBJ_DIR)/getopt.o
	${CC} -o $@  $(I2J_OBJS) ${LDFLAGS} $(OBJ_DIR)/getopt.o

clean:
	rm -f $(BIN_DIR_W32)/* $(BIN_DIR)/* $(OBJ_DIR_W32)/* $(OBJ_DIR)/*
