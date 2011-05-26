default: t_libopenjpip t_opj_server t_opj_dec_server t_tools

t_libopenjpip:
	make -C libopenjpip

t_opj_server:
	make -C opj_server

t_opj_dec_server:
	make -C opj_client/opj_dec_server

t_tools:
	make -C tools

clean:
	make clean -C libopenjpip
	make clean -C opj_server
	make clean -C opj_client/opj_dec_server
	make clean -C tools
