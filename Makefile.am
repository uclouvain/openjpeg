#OpenJPEG top source directory
SUBDIRS = @jpwl_dir@ libopenjpeg codec mj2 @jp3d_dir@ @doxy_dir@

clean-local:
	@rm -rf bin
	@l='$(SUBDIRS)'; for f in $$l ; do \
	rm -f $$f/.report.txt ; \
	done

distclean-local: clean-local

install-data-hook:
	@echo ""
	@l='$(SUBDIRS)'; for f in $$l ; do \
	cat $$f/.report.txt ; \
	done
	@echo ""
