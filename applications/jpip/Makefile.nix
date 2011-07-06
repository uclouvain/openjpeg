default: t_libopenjpip t_opj_server t_opj_dec_server t_tools

t_libopenjpip:
	make -C libopenjpip -f Makefile.nix

t_opj_server:
	make -C opj_server -f Makefile.nix

t_opj_dec_server:
	make -C opj_client/opj_dec_server  -f Makefile.nix

t_tools:
	make -C tools  -f Makefile.nix

clean:
	make clean -C libopenjpip -f Makefile.nix
	make clean -C opj_server  -f Makefile.nix
	make clean -C opj_client/opj_dec_server -f Makefile.nix
	make clean -C tools -f Makefile.nix
