
MAINTAINERCLEANFILES = Makefile.in

.PHONY: doc

PACKAGE_DOCNAME = $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc

if OPJ_BUILD_DOC

doc-clean:
	rm -rf $(top_srcdir)/doc/html/ $(top_srcdir)/doc/$(PACKAGE_DOCNAME).tar*

doc: all
	cd $(top_srcdir)/doc && \
	$(opj_doxygen) Doxyfile.dox
	rm -rf $(PACKAGE_DOCNAME).tar*
	rm -rf $(top_srcdir)/doc/$(PACKAGE_DOCNAME).tar*
	mkdir -p $(PACKAGE_DOCNAME)/doc
	cp -R $(top_srcdir)/doc/html/ $(PACKAGE_DOCNAME)/doc
	tar cf $(PACKAGE_DOCNAME).tar $(PACKAGE_DOCNAME)/
	bzip2 -9 $(PACKAGE_DOCNAME).tar
	rm -rf $(PACKAGE_DOCNAME)/
	if ! test -f $(top_srcdir)/doc/$(PACKAGE_DOCNAME).tar.bz2 ; then \
	   mv $(PACKAGE_DOCNAME).tar.bz2 $(top_srcdir)/doc; \
	fi

clean-local: doc-clean

else

doc:
	@echo "Documentation not built. Run ./configure --help"

endif

docsdir = $(docdir)-$(MAJOR_NR).$(MINOR_NR)
docs_DATA = $(top_builddir)/LICENSE $(top_builddir)/CHANGES

dist_man1_MANS = \
man/man1/image_to_j2k.1 \
man/man1/j2k_dump.1 \
man/man1/j2k_to_image.1

dist_man3_MANS = \
man/man3/libopenjpeg.3

install-data-hook:
	cd $(DESTDIR)$(man1dir) && \
	  rm -f jpwl_image_to_j2k.1 && \
	  $(LN_S) image_to_j2k.1 jpwl_image_to_j2k.1
	cd $(DESTDIR)$(man1dir) && \
	  rm -f jpwl_j2k_to_image.1 && \
	  $(LN_S) j2k_to_image.1 jpwl_j2k_to_image.1
	cd $(DESTDIR)$(man3dir) && \
	  rm -f jpwl_libopenjpeg.3 && \
	  $(LN_S) libopenjpeg.3 jpwl_libopenjpeg.3
	@echo -e " (MAN)\t$(man1dir)/image_to_j2k.1" >> $(top_builddir)/report.txt
	@echo -e " (LN)\t$(man1dir)/jpwl_image_to_j2k.1" >> $(top_builddir)/report.txt
	@echo -e " (MAN)\t$(man1dir)/j2k_dump.1" >> $(top_builddir)/report.txt
	@echo -e " (MAN)\t$(man1dir)/j2k_to_image.1" >> $(top_builddir)/report.txt
	@echo -e " (LN)\t$(man1dir)//jpwl_j2k_to_image.1" >> $(top_builddir)/report.txt
	@echo -e " (MAN)\t$(man3dir)/libopenjpeg.3" >> $(top_builddir)/report.txt
	@echo -e " (LN)\t$(man3dir)/jpwl_libopenjpeg.3" >> $(top_builddir)/report.txt

uninstall-hook:
	rm -f $(DESTDIR)$(man1dir)/jpwl_image_to_j2k.1
	rm -f $(DESTDIR)$(man1dir)/jpwl_j2k_to_image.1
	rm -f $(DESTDIR)$(man3dir)/jpwl_libopenjpeg.3

EXTRA_DIST = \
CMakeLists.txt \
Doxyfile.dox
