include_regular_expression("^.*$")

if (BUILD_WITH_OPENMP)
find_package(OpenMP)
if (OPENMP_FOUND)
	add_definitions(-DOPJ_NUM_CORES=8)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
endif()


install( FILES  ${CMAKE_CURRENT_BINARY_DIR}/opj_config.h
 DESTINATION ${OPENJPEG_INSTALL_INCLUDE_DIR} COMPONENT Headers)

include_directories(
  ${OPENJPEG_BINARY_DIR}/src/lib/openjp2 # opj_config.h and opj_config_private.h
  ${OPENJPEG_SOURCE_DIR}/src/bin/jp2
  ${OPENJPEG_SOURCE_DIR}/src/lib/openjp2
  
)
# Defines the source code for the library
set(OPENJPEG_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/dwt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/t1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/t1_opt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/mqc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/mct.c
  ${CMAKE_CURRENT_SOURCE_DIR}/raw.c
  ${CMAKE_CURRENT_SOURCE_DIR}/tcd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/t1_plugin.c
)

# Build the library
if(WIN32)
    add_definitions(-DOPJ_EXPORTS)
endif()
add_library(${OPENJPEG_PLUGIN_NAME} ${OPENJPEG_SRCS})
if(UNIX)
  target_link_libraries(${OPENJPEG_PLUGIN_NAME} m)
endif()
set_target_properties(${OPENJPEG_PLUGIN_NAME} PROPERTIES ${OPENJPEG_LIBRARY_PROPERTIES})

# Install library
install(TARGETS ${OPENJPEG_PLUGIN_NAME}
  EXPORT OpenJPEGTargets
  RUNTIME DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
  LIBRARY DESTINATION ${OPENJPEG_INSTALL_LIB_DIR} COMPONENT Libraries
  ARCHIVE DESTINATION ${OPENJPEG_INSTALL_LIB_DIR} COMPONENT Libraries
)

# Install includes files
install(FILES openjpeg.h opj_stdint.h
  DESTINATION ${OPENJPEG_INSTALL_INCLUDE_DIR} COMPONENT Headers
)

if(OPJ_USE_DSYMUTIL)
    add_custom_command(TARGET ${OPENJPEG_PLUGIN_NAME} POST_BUILD 
    COMMAND "dsymutil" "$<TARGET_SONAME_FILE:${OPENJPEG_PLUGIN_NAME}>"
    COMMENT "dsymutil $<TARGET_SONAME_FILE:${OPENJPEG_PLUGIN_NAME}>"
    DEPENDS ${OPENJPEG_PLUGIN_NAME})
endif()
