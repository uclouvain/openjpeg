MAJOR = @MAJOR_NR@
MINOR = @MINOR_NR@
BUILD = @BUILD_NR@

docdir = $(datarootdir)/doc/openjpeg-$(MAJOR).$(MINOR)

dist_doc_DATA = ../license.txt ../ChangeLog

docs:
	doxygen Doxyfile.dox
