  #JavaOpenJPEG/CMakeLists.txt
  # First thing define the common source:
  openjpegjni_srcs = ['JavaOpenJPEGDecoder.c', 'JavaOpenJPEG.c', openjpeg_source_dir, '/src/bin/common/opj_getopt.c', openjpeg_source_dir, '/src/bin/jp2/convert.c', 'index.c']
  # JNI binding:
  JNI_dep = dependency('JNI')
  # include_directories(jni_include_dirs)
  # required header file:
  # include_directories([openjpeg_binary_dir, '/src/lib/openjp2', openjpeg_source_dir, '/src/lib/openmj2', openjpeg_source_dir, '/src/bin/common', openjpeg_source_dir, '/src/bin/jp2'])
  openjpegjni_lib = library('openjpegjni', 'MODULE', openjpegjni_srcs)
  # Java module should not have a SONAME:
  # set_property(['TARGET', 'openjpegjni', 'PROPERTY', 'NO_SONAME', '1'])
  # FIXME (need to use old API):
  if get_option('build_mj2')
    # target_link_libraries(['openjpegjni', 'openmj2'])
  endif
  # target_link_libraries(['openjpegjni', png_libname, tiff_libname, lcms_libname, z_libname])
  if host_system == 'linux'
    # target_link_libraries(['openjpegjni', 'm'])
  endif
  # install(['TARGETS', 'openjpegjni', 'EXPORT', 'OpenJPEGTargets', 'LIBRARY', 'DESTINATION', openjpeg_install_jni_dir, 'COMPONENT', 'Libraries'])
  # build jar:
  Java_dep = dependency('Java')
  # javac, jar
  # build dep list:
  # file(['GLOB', 'java_srcs', 'java-sources/org/openJpeg/*.java'])
  # make sure target javac dir exists:
  # file(['MAKE_DIRECTORY', cmake_current_binary_dir, '/classes'])
  # Build java
  # add_custom_command(['OUTPUT', library_output_path, '/openjpeg.jar', 'COMMAND', java_javac_executable, '-sourcepath', '${CMAKE_CURRENT_SOURCE_DIR}/java-sources', java_srcs, '-d', cmake_current_binary_dir, '/classes', 'COMMAND', java_jar_executable, 'cvf', library_output_path, '/openjpeg.jar', 'org', 'WORKING_DIRECTORY', cmake_current_binary_dir, '/classes', 'DEPENDS', java_srcs, 'COMMENT', 'javac *.java; jar cvf -> openjpeg.jar'])
  # name the target
  # add_custom_target(['OpenJPEGJavaJar', 'ALL', 'DEPENDS', library_output_path, '/openjpeg.jar', 'COMMENT', 'building openjpeg.jar'])
  # install(['FILES', library_output_path, '/openjpeg.jar', 'DESTINATION', openjpeg_install_share_dir, 'COMPONENT', 'JavaModule'])
