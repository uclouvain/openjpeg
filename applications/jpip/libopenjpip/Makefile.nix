default: local server

local:
	make -f comMakefile.mk

server:
	rm *.o && make jpipserver=yes -f comMakefile.mk

clean:
	rm -f *.a *.o *~
