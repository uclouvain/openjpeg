# Generate target to build the html documentation through CMake tool
# After having configured the project with the BUILD_DOC option you can run make doc
# to generate the html documentation in the doc/html repository of the build folder.

# Try to find the doxygen tool
find_package(Doxygen)

if(DOXYGEN_FOUND)
  # Configure the doxygen config file with variable from CMake and move it
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.dox.cmake.in
    ${CMAKE_BINARY_DIR}/doc/Doxyfile-html.dox @ONLY)

  # Configure the html mainpage file of the doxygen documentation with variable
  # from CMake and move it  
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox.cmake
    ${CMAKE_BINARY_DIR}/doc/mainpage.dox @ONLY)

  # Generate new target to build the html documentation
  add_custom_target(doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile-html.dox)

else(DOXYGEN_FOUND)

  message(STATUS "Doxygen not found, we cannot generate the documentation")

endif(DOXYGEN_FOUND)
