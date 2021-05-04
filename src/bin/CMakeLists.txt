# source code for openjpeg apps:
add_subdirectory(common)
# Part 1 & 2:
add_subdirectory(jp2)

# optionals components:
if(BUILD_JPIP)
  add_subdirectory(jpip)
endif()

# wx apps:
add_subdirectory(wx)
