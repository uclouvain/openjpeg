LIBDIR = ../libopenjpip
LIBFNAME = $(LIBDIR)/libopenjpip_local.a
CFLAGS  = -O3 -Wall -I$(LIBDIR)
LDFLAGS = -L$(LIBDIR) -lm -lopenjpip_local

ALL = jpip_to_jp2 jpip_to_j2k test_index addXMLinJP2

all: $(ALL)

jpip_to_jp2: jpip_to_jp2.o $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

jpip_to_j2k: jpip_to_j2k.o $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

test_index: test_index.o $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f $(ALL) *.o *~
	make clean -C indexer -f Makefile.nix
