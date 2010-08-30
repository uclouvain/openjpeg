Basic instructions on how to build using CMake (CMake 2.4.5 or newer is required)

  svn co http://openjpeg.googlecode.com/svn/trunk openjpeg-read-only
  cd openjpeg-read-only
  cmake -G "Unix Makefiles"
  make
	
Executables are located in the ./bin directory.
	
For MacOSX users, if instructions above fail, try to add "-DCMAKE_OSX_ARCHITECTURES:STRING=i386" to the cmake command above.
	
To enable testing (and automatic result upload to http://my.cdash.org/index.php?project=OPENJPEG) : 

	svn co http://openjpeg.googlecode.com/svn/trunk openjpeg-read-only
	cd openjpeg-read-only
	cmake -G "Unix Makefiles" -DBUILD_TESTING:BOOL=ON -DJPEG2000_CONFORMANCE_DATA_ROOT:PATH=/path/to/your/JPEG2000/test/files
	make
	make Experimental

Note : JPEG2000 test files are available here : http://www.crc.ricoh.com/~gormish/jpeg2000conformance/