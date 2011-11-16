JPIPLIBDIR = ../libopenjpip

SLIBFNAME = $(JPIPLIBDIR)/libopenjpip_server.a
SCFLAGS  = -O3 -Wall -m32 -I$(JPIPLIBDIR) -DSERVER -DQUIT_SIGNAL=\"quitJPIP\"
SLDFLAGS = -L$(JPIPLIBDIR) -lm -lfcgi -lcurl -lopenjpip_server

J2KINCDIR = ../../../libopenjpeg
J2KLIBDIR = $(J2KINCDIR)/.libs
LIBFNAME = $(JPIPLIBDIR)/libopenjpip_local.a $(J2KLIBDIR)/libopenjpeg.a
CFLAGS  = -O3 -Wall -I$(JPIPLIBDIR)
LDFLAGS = -L$(JPIPLIBDIR) -L$(J2KLIBDIR) -lm -lcurl -lopenjpip_local

ALL = opj_server opj_dec_server jpip_to_jp2 jpip_to_j2k test_index addXMLinJP2

all: $(ALL)

opj_server: opj_server.c $(SLIBFNAME)
	  $(CC) $(SCFLAGS) $< $(SLDFLAGS) $(SLIBFNAME) -o $@

opj_dec_server: opj_dec_server.c $(LIBFNAME)
	$(CC) $(CFLAGS) $< $(LDFLAGS) $(LIBFNAME) -o $@

jpip_to_jp2: jpip_to_jp2.c $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) $(LIBFNAME) -o $@

jpip_to_j2k: jpip_to_j2k.c $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) $(LIBFNAME) -o $@

test_index: test_index.c $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) $(LIBFNAME) -o $@

clean:
	rm -f $(ALL) *.o *~
