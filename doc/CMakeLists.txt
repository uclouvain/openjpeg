FIND_PACKAGE(Doxygen)
#
IF(DOXYGEN_EXECUTABLE)
# The Doxyfile.dox is poorly defined and produce output
# in the source dir
ADD_CUSTOM_TARGET(doc
  ${DOXYGEN}
  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.dox
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
ENDIF(DOXYGEN_EXECUTABLE)

