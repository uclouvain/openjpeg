prefix=@CMAKE_INSTALL_PREFIX@
bindir=${prefix}/@OPENJPEG_INSTALL_BIN_DIR@
datadir=${prefix}/@OPENJPEG_INSTALL_DATA_DIR@
libdir=${prefix}/@OPENJPEG_INSTALL_LIB_DIR@
includedir=${prefix}/@OPENJPEG_INSTALL_INCLUDE_DIR@

Name: openjpeg
Description: JPEG2000 files library
URL: http://www.openjpeg.org/ 
Version: @OPENJPEG_VERSION@
Libs: -L${libdir} -lopenjpeg
Cflags: -I${includedir}
