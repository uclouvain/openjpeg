# NON-REGRESSION TESTS ON THIS DATASET LOCATED ${OPJ_DATA_ROOT}/input/nonregression

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Temporary)

set(TEMP ${CMAKE_CURRENT_BINARY_DIR}/Temporary)
set(BASELINE_NR ${OPJ_DATA_ROOT}/baseline/nonregression)
set(INPUT_NR ${OPJ_DATA_ROOT}/input/nonregression)


set(INPUT_NR_PATH ${INPUT_NR})
set(TEMP_PATH ${TEMP})
set(INPUT_CONF_PATH ${OPJ_DATA_ROOT}/input/conformance)

# need kdu_expand if possible
find_package(KAKADU)

#########################################################################
# GENERATION OF THE TEST SUITE (DUMP)
# Dump all files with the selected extension inside the input directory

# Define a list of file which should be gracefully rejected:
set(BLACKLIST_JPEG2000
    broken.jp2 
    broken2.jp2
    broken3.jp2
    broken4.jp2
    gdal_fuzzer_assert_in_opj_j2k_read_SQcd_SQcc.patch.jp2	
    gdal_fuzzer_check_comp_dx_dy.jp2
    gdal_fuzzer_check_number_of_tiles.jp2
    gdal_fuzzer_unchecked_numresolutions.jp2
   )

file(GLOB_RECURSE OPJ_DATA_NR_LIST
     "${INPUT_NR}/*.j2k"
     "${INPUT_NR}/*.j2c"
     "${INPUT_NR}/*.jp2"
     #"${INPUT_NR}/*.jpx"
     )

foreach(INPUT_FILENAME ${OPJ_DATA_NR_LIST})
  get_filename_component(INPUT_FILENAME_NAME ${INPUT_FILENAME} NAME)
  get_filename_component(INPUT_FILENAME_NAME_WE ${INPUT_FILENAME_NAME} NAME_WE)
  string(REGEX MATCH ${INPUT_FILENAME_NAME} bad_jpeg2000 ${BLACKLIST_JPEG2000})

  # Dump the input image
  add_test(NR-${INPUT_FILENAME_NAME}-dump
           ${EXECUTABLE_OUTPUT_PATH}/opj_dump
  	       -i ${INPUT_FILENAME}
  	       -o ${TEMP}/${INPUT_FILENAME_NAME}.txt
  	       -v
  	       )
  	
  if(bad_jpeg2000)
    set_tests_properties(NR-${INPUT_FILENAME_NAME}-dump
                         PROPERTIES WILL_FAIL TRUE)

  else()
  	
    # Compare the dump output with the baseline
    add_test(NR-${INPUT_FILENAME_NAME}-compare_dump2base
  	        ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
            -b ${BASELINE_NR}/opj_v2_${INPUT_FILENAME_NAME_WE}.txt
            -t ${TEMP}/${INPUT_FILENAME_NAME}.txt
            )

    set_tests_properties(NR-${INPUT_FILENAME_NAME}-compare_dump2base
                         PROPERTIES DEPENDS
                         NR-${INPUT_FILENAME_NAME}-dump)
  endif()

endforeach()


#########################################################################
# GENERATION OF THE TEST SUITE (DECODE AND ENCODE)
# Read one and more input file(s) (located in ${OPJ_DATA_ROOT}/input/nonregression)
# to know which files processed and with which options.

# Configure the test suite file:
configure_file("test_suite.ctest.in"
               "${CMAKE_CURRENT_BINARY_DIR}/test_suite.ctest"
               @ONLY)

# Read the file into a list
file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/test_suite.ctest OPJ_TEST_CMD_LINE_LIST)

# Try to find and configure and read another test suite file
file(GLOB TEST_SUITE_FILES *.ctest.in)
if (TEST_SUITE_FILES)
  foreach(TEST_SUITE_FILE ${TEST_SUITE_FILES})

    # Avoid to process the official test suite
    set(FILE_ALREADY_READ 0)
    get_filename_component(TEST_SUITE_FILENAME ${TEST_SUITE_FILE} NAME)
    string(REGEX MATCH "^test_suite.ctest.in$" FILE_ALREADY_READ ${TEST_SUITE_FILENAME})

    if(NOT FILE_ALREADY_READ)
      # Configure the additional test suite file:
      get_filename_component(TEST_SUITE_FILE_SUB ${TEST_SUITE_FILE} NAME_WE)
      configure_file("${TEST_SUITE_FILE}"
                     "${CMAKE_CURRENT_BINARY_DIR}/${TEST_SUITE_FILE_SUB}.ctest"
                     @ONLY)
      # Read the additional file into a list
      file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SUITE_FILE_SUB}.ctest OPJ_TEST_CMD_LINE_LIST_TEMP)
      # Append the list of command
      set(OPJ_TEST_CMD_LINE_LIST ${OPJ_TEST_CMD_LINE_LIST} ${OPJ_TEST_CMD_LINE_LIST_TEMP})

    endif()

  endforeach()

else()

  message(FATAL_ERROR "One test suite should be available (test_suite.ctest.in) !!!")

endif()


# Parse the command line found in the file(s)
set(IT_TEST_ENC 0)
set(IT_TEST_DEC 0)
foreach(OPJ_TEST_CMD_LINE ${OPJ_TEST_CMD_LINE_LIST})

  set(IGNORE_LINE_FOUND 0)

  # Replace space by ; to generate a list
  string(REPLACE " " ";" CMD_ARG_LIST ${OPJ_TEST_CMD_LINE})

  # Check if the first argument begin by the comment sign
  list(GET CMD_ARG_LIST 0 EXE_NAME)

  if(EXE_NAME)
    string(REGEX MATCH "^#" IGNORE_LINE_FOUND ${EXE_NAME})
  endif()

  if(IGNORE_LINE_FOUND OR NOT EXE_NAME)

    #message( STATUS "Current line is ignored: ${OPJ_TEST_CMD_LINE}")

  else()

    # Check if the first argument begin by the failed sign
    set(FAILED_TEST_FOUND 0)
    string(REGEX MATCH "^!" FAILED_TEST_FOUND ${EXE_NAME})

    if (FAILED_TEST_FOUND)
      # Manage the different cases with the failed sign to remove the first argument which must be opj_compress
      set(FAILED_TEST_FOUND_1 0)
      string(REGEX MATCH "^!opj_compress$|^!opj_decompress$" FAILED_TEST_FOUND_1 ${EXE_NAME})

      if (FAILED_TEST_FOUND_1)

        list(REMOVE_AT CMD_ARG_LIST 0)

      else ()

        set(FAILED_TEST_FOUND_2 0)
        list(GET CMD_ARG_LIST 1 EXE_NAME)
        string(REGEX MATCH "^opj_compress$|^opj_decompress$" FAILED_TEST_FOUND_2 ${EXE_NAME})

        if (FAILED_TEST_FOUND_2)

          list(REMOVE_AT CMD_ARG_LIST 0)
          list(REMOVE_AT CMD_ARG_LIST 0)

        else ()

          message( FATAL_ERROR "${EXE_NAME} is not the right executable name to encode file (try to use opj_compress or opj_decompress)")

        endif ()
      endif ()

    else ()
      # Check if the first argument is equal to opj_compress
      string(REGEX MATCH "^opj_compress$|^opj_decompress$" EXE_NAME_FOUND ${EXE_NAME})

      if(EXE_NAME_FOUND)

        string(REGEX MATCH "opj_compress" ENC_TEST_FOUND ${EXE_NAME})

      else()

        message( FATAL_ERROR "${EXE_NAME} is not the right executable name to encode file (try to use opj_compress)")

      endif()

      list(REMOVE_AT CMD_ARG_LIST 0)

    endif ()

    # Parse the argument list to find the input filename and output filename
    set(CMD_ARG_LIST_2 "")
    set(ARG_POS 0)
    set(INPUT_ARG_POS 0)
    set(OUTPUT_ARG_POS 0)

    foreach(CMD_ARG_ELT ${CMD_ARG_LIST})

      math(EXPR ARG_POS "${ARG_POS}+1" )

      string(COMPARE EQUAL ${CMD_ARG_ELT} "-i" INPUT_ARG_FOUND)
      if(INPUT_ARG_FOUND)
        set(INPUT_ARG_POS ${ARG_POS})
        set(INPUT_ARG_FOUND 0)
      endif()

      string(COMPARE EQUAL ${CMD_ARG_ELT} "-o" OUTPUT_ARG_FOUND)
      if(OUTPUT_ARG_FOUND)
        set(OUTPUT_ARG_POS ${ARG_POS})
        set(OUTPUT_ARG_FOUND 0)
      endif()

      list (APPEND CMD_ARG_LIST_2 ${CMD_ARG_ELT})

    endforeach()

    list(GET CMD_ARG_LIST_2 ${INPUT_ARG_POS} INPUT_FILENAME)
    get_filename_component(INPUT_FILENAME_NAME ${INPUT_FILENAME} NAME)
    get_filename_component(INPUT_FILENAME_NAME_WE ${INPUT_FILENAME_NAME} NAME_WE)
    list(GET CMD_ARG_LIST_2 ${OUTPUT_ARG_POS} OUTPUT_FILENAME)
    get_filename_component(OUTPUT_FILENAME_NAME_WE ${OUTPUT_FILENAME} NAME_WE)

    #-----
    # Now we can add the test suite corresponding to a line command in the file
    #-----

    # ENCODER TEST SUITE
    if(ENC_TEST_FOUND)
        math(EXPR IT_TEST_ENC "${IT_TEST_ENC}+1" )

        # Encode an image into the jpeg2000 format
        add_test(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode
          ${EXECUTABLE_OUTPUT_PATH}/opj_compress
          ${CMD_ARG_LIST_2}
        )

        if(FAILED_TEST_FOUND)
            set_tests_properties(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode PROPERTIES WILL_FAIL TRUE)
        else()

          # Dump the encoding file
          add_test(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-dump
            ${EXECUTABLE_OUTPUT_PATH}/opj_dump
            -i ${OUTPUT_FILENAME}
            -o ${OUTPUT_FILENAME}-ENC-${IT_TEST_ENC}.txt
          )
          set_tests_properties(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-dump
                               PROPERTIES DEPENDS
                               NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode)

          # Compare the dump file with the baseline
          add_test(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dump2base
            ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
            -b ${BASELINE_NR}/opj_v2_${OUTPUT_FILENAME_NAME_WE}-ENC-${IT_TEST_ENC}.txt
            -t ${OUTPUT_FILENAME}-ENC-${IT_TEST_ENC}.txt
          )

          set_tests_properties(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dump2base
                               PROPERTIES DEPENDS
                               NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-dump)

          # Decode the encoding file with kakadu expand command
          if (KDU_EXPAND_EXECUTABLE)
            add_test(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-decode-ref
              ${KDU_EXPAND_EXECUTABLE}
              -i ${OUTPUT_FILENAME}
              -o ${OUTPUT_FILENAME}.raw
              )

            set_tests_properties(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-decode-ref
                                 PROPERTIES DEPENDS
                                 NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode)

            # Compare the decoding file with baseline generated from the kdu_expand and baseline.j2k
            add_test(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dec-ref-out2base
              ${EXECUTABLE_OUTPUT_PATH}/compareRAWimages
              -b ${BASELINE_NR}/opj_${OUTPUT_FILENAME_NAME_WE}-ENC-${IT_TEST_ENC}.raw
              -t ${OUTPUT_FILENAME}.raw
            )

            set_tests_properties(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dec-ref-out2base
                                 PROPERTIES DEPENDS
                                 NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-decode-ref)

          endif()
      endif()

    # DECODER TEST SUITE
    else()
      math(EXPR IT_TEST_DEC "${IT_TEST_DEC}+1" )

      # Decode the input image
      add_test(NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-decode
        ${EXECUTABLE_OUTPUT_PATH}/opj_decompress
        ${CMD_ARG_LIST_2}
      )

      if(FAILED_TEST_FOUND)

        set_tests_properties(NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-decode PROPERTIES WILL_FAIL TRUE)

      else()

# FIXME: add a compare2base function base on raw which
# can output png diff files if necesary
#  add_test(NR-${filename}-compare2base
#      ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
#     -b ${BASELINE_NR}/opj_${filenameRef}
#     -t ${TEMP}/${filename}.pgx
#     -n ${nbComponents}
#     -d
#     -s b_t_
#     )
#
#  set_tests_properties(NR-${filename}-compare2base
#                       PROPERTIES DEPENDS
#                       NR-${filename}-decode)

      endif()

    endif()

  endif()

endforeach()
