CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

IF(UNIX OR CYGWIN)
  SET(CMAKE_INCLUDE_PATH /usr/include /usr/local/include /opt/include 
    /opt/local/include /usr/include/libpng /usr/include/libpng14
	/usr/include/libpng12 /usr/local/include/libpng 
	/usr/local/include/libpng14 /usr/local/include/libpng12
	/opt/include/libpng /opt/include/libpng14 /opt/include/libpng12
	/opt/local/include/libpng /opt/local/include/libpng14)
  SET(CMAKE_LIBRARY_PATH /usr/lib /usr/local/lib /opt/lib /opt/local/lib)
ENDIF(UNIX OR CYGWIN)

#------------
# Try to find lib Z
FIND_PACKAGE(ZLIB)

IF(ZLIB_FOUND)

  SET(Z_LIBNAME ${ZLIB_LIBRARIES} PARENT_SCOPE)
  SET(Z_INCLUDE_DIRNAME ${ZLIB_INCLUDE_DIRS} PARENT_SCOPE) 
  message("Your system seems to have a Z lib available, we will used it to generate PNG lib")

ELSE (ZLIB_FOUND) # not found
  
  IF(BUILD_THIRDPARTY)
    # Try to build it
    message("We will build Z lib from thirdparty")
    ADD_SUBDIRECTORY(libz)
    SET(Z_LIBNAME ${ZLIB_LIBRARIES} PARENT_SCOPE)
    SET(Z_INCLUDE_DIRNAME ${ZLIB_INCLUDE_DIRS} PARENT_SCOPE)
    SET(ZLIB_FOUND 1)
  ELSE (BUILD_THIRDPARTY)
    message("Z lib not found, activate BUILD_THIRDPARTY if you want build it (necessary to build libPNG)")
  ENDIF(BUILD_THIRDPARTY) 
    
ENDIF(ZLIB_FOUND)

#------------
# Try to find lib PNG (which depends to zlib)
IF (ZLIB_FOUND)
  FIND_PACKAGE(PNG)

  IF(PNG_FOUND)

    message("Your system seems to have a PNG lib available, we will used it")
    SET(HAVE_PNG_H 1 PARENT_SCOPE)
    SET(HAVE_LIBPNG 1 PARENT_SCOPE)
    SET(PNG_LIBNAME ${PNG_LIBRARIES} PARENT_SCOPE)
    SET(PNG_INCLUDE_DIRNAME ${PNG_PNG_INCLUDE_DIR} PARENT_SCOPE) 
    
  ELSE(PNG_FOUND) # not found
    
    IF(BUILD_THIRDPARTY)
      # Try to build it
      message("We will build PNG lib from thirdparty")
      ADD_SUBDIRECTORY(libpng)
      SET(HAVE_PNG_H 1 PARENT_SCOPE)
      SET(HAVE_LIBPNG 1 PARENT_SCOPE)
      SET(PNG_LIBNAME ${PNG_LIBRARIES} PARENT_SCOPE)
      SET(PNG_INCLUDE_DIRNAME ${PNG_INCLUDE_DIR} PARENT_SCOPE)
    ELSE (BUILD_THIRDPARTY)
      SET(HAVE_PNG_H 0 PARENT_SCOPE)
      SET(HAVE_LIBPNG 0 PARENT_SCOPE)
      message("PNG lib not found, activate BUILD_THIRDPARTY if you want build it")
    ENDIF(BUILD_THIRDPARTY) 

  ENDIF(PNG_FOUND)
ENDIF (ZLIB_FOUND)

#------------
# Try to find lib TIFF
FIND_PACKAGE(TIFF)

IF(TIFF_FOUND)

  message("Your system seems to have a TIFF lib available, we will used it")
  SET(HAVE_TIFF_H 1 PARENT_SCOPE)
  SET(HAVE_LIBTIFF 1 PARENT_SCOPE)
  SET(TIFF_LIBNAME ${TIFF_LIBRARIES} PARENT_SCOPE)
  SET(TIFF_INCLUDE_DIRNAME ${TIFF_INCLUDE_DIR} PARENT_SCOPE) 

ELSE (TIFF_FOUND) # not found
  
  IF(BUILD_THIRDPARTY)
    # Try to build it
    message("We will build TIFF lib from thirdparty")
    ADD_SUBDIRECTORY(libtiff)
    SET(TIFF_LIBNAME ${TIFF_LIBRARIES} PARENT_SCOPE)
    SET(TIFF_INCLUDE_DIRNAME ${TIFF_INCLUDE_DIR} PARENT_SCOPE)
    SET(HAVE_TIFF_H 1 PARENT_SCOPE)
    SET(HAVE_LIBTIFF 1 PARENT_SCOPE)
  ELSE (BUILD_THIRDPARTY)
    SET(HAVE_TIFF_H 0 PARENT_SCOPE)
    SET(HAVE_LIBTIFF 0 PARENT_SCOPE)
    message("TIFF lib not found, activate BUILD_THIRDPARTY if you want build it")
  ENDIF(BUILD_THIRDPARTY) 
    
ENDIF(TIFF_FOUND)

#------------
# Try to find lib LCMS2 (or by default LCMS)
FIND_PACKAGE(LCMS2)

IF(LCMS2_FOUND)

  message("Your system seems to have a LCMS2 lib available, we will used it")
  SET(HAVE_LCMS2_H 1 PARENT_SCOPE)
  SET(HAVE_LIBLCMS2 1 PARENT_SCOPE)
  SET(LCMS_LIBNAME ${LCMS2_LIBRARIES} PARENT_SCOPE)
  SET(LCMS_INCLUDE_DIRNAME ${LCMS2_INCLUDE_DIRS} PARENT_SCOPE) 

ELSE (LCMS2_FOUND) # not found lcms2
  # try to find LCMS
  FIND_PACKAGE(LCMS)  
  
  IF(LCMS_FOUND)
  
    message("Your system seems to have a LCMS lib available, we will used it")
    SET(HAVE_LCMS_H 1 PARENT_SCOPE)
    SET(HAVE_LIBLCMS 1 PARENT_SCOPE)
    SET(LCMS_LIBNAME ${LCMS_LIBRARIES} PARENT_SCOPE)
    SET(LCMS_INCLUDE_DIRNAME ${LCMS_INCLUDE_DIRS} PARENT_SCOPE) 
   
  ELSE (LCMS_FOUND) # not found lcms
    SET(HAVE_LCMS_H 0 PARENT_SCOPE)
    SET(HAVE_LIBLCMS 0 PARENT_SCOPE)
    
    IF( BUILD_THIRDPARTY)

      # Try to build lcms2
      message("We will build LCMS2 lib from thirdparty")
      ADD_SUBDIRECTORY(liblcms2)
      SET(LCMS_LIBNAME "lcms2" PARENT_SCOPE)
      SET(LCMS_INCLUDE_DIRNAME ${OPENJPEG_SOURCE_DIR}/thirdparty/liblcms2/include PARENT_SCOPE) #
      SET(HAVE_LCMS2_H 1 PARENT_SCOPE)
      SET(HAVE_LIBLCMS2 1 PARENT_SCOPE)
    ELSE (BUILD_THIRDPARTY)
        SET(HAVE_LCMS2_H 0 PARENT_SCOPE)
        SET(HAVE_LIBLCMS2 0 PARENT_SCOPE)
        message("LCMS2 or LCMS lib not found, activate BUILD_THIRDPARTY if you want build it")
    ENDIF(BUILD_THIRDPARTY) 
    
  ENDIF (LCMS_FOUND)
ENDIF(LCMS2_FOUND)


