# - Define macro to check large file support
#
#  OPJ_TEST_LARGE_FILES(VARIABLE)
#
#  VARIABLE will be set to true if off_t is 64 bits, and fseeko/ftello present.
#  This macro will also defines the necessary variable enable large file support, for instance
#  _LARGE_FILES
#  _LARGEFILE_SOURCE
#  _FILE_OFFSET_BITS 64  
#  HAVE_FSEEKO
#
#  However, it is YOUR job to make sure these defines are set in a #cmakedefine so they
#  end up in a config.h file that is included in your source if necessary!
#
#  Adapted from Gromacs project (http://www.gromacs.org/)
#  by Julien Malik
#  

MACRO(OPJ_TEST_LARGE_FILES VARIABLE)
    IF("${VARIABLE}" MATCHES "^${VARIABLE}$")

        # On most platforms it is probably overkill to first test the flags for 64-bit off_t,
        # and then separately fseeko. However, in the future we might have 128-bit filesystems
        # (ZFS), so it might be dangerous to indiscriminately set e.g. _FILE_OFFSET_BITS=64.

        MESSAGE(STATUS "Checking for 64-bit off_t")

        # First check without any special flags
        TRY_COMPILE(FILE64_OK "${PROJECT_BINARY_DIR}"
                    "${PROJECT_SOURCE_DIR}/CMake/TestFileOffsetBits.c")
        if(FILE64_OK)
          MESSAGE(STATUS "Checking for 64-bit off_t - present")
       	endif(FILE64_OK)

        if(NOT FILE64_OK)
            # Test with _FILE_OFFSET_BITS=64
            TRY_COMPILE(FILE64_OK "${PROJECT_BINARY_DIR}"
                        "${PROJECT_SOURCE_DIR}/CMake/TestFileOffsetBits.c"
                        COMPILE_DEFINITIONS "-D_FILE_OFFSET_BITS=64" )
            if(FILE64_OK)
                MESSAGE(STATUS "Checking for 64-bit off_t - present with _FILE_OFFSET_BITS=64")
                set(_FILE_OFFSET_BITS 64)
            endif(FILE64_OK)
        endif(NOT FILE64_OK)

        if(NOT FILE64_OK)
            # Test with _LARGE_FILES
            TRY_COMPILE(FILE64_OK "${PROJECT_BINARY_DIR}"
                        "${PROJECT_SOURCE_DIR}/CMake/TestFileOffsetBits.c"
                        COMPILE_DEFINITIONS "-D_LARGE_FILES" )
            if(FILE64_OK)
                MESSAGE(STATUS "Checking for 64-bit off_t - present with _LARGE_FILES")
                set(_LARGE_FILES 1)
            endif(FILE64_OK)
        endif(NOT FILE64_OK)
	
        if(NOT FILE64_OK)
            # Test with _LARGEFILE_SOURCE
            TRY_COMPILE(FILE64_OK "${PROJECT_BINARY_DIR}"
                        "${PROJECT_SOURCE_DIR}/CMake/TestFileOffsetBits.c"
                        COMPILE_DEFINITIONS "-D_LARGEFILE_SOURCE" )
            if(FILE64_OK)
                MESSAGE(STATUS "Checking for 64-bit off_t - present with _LARGEFILE_SOURCE")
                set(_LARGEFILE_SOURCE 1)
            endif(FILE64_OK)
        endif(NOT FILE64_OK)


        #if(NOT FILE64_OK)
        #    # now check for Windows stuff
        #    TRY_COMPILE(FILE64_OK "${PROJECT_BINARY_DIR}"
        #                "${PROJECT_SOURCE_DIR}/CMake/TestWindowsFSeek.c")
        #    if(FILE64_OK)
        #        MESSAGE(STATUS "Checking for 64-bit off_t - present with _fseeki64")
        #        set(HAVE__FSEEKI64 1)
        #    endif(FILE64_OK)
        #endif(NOT FILE64_OK)

        if(NOT FILE64_OK)
            MESSAGE(STATUS "Checking for 64-bit off_t - not present")
        endif(NOT FILE64_OK)
        
        SET(_FILE_OFFSET_BITS ${_FILE_OFFSET_BITS} CACHE INTERNAL "Result of test for needed _FILE_OFFSET_BITS=64")
        SET(_LARGE_FILES      ${_LARGE_FILES}      CACHE INTERNAL "Result of test for needed _LARGE_FILES")
        SET(_LARGEFILE_SOURCE ${_LARGEFILE_SOURCE} CACHE INTERNAL "Result of test for needed _LARGEFILE_SOURCE")

        # Set the flags we might have determined to be required above
        CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/CMake/TestLargeFiles.c.cmake.in" 
                       "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestLargeFiles.c")

        MESSAGE(STATUS "Checking for fseeko/ftello")
        
	    # Test if ftello/fseeko are	available
	    TRY_COMPILE(FSEEKO_COMPILE_OK
	                "${PROJECT_BINARY_DIR}"
                    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestLargeFiles.c")
	    
	    IF(FSEEKO_COMPILE_OK)
            MESSAGE(STATUS "Checking for fseeko/ftello - present")
        ENDIF(FSEEKO_COMPILE_OK)

        IF(NOT FSEEKO_COMPILE_OK)
                # glibc 2.2 needs _LARGEFILE_SOURCE for fseeko (but not for 64-bit off_t...)
                TRY_COMPILE(FSEEKO_COMPILE_OK
                            "${PROJECT_BINARY_DIR}"
                            "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestLargeFiles.c"
                            COMPILE_DEFINITIONS "-D_LARGEFILE_SOURCE" )
                
                IF(FSEEKO_COMPILE_OK)
                    MESSAGE(STATUS "Checking for fseeko/ftello - present with _LARGEFILE_SOURCE")
                    SET(_LARGEFILE_SOURCE ${_LARGEFILE_SOURCE} CACHE INTERNAL "Result of test for needed _LARGEFILE_SOURCE")
                ENDIF(FSEEKO_COMPILE_OK)
        ENDIF(NOT FSEEKO_COMPILE_OK)

	    if(FSEEKO_COMPILE_OK)
                SET(HAVE_FSEEKO ON CACHE INTERNAL "Result of test for fseeko/ftello")
        else(FSEEKO_COMPILE_OK)
                MESSAGE(STATUS "Checking for fseeko/ftello - not found")
                SET(HAVE_FSEEKO OFF CACHE INTERNAL "Result of test for fseeko/ftello")
        endif(FSEEKO_COMPILE_OK)

	    if(FILE64_OK AND FSEEKO_COMPILE_OK)
                MESSAGE(STATUS "Large File support - found")
                SET(${VARIABLE} ON CACHE INTERNAL "Result of test for large file support")
        else(FILE64_OK AND FSEEKO_COMPILE_OK)
                MESSAGE(STATUS "Large File support - not found")
                SET(${VARIABLE} OFF CACHE INTERNAL "Result of test for large file support")
        endif(FILE64_OK AND FSEEKO_COMPILE_OK)

    ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(OPJ_TEST_LARGE_FILES VARIABLE)



