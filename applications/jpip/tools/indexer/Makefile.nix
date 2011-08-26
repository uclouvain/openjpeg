INCDIR = ../../../../libopenjpeg
LIBDIR = $(INCDIR)/.libs
LIBFNAME = $(LIBDIR)/libopenjpeg.a
CFLAGS  = -O3 -Wall -I$(INCDIR)
LDFLAGS = -L$(LIBDIR) -lm 

ALL = j2k_to_idxjp2

all: $(ALL)

j2k_to_idxjp2: j2k_to_idxjp2.o event_mgr_handler.o idxjp2_manager.o j2k_decoder.o cidx_manager.o cio_ext.o tpix_manager.o thix_manager.o ppix_manager.o phix_manager.o $(LIBFNAME)
	$(CC) $(CFLAGS) $< event_mgr_handler.o idxjp2_manager.o j2k_decoder.o cidx_manager.o cio_ext.o tpix_manager.o thix_manager.o ppix_manager.o phix_manager.o $(LDFLAGS) $(LIBFNAME) -o $@
clean:
	rm -f $(ALL) *.o *~
