#OpenJPEG top source directory
SUBDIRS = libopenjpeg codec mj2 @jpwl_dir@ @jp3d_dir@ doc

clean-local:
	@rm -rf bin
	@list='$(SUBDIRS)'; for dir in $${list} ; do \
	rm -f $${dir}/.report.txt ; \
	done

distclean-local: clean-local

pkgconf_dir = $(libdir)/pkgconfig

install-data-hook:
	@echo ""
	@list='$(SUBDIRS)'; for dir in $${list} ; do \
	cat $${dir}/.report.txt ; \
	done
	@echo ""
	@echo "Installing: libopenjpeg1.pc $(DESTDIR)$(pkgconf_dir)"
	@echo ""
	@$(INSTALL) -d $(DESTDIR)$(pkgconf_dir)
	@cp libopenjpeg1.pc $(DESTDIR)$(pkgconf_dir)/libopenjpeg1.pc
	@(cd $(DESTDIR)$(pkgconf_dir) && $(RM) libopenjpeg.pc && \
	$(LN_S) libopenjpeg1.pc libopenjpeg.pc)

uninstall-hook:
	@(cd $(DESTDIR)$(pkgconf_dir) && $(RM) libopenjpeg.pc libopenjpeg1.pc)
