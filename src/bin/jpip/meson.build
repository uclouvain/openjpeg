# Headers file are located here:
inc_dirs = ['../../lib/openjp2',
            '../common',
            '../jp2',
            '../../lib/openjpip']

# Tool to embed metadata into JP2 file
opj_jpip_addxml_exe = executable ('opj_jpip_addxml', ['opj_jpip_addxml.c'], install : true)

# Install exe
if get_option('build_jpip_server')
  opj_server_srcs = ['opj_server.c']
  # Build executable

  # target_link_libraries(['opj_server', fcgi_libraries, 'openjpip_server'])
  # set_property(['TARGET', 'opj_server', 'APPEND', 'PROPERTY', 'COMPILE_DEFINITIONS', 'SERVER', 'QUIT_SIGNAL=', 'quitJPIP'])
  # On unix you need to link to the math library:

  opj_server_exe = executable('opj_server', opj_server_srcs,
                              dependencies : [m_dep],
                              install : True)
endif

# Loop over all executables:
foreach exe :  ['opj_dec_server', 'opj_jpip_transcode', 'opj_jpip_test']
  exe_c = exe + '.c'
  exe_c_args = []
  exe_src = [exe_c] + common_srcs
  exe_deps = [libopenjpip_dep, rt_dep, m_dep]
  exe_exe = executable (exe, exe_src
                        , c_args : exe_c_args
                        , dependencies: exe_deps
                        , install: true
                        )
endforeach

message ('no java client')
# Build the two java clients:
#java_dep = dependency('Java')
# javac, jar
# User can override this:
#if 'NOT', 'DEFINED', 'JAVA_SOURCE_VERSION'
#  java_source_version = '1.5'
#endif
#if 'NOT', 'DEFINED', 'JAVA_TARGET_VERSION'
#  java_target_version = '1.5'
#endif

# Only build the java viewer if dev is found:
#if 'Java_Development_FOUND' and 'Java_JAVAC_EXECUTABLE'
  #jflags = '$ENV{JFLAGS}'
  # search for package org.apache.xerces.parsers
  # find_file(['APACHE_XERCES_JAR', 'NAMES', 'xerces-j2.jar', 'xercesImpl.jar', 'PATHS', '/usr/share/java/', 'NO_DEFAULT_PATH'])
  # mark_as_advanced('APACHE_XERCES_JAR')
  # Decide to build the simple viewer or the xerces one:
  #if 'EXISTS', apache_xerces_jar
    # configure_file([cmake_current_source_dir, '/opj_viewer_xerces/dist/manifest.txt.in', cmake_current_binary_dir, '/opj_viewer_xerces/dist/manifest.txt', '@ONLY'])
    # build dep list:
    # file(['GLOB', 'java2_srcs', 'opj_viewer_xerces/src/*.java'])
    # Need some common files:
    # list(['APPEND', 'java2_srcs', cmake_current_source_dir, '/opj_viewer/src/ImageManager.java', cmake_current_source_dir, '/opj_viewer/src/ImgdecClient.java', cmake_current_source_dir, '/opj_viewer/src/JPIPHttpClient.java', cmake_current_source_dir, '/opj_viewer/src/MML.java', cmake_current_source_dir, '/opj_viewer/src/PnmImage.java', cmake_current_source_dir, '/opj_viewer/src/RegimViewer.java', cmake_current_source_dir, '/opj_viewer/src/ResizeListener.java'])
    # make sure target javac dir exists:
    # file(['MAKE_DIRECTORY', cmake_current_binary_dir, '/classes2'])
    # Build java
    # add_custom_command(['OUTPUT', library_output_path, '/opj_jpip_viewer.jar', 'COMMAND', java_javac_executable, jflags, '-source', java_source_version, '-target', java_target_version, '-classpath', apache_xerces_jar, java2_srcs, '-d', cmake_current_binary_dir, '/classes2', 'COMMAND', java_jar_executable, 'cfm', library_output_path, '/opj_jpip_viewer.jar', cmake_current_binary_dir, '/opj_viewer_xerces/dist/manifest.txt', '-C', cmake_current_binary_dir, '/classes2', '.', 'DEPENDS', java2_srcs, cmake_current_source_dir, '/opj_viewer_xerces/dist/manifest.txt.in', 'COMMENT', 'javac *.java; jar cvf -> opj_viewer_xerces.jar'])
    # name the target
    # add_custom_target(['OPJViewerXercesJar', 'ALL', 'DEPENDS', library_output_path, '/opj_jpip_viewer.jar', 'COMMENT', 'building opj_jpip_viewer.jar (xerces)'])
    # install(['FILES', library_output_path, '/opj_jpip_viewer.jar', 'DESTINATION', openjpeg_install_share_dir, 'COMPONENT', 'JavaModule'])
  #else
    # opj_viewer (simple, no xerces)
    # build dep list:
    # file(['GLOB', 'java1_srcs', 'opj_viewer/src/*.java'])
    # make sure target javac dir exists:
    # file(['MAKE_DIRECTORY', cmake_current_binary_dir, '/classes1'])
    # Build java
    # add_custom_command(['OUTPUT', library_output_path, '/opj_jpip_viewer.jar', 'COMMAND', java_javac_executable, jflags, '-source', java_source_version, '-target', java_target_version, java1_srcs, '-d', cmake_current_binary_dir, '/classes1', 'COMMAND', java_jar_executable, 'cfm', library_output_path, '/opj_jpip_viewer.jar', cmake_current_source_dir, '/opj_viewer/dist/manifest.txt', '-C', cmake_current_binary_dir, '/classes1', '.', 'DEPENDS', java1_srcs, cmake_current_source_dir, '/opj_viewer/dist/manifest.txt', 'COMMENT', 'javac *.java; jar cvf -> opj_jpip_viewer.jar'])
    # name the target
    # add_custom_target(['OPJViewerJar', 'ALL', 'DEPENDS', library_output_path, '/opj_jpip_viewer.jar', 'COMMENT', 'building opj_jpip_viewer.jar (no xerces found)'])
    # install(['FILES', library_output_path, '/opj_jpip_viewer.jar', 'DESTINATION', openjpeg_install_share_dir, 'COMPONENT', 'JavaModule'])
  #endif
#else
  # message(['WARNING', 'No java compiler found. Wont be able to build java viewer'])
#endif
