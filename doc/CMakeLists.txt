# Generate target to build the html documentatiop through CMake tool
# After configure the project with the BUILD_DOC option you can run make html 
# to generate the html documentation in the doc/html repository of the build folder.
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Try to find the doxygen tool
FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND)

  # Configure the doxygen config file with variable from CMake and move it
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-html.dox.cmake ${CMAKE_BINARY_DIR}/doc/Doxyfile-html.dox @ONLY)
  
  # Configure the html mainpage file of the doxygen documentation with variable from CMake and move it  
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox.cmake ${CMAKE_BINARY_DIR}/doc/mainpage.dox)

  # Generate new target to build the html documentation
  ADD_CUSTOM_TARGET(html ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile-html.dox)

ELSE(DOXYGEN_FOUND)

  MESSAGE(STATUS "Doxygen not found, we cannot generate the documentation")

ENDIF(DOXYGEN_FOUND)
