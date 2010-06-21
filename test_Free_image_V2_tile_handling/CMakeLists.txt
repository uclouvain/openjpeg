# Build the demo app, small examples

# First thing define the common source:

# Then check if getopt is present:
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)

# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  )

# Do the proper thing when building static...if only there was configured
# headers or def files instead
IF(NOT BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DOPJ_STATIC)
ENDIF(NOT BUILD_SHARED_LIBS)

INCLUDE(${OPENJPEG_SOURCE_DIR}/CMake/Free_CMakeImport.cmake)

# Loop over all executables:
FOREACH(exe test2_encoder test2_decoder)
  
  ADD_EXECUTABLE(${exe} ${exe}.c)
  TARGET_LINK_LIBRARIES(${exe} ${OPJ_PREFIX}openjpeg)
  TARGET_LINK_LIBRARIES(${exe} ${FREEIMAGE_LIBRARIES})
ENDFOREACH(exe)

