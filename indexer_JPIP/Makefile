CC = gcc

LDFLAGS = -lm
CFLAGS = -Wall

all: index_create


bio.o : bio.c bio.h
cio.o : cio.c cio.h
int.o : int.c
pi.o  : pi.c pi.h int.h
index_create.o : index_create.c j2k.h cio.h tcd.h int.h
t2.o  : t2.c t2.h tcd.h bio.h j2k.h pi.h tgt.h int.h cio.h
tgt.o : tgt.c bio.h tgt.h
tcd.o : tcd.c tcd.h t2.h int.h
jpip.o : jpip.c j2k.h cio.h tcd.h int.h
jp2.o : jp2.c j2k.h cio.h tcd.h int.h

index_create : bio.o cio.o int.o pi.o t2.o tgt.o tcd.o index_create.o jpip.o jp2.o

clean:
	rm -rf *.o *.*~ *~ core.*
