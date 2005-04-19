# 
# Valentin Mesaros (Octalis | DICE/UCL Belgium)
# 21 March 2005
# 
# Makefile for mj2 source files (frames_to_mj2, mj2_to_frames, extract_j2k_from_mj2, wrap_j2k_in_mj2)
# 

INCLUDES = -Imj2 \
           -I../libopenjpeg

C_OPT = -Wall -O -fno-strength-reduce -fomit-frame-pointer 
CFLAGS = $(INCLUDES) $(C_OPT)

CC = gcc

OBJ_MJ2_DIR = obj
BIN_MJ2_DIR = bin
LIB_OPENJPEG_DIR = ../libopenjpeg/lib
LIB_OPENJPEG = $(LIB_OPENJPEG_DIR)/libopenjpeg.a

all: $(OBJ_MJ2_DIR) $(BIN_MJ2_DIR) \
     $(BIN_MJ2_DIR)/frames_to_mj2 \
     $(BIN_MJ2_DIR)/mj2_to_frames \
     $(BIN_MJ2_DIR)/extract_j2k_from_mj2 \
     $(BIN_MJ2_DIR)/wrap_j2k_in_mj2


$(OBJ_MJ2_DIR):
	mkdir $(OBJ_MJ2_DIR)

$(BIN_MJ2_DIR):
	mkdir $(BIN_MJ2_DIR)

$(OBJ_MJ2_DIR)/%.o:
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJ_MJ2_DIR)/frames_to_mj2.o:		frames_to_mj2.c
$(OBJ_MJ2_DIR)/mj2_to_frames.o:		mj2_to_frames.c
$(OBJ_MJ2_DIR)/mj2.o:			mj2.c
$(OBJ_MJ2_DIR)/mj2_convert.o:		mj2_convert.c
$(OBJ_MJ2_DIR)/extract_j2k_from_mj2.o:	extract_j2k_from_mj2.c
$(OBJ_MJ2_DIR)/wrap_j2k_in_mj2.o:	wrap_j2k_in_mj2.c



MJ2_OBJS = $(addprefix $(OBJ_MJ2_DIR)/, mj2.o mj2_convert.o)


$(BIN_MJ2_DIR)/frames_to_mj2: $(OBJ_MJ2_DIR)/frames_to_mj2.o $(MJ2_OBJS) $(LIB_OPENJPEG) 
	$(CC) $(CFLAGS) \
              -o $(BIN_MJ2_DIR)/frames_to_mj2 \
              $(MJ2_OBJS) $(OBJ_MJ2_DIR)/frames_to_mj2.o \
              $(LIB_OPENJPEG) \
              -lm


$(BIN_MJ2_DIR)/mj2_to_frames: $(OBJ_MJ2_DIR)/mj2_to_frames.o $(MJ2_OBJS) $(LIB_OPENJPEG)
	$(CC) $(CFLAGS) \
              -o $(BIN_MJ2_DIR)/mj2_to_frames \
              $(MJ2_OBJS) $(OBJ_MJ2_DIR)/mj2_to_frames.o \
              $(LIB_OPENJPEG) \
              -lm


$(BIN_MJ2_DIR)/extract_j2k_from_mj2: $(OBJ_MJ2_DIR)/extract_j2k_from_mj2.o $(MJ2_OBJS) $(LIB_OPENJPEG)
	$(CC) $(CFLAGS) \
              -o $(BIN_MJ2_DIR)/extract_j2k_from_mj2 \
              $(MJ2_OBJS) $(OBJ_MJ2_DIR)/extract_j2k_from_mj2.o \
              $(LIB_OPENJPEG) \
              -lm
$(BIN_MJ2_DIR)/wrap_j2k_in_mj2: $(OBJ_MJ2_DIR)/wrap_j2k_in_mj2.o $(MJ2_OBJS) $(LIB_OPENJPEG)
	$(CC) $(CFLAGS) \
              -o $(BIN_MJ2_DIR)/wrap_j2k_in_mj2 \
              $(MJ2_OBJS) $(OBJ_MJ2_DIR)/wrap_j2k_in_mj2.o \
              $(LIB_OPENJPEG) \
              -lm

clean: 
	rm -f $(BIN_MJ2_DIR)/* $(OBJ_MJ2_DIR)/*