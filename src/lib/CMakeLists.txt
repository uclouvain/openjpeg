# source code for openjpeg project:
# Part 1 & 2:
add_subdirectory(openjp2)

# optionals components:
if(BUILD_JPIP)
  add_subdirectory(openjpip)
endif()
if(BUILD_JPWL)
  add_subdirectory(openjpwl)
endif()
if(BUILD_JP3D)
  add_subdirectory(openjp3d)
endif()
