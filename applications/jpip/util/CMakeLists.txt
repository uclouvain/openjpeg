# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/applications/jpip/libopenjpip
  ${FCGI_INCLUDE_DIRS}
)

IF(BUILD_JPIP_SERVER)
  
  SET(OPJ_SERVER_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/opj_server.c
  )
  
  # Build executable
  ADD_EXECUTABLE(opj_server ${OPJ_SERVER_SRCS})
  TARGET_LINK_LIBRARIES(opj_server openjpip_server)
  SET_PROPERTY(
    TARGET opj_server
    APPEND PROPERTY
    COMPILE_DEFINITIONS SERVER QUIT_SIGNAL="quitJPIP"
    )

  # On unix you need to link to the math library:
  IF(UNIX)
    TARGET_LINK_LIBRARIES(opj_server m)
  ENDIF(UNIX)

  # Install exe
  INSTALL(TARGETS opj_server
    EXPORT OpenJPEGTargets
    DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
    )
ENDIF(BUILD_JPIP_SERVER)

SET(EXES
  opj_dec_server
  jpip_to_jp2
  jpip_to_j2k
  test_index
  )
FOREACH(exe ${EXES})
  ADD_EXECUTABLE(${exe} ${exe}.c)
  TARGET_LINK_LIBRARIES(${exe} openjpip_local)
  INSTALL(TARGETS ${exe}
    EXPORT OpenJPEGTargets
    DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
    )
ENDFOREACH(exe)

# Build the two java clients:
FIND_PACKAGE(Java 1.5 REQUIRED) # javac, jar

# 1. opj_viewer
# build dep list:
file(GLOB java1_srcs "opj_viewer/src/*.java")

# make sure target javac dir exists:
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/classes1)
# Build java
ADD_CUSTOM_COMMAND(
  OUTPUT ${LIBRARY_OUTPUT_PATH}/opj_viewer.jar
  COMMAND ${Java_JAVAC_EXECUTABLE} 
  ${java1_srcs} -d ${CMAKE_CURRENT_BINARY_DIR}/classes1
  COMMAND ${Java_JAR_EXECUTABLE} cfm ${LIBRARY_OUTPUT_PATH}/opj_viewer.jar
  ${CMAKE_CURRENT_SOURCE_DIR}/opj_viewer/dist/manifest.txt -C
  ${CMAKE_CURRENT_BINARY_DIR}/classes1 .
  DEPENDS ${java1_srcs}
  ${CMAKE_CURRENT_SOURCE_DIR}/opj_viewer/dist/manifest.txt
  COMMENT "javac *.java; jar cvf -> opj_viewer.jar"
)

# name the target
ADD_CUSTOM_TARGET(OPJViewerJar ALL
  DEPENDS ${LIBRARY_OUTPUT_PATH}/opj_viewer.jar
  COMMENT "building opj_viewer.jar"
)

# 2. opj_viewer_xerces
# search for package org.apache.xerces.parsers
find_file(APACHE_XERCES_JAR
  NAMES xerces-j2.jar xercesImpl.jar
  PATHS /usr/share/java/
  NO_DEFAULT_PATH
  )
mark_as_advanced(APACHE_XERCES_JAR)

if(EXISTS ${APACHE_XERCES_JAR})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_viewer_xerces/dist/manifest.txt.in
    ${CMAKE_CURRENT_BINARY_DIR}/opj_viewer_xerces/dist/manifest.txt
    @ONLY
    )
  # build dep list:
  file(GLOB java2_srcs "opj_viewer_xerces/src/*.java")

  # Need some common files:
  list(APPEND java2_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_viewer/src/ImageManager.java
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_viewer/src/ImgdecClient.java
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_viewer/src/JPIPHttpClient.java
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_viewer/src/MML.java
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_viewer/src/PnmImage.java
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_viewer/src/RegimViewer.java
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_viewer/src/ResizeListener.java
    )

  # make sure target javac dir exists:
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/classes2)
  # Build java
  ADD_CUSTOM_COMMAND(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/opj_viewer_xerces.jar
    COMMAND ${Java_JAVAC_EXECUTABLE} 
    -classpath ${APACHE_XERCES_JAR}
    ${java2_srcs} -d ${CMAKE_CURRENT_BINARY_DIR}/classes2
    COMMAND ${Java_JAR_EXECUTABLE} cfm ${LIBRARY_OUTPUT_PATH}/opj_viewer_xerces.jar
    ${CMAKE_CURRENT_BINARY_DIR}/opj_viewer_xerces/dist/manifest.txt 
    -C ${CMAKE_CURRENT_BINARY_DIR}/classes2 .
    DEPENDS ${java2_srcs}
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_viewer_xerces/dist/manifest.txt.in
    COMMENT "javac *.java; jar cvf -> opj_viewer_xerces.jar"
    )

  # name the target
  ADD_CUSTOM_TARGET(OPJViewerXercesJar ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/opj_viewer_xerces.jar
    COMMENT "building opj_viewer_xerces.jar"
    )
endif()
