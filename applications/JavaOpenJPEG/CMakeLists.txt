#JavaOpenJPEG/CMakeLists.txt

# First thing define the common source:
SET(openjpegjni_SRCS
  JavaOpenJPEGDecoder.c
  JavaOpenJPEG.c
  )

# JNI binding:
find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})

# required header file:
include_directories(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  ${OPENJPEG_SOURCE_DIR}/applications/common
  ${OPENJPEG_SOURCE_DIR}/applications/codec
  )

add_library(openjpegjni MODULE
  ${openjpegjni_SRCS}
  )

# build jar:
FIND_PACKAGE(Java 1.5 REQUIRED) # javac, jar

# build dep list:
file(GLOB java_srcs "java-sources/org/openJpeg/*.java")

# make sure target javac dir exists:
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/classes)
# Build java
ADD_CUSTOM_COMMAND(
  OUTPUT ${LIBRARY_OUTPUT_PATH}/openjpeg.jar
  COMMAND ${Java_JAVAC_EXECUTABLE} -sourcepath  "${CMAKE_CURRENT_SOURCE_DIR}/java-sources"
  ${java_srcs} -d ${CMAKE_CURRENT_BINARY_DIR}/classes
  COMMAND ${Java_JAR_EXECUTABLE} cvf ${LIBRARY_OUTPUT_PATH}/openjpeg.jar org
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/classes
  DEPENDS ${java_srcs}
  COMMENT "javac *.java; jar cvf -> openjpeg.jar"
)

# name the target
ADD_CUSTOM_TARGET(OpenJPEGJavaJar ALL
  DEPENDS ${LIBRARY_OUTPUT_PATH}/openjpeg.jar
  COMMENT "building openjpeg.jar"
)
