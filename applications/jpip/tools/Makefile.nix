LIBDIR = ../libopenjpip
LIBFNAME = $(LIBDIR)/libopenjpip_local.a
CFLAGS  = -O3 -Wall -I$(LIBDIR)
LDFLAGS = -L$(LIBDIR) -lm -lopenjpip_local

ALL = jpt_to_jp2 jpt_to_j2k test_index addXMLinJP2

all: t_indexer $(ALL)
  
t_indexer:
	make -C indexer

jpt_to_jp2: jpt_to_jp2.o $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

jpt_to_j2k: jpt_to_j2k.o $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

test_index: test_index.o $(LIBFNAME)
	    $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f $(ALL) *.o *~
	make clean -C indexer
