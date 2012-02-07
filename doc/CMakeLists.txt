# Generate target to build the html documentation through CMake tool
# After having configured the project with the BUILD_DOC option you can run make doc
# to generate the html documentation in the doc/html repository of the build folder.

# Try to find the doxygen tool
FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND)
  # Configure the doxygen config file with variable from CMake and move it
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.dox.cmake.in
    ${CMAKE_BINARY_DIR}/doc/Doxyfile-html.dox @ONLY)
  
  # Generate new target to build the html documentation
  ADD_CUSTOM_TARGET(doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile-html.dox)

ELSE(DOXYGEN_FOUND)

  MESSAGE(STATUS "Doxygen not found, we cannot generate the documentation")

ENDIF(DOXYGEN_FOUND)
