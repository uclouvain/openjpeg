J2KINCDIR = ../../../libopenjpeg
J2KLIBDIR = $(J2KINCDIR)/.libs
LIBDIR = ../libopenjpip
LIBFNAME = $(LIBDIR)/libopenjpip_local.a $(J2KLIBDIR)/libopenjpeg.a
CFLAGS  = -O3 -Wall -I$(LIBDIR) -I$(J2KINCDIR)
LDFLAGS = -L$(LIBDIR) -L$(J2KLIBDIR) -lm -lopenjpip_local

ALL = jpip_to_jp2 jpip_to_j2k test_index addXMLinJP2

all: $(ALL)

jpip_to_jp2: jpip_to_jp2.o $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) $(LIBFNAME) -o $@

jpip_to_j2k: jpip_to_j2k.o $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) $(LIBFNAME) -o $@

test_index: test_index.o $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) $(LIBFNAME) -o $@

clean:
	rm -f $(ALL) *.o *~
