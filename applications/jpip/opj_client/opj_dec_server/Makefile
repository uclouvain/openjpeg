LIBDIR = ../../libopenjpip
LIBFNAME = $(LIBDIR)/libopenjpip_local.a
CFLAGS  = -O3 -Wall -I$(LIBDIR)
LDFLAGS = -L$(LIBDIR) -lm -lopenjpeg -lopenjpip_local
#-lws2_32

ALL = opj_dec_server

all: $(ALL)

opj_dec_server: opj_dec_server.o jp2k_decoder.o imgsock_manager.o jptstream_manager.o cache_manager.o $(LIBFNAME)
	$(CC) $(CFLAGS) $< jp2k_decoder.o imgsock_manager.o jptstream_manager.o cache_manager.o $(LDFLAGS) -o $@
clean:
	rm -f $(ALL) *.o *~
