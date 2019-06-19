cmake_minimum_required(VERSION 3.9)

include_regular_expression("^.*$")

install( FILES  ${CMAKE_CURRENT_SOURCE_DIR}/opj_config.h
DESTINATION ${OPENJPEG_INSTALL_INCLUDE_DIR} COMPONENT Headers)

include_directories(
  ${${OPENJPEG_NAMESPACE}_SOURCE_DIR}/src/lib/openjp2 # opj_config.h and opj_config_private.h
)

# list the source files
set(SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/thread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/dwt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/event.c
    ${CMAKE_CURRENT_SOURCE_DIR}/image.c
    ${CMAKE_CURRENT_SOURCE_DIR}/invert.c
    ${CMAKE_CURRENT_SOURCE_DIR}/j2k.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jp2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mqc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_clock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/t1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/t2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tcd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tgt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/function_list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_malloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sparse_array.c
)

set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/thread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bio.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cio.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dwt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/event.h
    ${CMAKE_CURRENT_SOURCE_DIR}/image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/invert.h
    ${CMAKE_CURRENT_SOURCE_DIR}/j2k.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jp2.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mct.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mqc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mqc_inl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openjpeg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_clock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/t1.h
    ${CMAKE_CURRENT_SOURCE_DIR}/t2.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tcd.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tgt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/function_list.h
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_codec.h
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_includes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_intmath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_malloc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/opj_stdint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sparse_array.h
)

option(OPJ_DISABLE_TPSOT_FIX "Disable TPsot==TNsot fix. See https://github.com/uclouvain/openjpeg/issues/254." OFF)
if(OPJ_DISABLE_TPSOT_FIX)
    target_compile_definitions(${OPENJPEG_LIBRARY_NAME}
        PRIVATE
           -DOPJ_DISABLE_TPSOT_FIX 
    )
endif()

# Special case for old i586-mingw32msvc-gcc cross compiler
if(NOT WIN32 AND CMAKE_COMPILER_IS_GNUCC AND CMAKE_C_COMPILER MATCHES ".*mingw32msvc.*" )
  set(WIN32 YES)
endif()

# Build the library
if(WIN32)
  add_library(${OPENJPEG_LIBRARY_NAME} 
      	${HEADER_FILES}
  	    ${SOURCE_FILES}
    )
  if(BUILD_SHARED_LIBS)
    target_compile_definitions(${OPENJPEG_LIBRARY_NAME}
        PRIVATE
           -DOPJ_EXPORTS 
    )
  else()
    target_compile_definitions(${OPENJPEG_LIBRARY_NAME}
        PRIVATE
           -DOPJ_STATIC 
    )
  endif()
  set(INSTALL_LIBS ${OPENJPEG_LIBRARY_NAME})
else()
  if(BUILD_SHARED_LIBS AND BUILD_STATIC_LIBS)
    # Builds both static and dynamic libs
    add_library(${OPENJPEG_LIBRARY_NAME} SHARED	
        ${HEADER_FILES}
	      ${SOURCE_FILES}
    )
    add_library(openjp2_static STATIC 
        ${HEADER_FILES}
	      ${SOURCE_FILES}
    )
    set_target_properties(openjp2_static PROPERTIES OUTPUT_NAME ${OPENJPEG_LIBRARY_NAME})
    set(INSTALL_LIBS ${OPENJPEG_LIBRARY_NAME} openjp2_static)
  else()
    add_library(${OPENJPEG_LIBRARY_NAME} 
        ${HEADER_FILES}
	      ${SOURCE_FILES}
    )
    set(INSTALL_LIBS ${OPENJPEG_LIBRARY_NAME})
  endif()
endif()

target_include_directories(${OPENJPEG_LIBRARY_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

if(UNIX)
  target_link_libraries(${OPENJPEG_LIBRARY_NAME} m)
endif()
set_target_properties(${OPENJPEG_LIBRARY_NAME} PROPERTIES ${OPENJPEG_LIBRARY_PROPERTIES})
if(NOT ${CMAKE_VERSION} VERSION_LESS "2.8.12")
  target_compile_options(${OPENJPEG_LIBRARY_NAME} PRIVATE ${OPENJP2_COMPILE_OPTIONS})
endif()

if(BUILD_LUTS_GENERATOR)
# internal utility to generate t1_luts.h (part of the jp2 lib)
# no need to install:
  add_executable(t1_generate_luts t1_generate_luts.c)
  if(UNIX)
    target_link_libraries(t1_generate_luts m)
  endif()
endif()

# Experimental option; let's how cppcheck performs
# Implementation details:
# I could not figure out how to easily upload a file to CDash. Instead simply
# pretend cppcheck is part of the Build step. Technically cppcheck can even
# output gcc formatted error/warning report
# Another implementation detail: I could not redirect error to the error
# catching mechanism something is busted in cmake 2.8.5, I had to use the
# warning regex to catch them.
if(OPENJPEG_CPPCHECK)
  find_package(CPPCHECK REQUIRED)
  foreach(f ${HEADER_FILES})
    # cppcheck complains about too many configuration, pretend to be WIN32:
    add_custom_command(TARGET ${OPENJPEG_LIBRARY_NAME}
      COMMAND ${CPPCHECK_EXECUTABLE} -DWIN32 ${f})
  endforeach()

  foreach(f ${SOURCE_FILES})
    # cppcheck complains about too many configuration, pretend to be WIN32:
    add_custom_command(TARGET ${OPENJPEG_LIBRARY_NAME}
      COMMAND ${CPPCHECK_EXECUTABLE} -DWIN32 ${f})
  endforeach()
endif()

if(OPJ_USE_DSYMUTIL)
  if(BUILD_SHARED_LIBS)
    add_custom_command(TARGET ${OPENJPEG_LIBRARY_NAME} POST_BUILD
    COMMAND "dsymutil" "$<TARGET_SONAME_FILE:${OPENJPEG_LIBRARY_NAME}>"
    COMMENT "dsymutil $<TARGET_SONAME_FILE:${OPENJPEG_LIBRARY_NAME}>"
    DEPENDS ${OPENJPEG_LIBRARY_NAME})
  endif()
endif()

#################################################################################
# threading configuration
#################################################################################
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)

option(OPJ_USE_THREAD "Build with thread/mutex support " ON)
if(NOT OPJ_USE_THREAD)
    target_compile_definitions(${OPENJPEG_LIBRARY_NAME}
        PRIVATE
           -DMUTEX_stub 
    )
endif(NOT OPJ_USE_THREAD)

find_package(Threads QUIET)

if(OPJ_USE_THREAD AND WIN32 AND NOT Threads_FOUND )
    target_compile_definitions(${OPENJPEG_LIBRARY_NAME}
        PRIVATE
           -DMUTEX_win32 
    )
    set(Threads_FOUND YES)
endif()

if(OPJ_USE_THREAD AND Threads_FOUND AND CMAKE_USE_WIN32_THREADS_INIT )
    target_compile_definitions(${OPENJPEG_LIBRARY_NAME}
        PRIVATE
           -DMUTEX_win32 
    )
endif(OPJ_USE_THREAD AND Threads_FOUND AND CMAKE_USE_WIN32_THREADS_INIT )

if(OPJ_USE_THREAD AND Threads_FOUND AND CMAKE_USE_PTHREADS_INIT )
    target_compile_definitions(${OPENJPEG_LIBRARY_NAME}
        PRIVATE
           -DMUTEX_pthread 
    )
endif(OPJ_USE_THREAD AND Threads_FOUND AND CMAKE_USE_PTHREADS_INIT )

if(OPJ_USE_THREAD AND NOT Threads_FOUND)
  message(FATAL_ERROR "No thread library found and thread/mutex support is required by OPJ_USE_THREAD option")
endif(OPJ_USE_THREAD AND NOT Threads_FOUND)

if(OPJ_USE_THREAD AND Threads_FOUND AND CMAKE_USE_PTHREADS_INIT)
   TARGET_LINK_LIBRARIES(${OPENJPEG_LIBRARY_NAME} ${CMAKE_THREAD_LIBS_INIT})
endif(OPJ_USE_THREAD AND Threads_FOUND AND CMAKE_USE_PTHREADS_INIT)
