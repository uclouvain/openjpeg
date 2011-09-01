CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE_DIRECTORIES(${OPENJPEG_SOURCE_DIR}/libopenjpeg
                    ${OPENJPEG_SOURCE_DIR}/applications/codec
                    ${OPENJPEG_SOURCE_DIR}/applications/common
                    ${Z_INCLUDE_DIRNAME}
                    ${PNG_INCLUDE_DIRNAME}
                    ${TIFF_INCLUDE_DIRNAME}
                    )

# First thing define the common source:
SET(comparePGXimages_SRCS comparePGXimages.c ${OPENJPEG_SOURCE_DIR}/applications/codec/convert.c ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c)

SET(compare_dump_files_SRCS compare_dump_files.c ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c)

SET(compareRAWimages_SRCS compareRAWimages.c ${OPENJPEG_SOURCE_DIR}/applications/common/opj_getopt.c)

ADD_EXECUTABLE(comparePGXimages ${comparePGXimages_SRCS})
TARGET_LINK_LIBRARIES(comparePGXimages ${OPENJPEG_LIBRARY_NAME}
                                       ${Z_LIBNAME}
                                       ${PNG_LIBNAME}
                                       ${TIFF_LIBNAME}
                                       )
                                       
ADD_EXECUTABLE(compare_dump_files ${compare_dump_files_SRCS})

ADD_EXECUTABLE(compareRAWimages ${compareRAWimages_SRCS})

# No image send to the dashboard if lib PNG is not available.
IF(NOT HAVE_LIBPNG)
  MESSAGE(WARNING "Lib PNG seems to be not available: if you want run the non-regression tests with images reported to the dashboard, you need it (try BUILD_THIRDPARTY)") 
ENDIF(NOT HAVE_LIBPNG)

ADD_SUBDIRECTORY(conformance)
ADD_SUBDIRECTORY(nonregression)
