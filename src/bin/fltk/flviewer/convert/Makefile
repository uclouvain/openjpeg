LIBS=`fltk-config --ldflags`
INC=`fltk-config --cflags`
CC=g++

Convert:
	rm -f convert
	 $(CC) -Wall $(INC) convert.c -o convert $(LIBS)

clean:
	rm -f convert
