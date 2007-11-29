# Makefile for OPJViewer

# General configuration variables:
CC = $(shell wx-config-2.8 --cxx)
AR = ar

CFLAGS = -DUSE_JPWL -DwxUSE_LIBOPENJPEG -DwxUSE_GUI=1 -DOPJ_STATIC -DOPJ_HTMLABOUT $(shell wx-config-2.8 --cxxflags) # -g -p -pg -DUSE_JPWL

OPJV_SRCS = source/imagj2k.cpp source/imagmj2.cpp source/wxj2kparser.cpp source/imagjp2.cpp source/OPJViewer.cpp source/wxjp2parser.cpp source/OPJViewer.cpp source/OPJThreads.cpp

MODULES = $(OPJV_SRCS:.cpp=.o)

all: opjviewer lib

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

lib:
	cd ../jpwl; make

opjviewer: $(OPJV_SRCS) lib
        $(CC) $(CFLAGS) -I .. $(OPJV_SRCS) -o OPJViewer -L ../jpwl -lopenjpeg_JPWL -lm -lstdc++ -ltiff $(shell wx-config-2.8 --libs)


clean:
	rm -f OPJViewer *.o *.a
	cd ../libopenjpeg; rm -f *.o




#.cpp.o :
#       $(CXX) -g -c `wx-config-2.8 --cxxflags` -I ../.. -D wxUSE_LIBOPENJPEG -D wxHACK_BOOLEAN -o $@ $<

#all:    $(PROGRAM)

#$(PROGRAM):     $(OBJECTS)
#       $(CXX) -o $(PROGRAM) $(OBJECTS) -lopenjpeg -L ../.. `wx-config-2.8 --libs`

#clean:
#       rm -f *.o $(PROGRAM)
