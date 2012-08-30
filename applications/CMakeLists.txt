# Build the each application if it needed

if(BUILD_CODEC)
  add_subdirectory(codec)
endif()

if(BUILD_MJ2)
  add_subdirectory(mj2)
endif()

# Client & Server:
if(BUILD_JPIP)
  add_subdirectory(jpip)
endif()

if(BUILD_VIEWER)
  add_subdirectory(OPJViewer)
endif()

if(BUILD_JAVA)
	add_subdirectory(JavaOpenJPEG)
endif()
