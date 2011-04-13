prefix=@CMAKE_INSTALL_PREFIX@
bindir=@OPENJPEG3D_INSTALL_BIN_DIR@
datadir=@OPENJPEG3D_INSTALL_DATA_DIR@
libdir=@OPENJPEG3D_INSTALL_LIB_DIR@
includedir=@OPENJPEG3D_INSTALL_INCLUDE_DIR@

Name: openjpeg3d
Description: JPEG2000 files library
URL: http://www.openjpeg.org/ 
Version: @OPENJPEG3D_VERSION@
Libs: -L${libdir} -lopenjpeg3d
Cflags: -I${includedir}
