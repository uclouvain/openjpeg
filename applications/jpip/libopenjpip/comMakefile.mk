ifdef jpipserver
CFLAGS  = -O3 -Wall -m32 -DSERVER
LIBNAME = libopenjpip_server.a
else
J2KINCDIR = ../../../libopenjpeg
CFLAGS  = -O3 -Wall  -I$(J2KINCDIR)
LIBNAME = libopenjpip_local.a
endif

all: $(LIBNAME)

ifdef jpipserver
$(LIBNAME): openjpip.o target_manager.o byte_manager.o box_manager.o boxheader_manager.o manfbox_manager.o \
	mhixbox_manager.o marker_manager.o codestream_manager.o faixbox_manager.o index_manager.o \
	msgqueue_manager.o metadata_manager.o placeholder_manager.o ihdrbox_manager.o imgreg_manager.o \
	cachemodel_manager.o j2kheader_manager.o jp2k_encoder.o query_parser.o channel_manager.o \
	session_manager.o jpip_parser.o sock_manager.o auxtrans_manager.o
	ar r $@ $^
else
$(LIBNAME): openjpip.o target_manager.o byte_manager.o box_manager.o boxheader_manager.o manfbox_manager.o \
	mhixbox_manager.o marker_manager.o codestream_manager.o faixbox_manager.o index_manager.o \
	msgqueue_manager.o metadata_manager.o placeholder_manager.o ihdrbox_manager.o imgreg_manager.o \
	cachemodel_manager.o j2kheader_manager.o jp2k_encoder.o  query_parser.o channel_manager.o \
	session_manager.o jpip_parser.o jp2k_decoder.o imgsock_manager.o jpipstream_manager.o cache_manager.o \
	dec_clientmsg_handler.o sock_manager.o
	ar r $@ $^
endif
clean:
	rm -f $(LIBNAME) *.o *~
