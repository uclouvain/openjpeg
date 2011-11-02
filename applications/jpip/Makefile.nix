default: t_libopenjpip t_util

t_libopenjpip:
	make -C libopenjpip -f Makefile.nix

t_util:
	make -C util -f Makefile.nix

clean:
	make clean -C libopenjpip -f Makefile.nix
	make clean -C util  -f Makefile.nix
