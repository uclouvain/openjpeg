FIND_PACKAGE(Doxygen)
#
IF(DOXYGEN_EXECUTABLE)
# The Doxyfile.dox is poorly defined and produce output
# in the source dir
ADD_CUSTOM_TARGET(doxygen
# By default doxygen target is added to the 'all' target. Project is small
# thus running doxygen is not too time consuming
  ALL
  ${DOXYGEN}
  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.dox
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
ENDIF(DOXYGEN_EXECUTABLE)

INSTALL(
  FILES       man/man1/image_to_j2k.1
              man/man1/j2k_dump.1
              man/man1/j2k_to_image.1
  DESTINATION ${OPENJPEG_INSTALL_MAN_DIR}/man1)
INSTALL(
  FILES       man/man3/libopenjpeg.3
  DESTINATION ${OPENJPEG_INSTALL_MAN_DIR}/man3)
