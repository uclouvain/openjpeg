#JavaOpenJPEG/CMakeLists.txt

# First thing define the common source:
SET(openjpegjni_SRCS
  JavaOpenJPEGDecoder.c
  JavaOpenJPEG.c
  ${OPENJPEG_SOURCE_DIR}/applications/codec/index.c
  ${OPENJPEG_SOURCE_DIR}/applications/codec/convert.c
  #${OPENJPEG_SOURCE_DIR}/applications/common/color.c
  )
if(NOT USE_SYSTEM_GETOPT)
  list(APPEND openjpegjni_SRCS
    ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c
  )
endif()

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
# Java module should not have a SONAME:
set_property(TARGET openjpegjni PROPERTY NO_SONAME 1)

TARGET_LINK_LIBRARIES(openjpegjni ${OPENJPEG_LIBRARY_NAME})

if(UNIX)
  target_link_libraries(openjpegjni m)
endif()

INSTALL(TARGETS openjpegjni
  EXPORT OpenJPEGTargets
  LIBRARY DESTINATION ${OPENJPEG_INSTALL_JNI_DIR} COMPONENT Libraries
)

# build jar:
FIND_PACKAGE(Java 1.5 REQUIRED) # javac, jar

# User can override this:
if(NOT DEFINED JAVA_SOURCE_VERSION) 
  set(JAVA_SOURCE_VERSION 1.5)
endif()
if(NOT DEFINED JAVA_TARGET_VERSION)
  set(JAVA_TARGET_VERSION 1.5)
endif()

# build dep list:
file(GLOB java_srcs "java-sources/org/openJpeg/*.java")

# make sure target javac dir exists:
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/classes)
# Build java
ADD_CUSTOM_COMMAND(
  OUTPUT ${LIBRARY_OUTPUT_PATH}/openjpeg.jar
  COMMAND ${Java_JAVAC_EXECUTABLE}
  -source ${JAVA_SOURCE_VERSION} -target ${JAVA_TARGET_VERSION}
  -sourcepath  "${CMAKE_CURRENT_SOURCE_DIR}/java-sources"
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

INSTALL(FILES ${LIBRARY_OUTPUT_PATH}/openjpeg.jar
  DESTINATION ${OPENJPEG_INSTALL_SHARE_DIR} COMPONENT JavaModule
)
