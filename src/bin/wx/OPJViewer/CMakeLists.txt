project(viewer CXX)

find_package(wxWidgets REQUIRED)
include(${wxWidgets_USE_FILE})

include_directories(
  ${OPENJPEG_SOURCE_DIR}/src/lib
  ${OPENJPEG_SOURCE_DIR}/src/bin
  ${OPENJPEG_SOURCE_DIR}/src/lib/openjp2
  ${OPENJPEG_BINARY_DIR}/src/lib/openjp2 # opj_config.h and opj_config_private.h
  )

add_definitions(-DwxUSE_LIBOPENJPEG -DOPENJPEG_VERSION="${OPENJPEG_VERSION}")
set(OPJV_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/source/imagjpeg2000.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/source/wxj2kparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/source/OPJViewer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/source/wxjp2parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/source/OPJDialogs.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/source/OPJThreads.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/source/OPJAbout.cpp
  ${OPENJPEG_SOURCE_DIR}/src/bin/jp2/index.c
  )
add_executable(opjviewer ${OPJV_SRCS})
target_link_libraries(opjviewer ${wxWidgets_LIBRARIES} openjpeg)
