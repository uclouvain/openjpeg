# Linux makefile for OpenJPEG

#VER_MAJOR = 1
#VER_MINOR = 0.0

# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/codec
  ${CMAKE_CURRENT_SOURCE_DIR}/libjp3dvm
  )


SUBDIRS(
  libjp3dvm
  codec
)


