# Build the each application if it needed

if(BUILD_CODEC)
  add_subdirectory(codec)
endif(BUILD_CODEC)

if(BUILD_MJ2)
  add_subdirectory(mj2)
endif(BUILD_MJ2)

# Client & Server:
if(BUILD_JPIP)
  add_subdirectory(jpip)
endif(BUILD_JPIP)

if(BUILD_VIEWER)
  add_subdirectory(OPJViewer)
endif(BUILD_VIEWER)

if(BUILD_JAVA)
	add_subdirectory(JavaOpenJPEG)
endif(BUILD_JAVA)
