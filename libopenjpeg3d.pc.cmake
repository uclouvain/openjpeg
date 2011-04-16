prefix=@CMAKE_INSTALL_PREFIX@
bindir=@OPENJP3D_INSTALL_BIN_DIR@
datadir=@OPENJP3D_INSTALL_DATA_DIR@
libdir=@OPENJP3D_INSTALL_LIB_DIR@
includedir=@OPENJP3D_INSTALL_INCLUDE_DIR@

Name: openjp3d
Description: JPEG2000 files library
URL: http://www.openjpeg.org/ 
Version: @OPENJP3D_VERSION@
Libs: -L${libdir} -lopenjp3d
Cflags: -I${includedir}
