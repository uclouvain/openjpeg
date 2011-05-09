ifdef jpipserver
CFLAGS  = -O3 -Wall -m32 -DSERVER
LIBNAME = libopenjpip_server.a
else
CFLAGS  = -O3 -Wall
LIBNAME = libopenjpip_local.a
endif

all: $(LIBNAME)

$(LIBNAME): target_manager.o byte_manager.o box_manager.o boxheader_manager.o manfbox_manager.o \
	mhixbox_manager.o marker_manager.o codestream_manager.o faixbox_manager.o index_manager.o \
	msgqueue_manager.o metadata_manager.o placeholder_manager.o ihdrbox_manager.o imgreg_manager.o 
	ar r $@ $^

clean:
	rm -f $(LIBNAME) *.o *~
