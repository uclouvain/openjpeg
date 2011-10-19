J2KINCDIR = ../../../../libopenjpeg
J2KLIBDIR = $(J2KINCDIR)/.libs
JPIPLIBDIR = ../../libopenjpip
LIBFNAME = $(JPIPLIBDIR)/libopenjpip_local.a $(J2KLIBDIR)/libopenjpeg.a
CFLAGS  = -O3 -Wall -I$(JPIPLIBDIR)
LDFLAGS = -L$(JPIPLIBDIR) -L$(J2KLIBDIR) -lm -lopenjpip_local
#-lws2_32

ALL = opj_dec_server

all: $(ALL)

opj_dec_server: opj_dec_server.o $(LIBFNAME)
	$(CC) $(CFLAGS) $< $(LDFLAGS) $(LIBFNAME) -o $@

clean:
	rm -f $(ALL) *.o *~
