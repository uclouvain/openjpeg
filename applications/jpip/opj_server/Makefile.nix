LIBDIR = ../libopenjpip

LIBFNAME = $(LIBDIR)/libopenjpip_server.a
CFLAGS  = -O3 -Wall -m32 -DSERVER -DQUIT_SIGNAL=\"quitJPIP\" -I$(LIBDIR)
LDFLAGS = -L$(LIBDIR) -lm -lfcgi -lopenjpip_server

ALL = opj_server

all: $(ALL)

opj_server: opj_server.o query_parser.o channel_manager.o session_manager.o jpip_parser.o $(LIBFNAME)
	  $(CC) $(CFLAGS) $< query_parser.o channel_manager.o session_manager.o jpip_parser.o $(LDFLAGS) -o $@

clean:
	rm -f $(ALL) *.o *~
