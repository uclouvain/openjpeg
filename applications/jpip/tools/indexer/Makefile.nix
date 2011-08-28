INCDIR = ../../../../libopenjpeg
INCDIR2 = ext_libopenjpeg
LIBDIR = $(INCDIR)/.libs
LIBFNAME = $(LIBDIR)/libopenjpeg.a
CFLAGS  = -O3 -Wall -I$(INCDIR) -I$(INCDIR2)
LDFLAGS = -L$(LIBDIR) -lm 

ALL = j2k_to_idxjp2

all: $(ALL)

j2k_to_idxjp2: j2k_to_idxjp2.o event_mgr_handler.o idxjp2_manager.o j2k_decoder.o $(INCDIR2)/cidx_manager.o  $(INCDIR2)/cio_ext.o  $(INCDIR2)/ext_j2k.o  $(INCDIR2)/ext_jp2.o  $(INCDIR2)/phix_manager.o  $(INCDIR2)/ppix_manager.o  $(INCDIR2)/thix_manager.o  $(INCDIR2)/tpix_manager.o $(LIBFNAME)
	$(CC) $(CFLAGS) $< event_mgr_handler.o idxjp2_manager.o j2k_decoder.o $(INCDIR2)/cidx_manager.o  $(INCDIR2)/cio_ext.o  $(INCDIR2)/ext_j2k.o  $(INCDIR2)/ext_jp2.o  $(INCDIR2)/phix_manager.o  $(INCDIR2)/ppix_manager.o  $(INCDIR2)/thix_manager.o  $(INCDIR2)/tpix_manager.o $(LDFLAGS) $(LIBFNAME) -o $@
clean:
	rm -f $(ALL) *.o *~
