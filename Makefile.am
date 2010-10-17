#OpenJPEG top source directory
SUBDIRS = libopenjpeg codec mj2 @jpwl_dir@ @jp3d_dir@ doc

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
