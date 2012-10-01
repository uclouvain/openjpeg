set(OPENMJ2_LIBRARY_NAME openmj2)
set(OPENMJ2_SRCS
  mj2.c
  mj2_convert.c
)

# Build the library
if(WIN32)
  if(BUILD_SHARED_LIBS)
    add_definitions(-DOPJ_EXPORTS)
  else(BUILD_SHARED_LIBS)
    add_definitions(-DOPJ_STATIC)
  endif(BUILD_SHARED_LIBS)
endif(WIN32)

include_directories(
  ${OPENJPEG_BINARY_DIR}/src/lib/openjp2 # opj_config.h
  ${OPENJPEG_SOURCE_DIR}/src/lib/openjp2
  )

# build mj2 lib:
add_library(${OPENMJ2_LIBRARY_NAME} ${OPENMJ2_SRCS})
target_link_libraries(${OPENMJ2_LIBRARY_NAME} openjp2)
if(UNIX)
  target_link_libraries(${OPENMJ2_LIBRARY_NAME} m)
endif()
set_target_properties(${OPENMJ2_LIBRARY_NAME} PROPERTIES ${OPENJPEG_LIBRARY_PROPERTIES})

# Install library
install(TARGETS ${OPENMJ2_LIBRARY_NAME}
  EXPORT OpenMJ2Targets
  DESTINATION ${OPENJPEG_INSTALL_LIB_DIR} 
  COMPONENT Libraries
)

# Install includes files
#INSTALL(FILES mj2.h
#  DESTINATION ${OPENJPEG_INSTALL_INCLUDE_DIR}/${subdir} 
#  COMPONENT Headers
#)
