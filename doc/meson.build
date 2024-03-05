  # Generate target to build the html documentation through CMake tool
  # After having configured the project with the "build_doc" option you can run make doc
  # to generate the html documentation in the doc/html repository of the build folder.
  # Try to find the doxygen tool
  doxygen_dep = dependency('Doxygen')
  if doxygen_dep.found()
    # Configure the doxygen config file with variable from CMake and move it
    # configure_file([cmake_current_source_dir, '/Doxyfile.dox.cmake.in', cmake_binary_dir, '/doc/Doxyfile-html.dox', '@ONLY'])
    # Configure the html mainpage file of the doxygen documentation with variable
    # from CMake and move it
    # configure_file([cmake_current_source_dir, '/mainpage.dox.in', cmake_binary_dir, '/doc/mainpage.dox', '@ONLY'])
    # configure_file([cmake_current_source_dir, '/openjpip.dox.in', cmake_binary_dir, '/doc/openjpip.dox', '@ONLY'])
    # copy png file to make local (binary tree) documentation valid:
    # configure_file([cmake_current_source_dir, '/jpip_architect.png', cmake_binary_dir, '/doc/html/jpip_architect.png', 'COPYONLY'])
    # configure_file([cmake_current_source_dir, '/jpip_protocol.png', cmake_binary_dir, '/doc/html/jpip_protocol.png', 'COPYONLY'])
    # file(['GLOB', 'headers', openjpeg_source_dir, '/src/lib/openjp2/*.h', openjpeg_source_dir, '/src/lib/openjp2/*.c', openjpeg_source_dir, '/src/lib/openjpip/*.h', openjpeg_source_dir, '/src/lib/openjpip/*.c'])
    # Generate new target to build the html documentation
    # add_custom_command(['OUTPUT', cmake_current_binary_dir, '/html/index.html', 'COMMAND', doxygen_executable, cmake_binary_dir, '/doc/Doxyfile-html.dox', 'DEPENDS', cmake_binary_dir, '/doc/Doxyfile-html.dox', cmake_binary_dir, '/doc/mainpage.dox', cmake_binary_dir, '/doc/openjpip.dox', headers])
    # add_custom_target(['doc', 'ALL', 'DEPENDS', cmake_binary_dir, '/doc/html/index.html', 'COMMENT', 'Building doxygen documentation'])
    # install HTML documentation (install png files too):
    # install(['DIRECTORY', cmake_binary_dir, '/doc/html', 'DESTINATION', 'share/doc', 'PATTERN', '.svn', 'EXCLUDE'])
  else
    message('STATUS: Doxygen not found, we cannot generate the documentation')
  endif
