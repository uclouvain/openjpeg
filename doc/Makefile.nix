#doc/Makefile
include ../config.nix

ifeq ($(HAS_DOXYGEN),yes)
docdir = $(prefix)/share/doc/openjpeg-$(MAJOR).$(MINOR)

all:
	doxygen Doxyfile.dox

install: all
	install -d $(docdir)
	cp -rf html $(docdir)
	cp -f ../license.txt ../ChangeLog $(docdir)

clean:
	rm -rf html

endif
