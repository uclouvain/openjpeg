# Makefile for OPJViewer

# General configuration variables:
CC = $(shell wx-config-2.8 --cxx)
AR = ar

CFLAGS = -DUSE_JPWL -DwxUSE_LIBOPENJPEG -DwxUSE_GUI=1 -DOPJ_STATIC -DOPJ_HTMLABOUT -DOPJ_INICONFIG -DUSE_JPSEC -DOPJ_MANYFORMATS $(shell wx-config-2.8 --cxxflags) # -g -p -pg -DUSE_JPWL

OPJV_SRCS = source/imagjpeg2000.cpp source/wxj2kparser.cpp source/OPJViewer.cpp source/wxjp2parser.cpp source/OPJDialogs.cpp source/OPJThreads.cpp source/OPJAbout.cpp ../codec/index.c

MODULES = $(OPJV_SRCS:.cpp=.o)

all: opjviewer lib

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

lib:
	cd ../jpwl; make

opjviewer: $(OPJV_SRCS) lib
	$(CC) $(CFLAGS) -I .. -I ../codec -I ../libopenjpeg $(OPJV_SRCS) -o OPJViewer -L ../jpwl -lopenjpeg_JPWL -lm -lstdc++ -ltiff $(shell wx-config-2.8 --libs)


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
