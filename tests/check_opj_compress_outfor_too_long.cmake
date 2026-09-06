execute_process(
  COMMAND "${OPJ_COMPRESS}" -OutFor "${OUTFOR_ARG}"
  RESULT_VARIABLE opj_result
  OUTPUT_VARIABLE opj_stdout
  ERROR_VARIABLE opj_stderr)

if(opj_result EQUAL 0)
  message(FATAL_ERROR "opj_compress unexpectedly accepted oversized -OutFor")
endif()

set(opj_output "${opj_stdout}${opj_stderr}")
if(NOT opj_output MATCHES "\\[ERROR\\] -OutFor argument is too long")
  message(FATAL_ERROR "missing bounded-length error. Output was:\n${opj_output}")
endif()
