#  Copyright (c) 2014 Mathieu Malaterre <mathieu.malaterre@voxxl.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# check md5 refs
#
# This script will be used to make sure we never introduce a regression on any
# of the nonregression file.
#
# The approach is relatively simple, we compute a md5sum for each of the decode
# file. Anytime the md5sum is different from the reference one, we assume
# something went wrong and simply fails.  of course if could happen during the
# course of openjpeg development that the internals are changed that impact the
# decoding process that the output would be bitwise different (while PSNR would
# be kept identical).

#message("0: ${REFFILE}")
#message("1: ${CMAKE_CURRENT_BINARY_DIR}")
#message("2: ${FILENAME}")
file(GLOB globfiles "Temporary/${FILENAME}*.pgx" "Temporary/${FILENAME}*.png")
#message("6: ${globfiles}")
if(NOT globfiles)
  message(SEND_ERROR "Could not find output PGX files: ${FILENAME}")
endif()

# REFFILE follow what md5sum -c would expect as input:
file(READ ${REFFILE} variable)
#string(REGEX REPLACE "\r?\n" ";" variable "${variable}")

foreach(pgxfullpath ${globfiles})
  get_filename_component(pgxfile ${pgxfullpath} NAME)
  #message("8: ${pgxfile}")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E md5sum ${pgxfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Temporary
    RESULT_VARIABLE res
    OUTPUT_VARIABLE output
    ERROR_VARIABLE  error_output
    OUTPUT_STRIP_TRAILING_WHITESPACE # important
  )
  
  # Pass the output back to ctest
  if(res)
    message(SEND_ERROR "md5 could not be computed, it failed with value ${res}. Output was: ${error_output}")
  endif()
  #message("3: ${output}")
  
  #message("4: ${variable}")
  string(REGEX MATCH "[0-9a-f]+  ${pgxfile}" output_var "${variable}")
  #message("5: ${output_var}")
  
  if("${output_var}" STREQUAL "${output}")
    #message("6: eqal")
  else()
    message(SEND_ERROR "not equal: [${output_var}] vs [${output}]")
  endif()
endforeach()
