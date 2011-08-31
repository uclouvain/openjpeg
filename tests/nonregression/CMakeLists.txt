# NON-REGRESSION TESTS ON THIS DATASET LOCATED ${OPJ_DATA_ROOT}/input/nonregression

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Temporary)

SET(TEMP ${CMAKE_CURRENT_BINARY_DIR}/Temporary)
SET(BASELINE_NR ${OPJ_DATA_ROOT}/baseline/nonregression)
SET(INPUT_NR ${OPJ_DATA_ROOT}/input/nonregression)

FILE(GLOB_RECURSE OPJ_DATA_NR_LIST
  "${INPUT_NR}/*.j2k"
  "${INPUT_NR}/*.j2c"
  "${INPUT_NR}/*.jp2"
  )

# Define a list of file which should be gracefully rejected:
SET(BLACKLIST_JPEG200
  illegalcolortransform.j2k
  )

FOREACH(filepath ${OPJ_DATA_NR_LIST})
  GET_FILENAME_COMPONENT(filename ${filepath} NAME)
  GET_FILENAME_COMPONENT(filenameSub ${filename} NAME_WE)
  STRING(REGEX MATCH ${filename} bad_jpeg2000 ${BLACKLIST_JPEG200})

  ADD_TEST(NR-${filename}-dump
    ${EXECUTABLE_OUTPUT_PATH}/j2k_dump
    -i ${filepath}
    -o ${TEMP}/${filename}.txt
    )
  ADD_TEST(NR-${filename}-decode
    ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
    -i ${filepath}
    -o ${TEMP}/${filename}.pgx
    )

  IF(bad_jpeg2000)
    SET_TESTS_PROPERTIES(NR-${filename}-dump
      PROPERTIES WILL_FAIL TRUE)
    SET_TESTS_PROPERTIES(NR-${filename}-decode
      PROPERTIES WILL_FAIL TRUE)
  ELSE(bad_jpeg2000)
    # Only run this test when previous succeeds
    ADD_TEST(NR-${filename}-compare_dump2base
      ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
      -b ${BASELINE_NR}/opj_${filenameSub}.txt
      -t ${TEMP}/${filename}.txt
      )

    SET_TESTS_PROPERTIES(NR-${filename}-compare_dump2base  
      PROPERTIES DEPENDS 
      NR-${filename}-dump)     

  ENDIF(bad_jpeg2000)
     
#  ADD_TEST(NR-${filename}-compare2base
#      ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
#     -b ${BASELINE_NR}/opj_${filenameRef}
#     -t ${TEMP}/${filename}.pgx
#     -n ${nbComponents}
#     -d 
#     -s b_t_
#     )
#
#  SET_TESTS_PROPERTIES(NR-${filename}-compare2base  
#                       PROPERTIES DEPENDS 
#                       NR-${filename}-decode)    
  
ENDFOREACH(filepath)


# TEST ENCODER SUITE
# Read a input file to know which files processed and with which options.

IF (NOT WIN32)
CONFIGURE_FILE("opj_ref_decode_cmd.sh.in"
               "${CMAKE_CURRENT_BINARY_DIR}/opj_ref_decode_cmd.sh"
               @ONLY)
ENDIF (NOT WIN32)
   
string(COMPARE EQUAL ${REF_DECODER_BIN_PATH} "NOTFOUND" REF_DECODER_NOTFOUND)            
IF (REF_DECODER_NOTFOUND)
  MESSAGE(STATUS "REF_DECODER_BIN_PATH not found, if you want all the encoding tests suite please provide a ragular path")  
ENDIF (REF_DECODER_NOTFOUND)


# Read the file into a list
FILE(STRINGS encoder_test_suite.txt OPJ_TEST_ENC_CMD_LINE_LIST)

SET(it_test 0)

FOREACH(OPJ_TEST_ENC_CMD_LINE ${OPJ_TEST_ENC_CMD_LINE_LIST})
  math(EXPR it_test "${it_test}+1" )

  #message(STATUS ${OPJ_TEST_ENC_CMD_LINE})

  string(REPLACE " " ";" CMD_ARG_LIST ${OPJ_TEST_ENC_CMD_LINE})
  
  #message(STATUS "list = ${CMD_ARG_LIST}")
  
  list(GET CMD_ARG_LIST 0 EXE_NAME)
  string(COMPARE EQUAL ${EXE_NAME} "image_to_j2k" EXE_NAME_FOUND)
  IF(EXE_NAME_FOUND)
    list(REMOVE_AT CMD_ARG_LIST 0)
  ELSE(EXE_NAME_FOUND)
    string(COMPARE EQUAL ${EXE_NAME} "#" IGNORE_LINE_FOUND)
    IF(IGNORE_LINE_FOUND)
      message( STATUS "Current line is ignored: ${OPJ_TEST_ENC_CMD_LINE}")
    ELSE(IGNORE_LINE_FOUND)
      message( FATAL_ERROR "${EXE_ARG} is not the right executable name to encode file (try to use image_to_j2k)")
    ENDIF(IGNORE_LINE_FOUND)   
  ENDIF(EXE_NAME_FOUND)
  
  IF(NOT IGNORE_LINE_FOUND)
    SET(CMD_ARG_LIST_2 "")
    SET(ARG_POS 0)
    SET(INPUT_ARG_POS 0)
    SET(OUTPUT_ARG_POS 0)
    
    FOREACH(CMD_ARG_ELT ${CMD_ARG_LIST})
      math(EXPR ARG_POS "${ARG_POS}+1" )
      
      # Will work only with cmake version > 2.8.4
      #message(STATUS "${CMD_ARG_ELT}")
      #string(FIND "${CMD_ARG_ELT}" "-i input/" INPUT_ARG_FOUND)
      #message(STATUS "pos = ${INPUT_ARG_FOUND}")
      #IF(INPUT_ARG_FOUND)
      #  string(SUBSTRING ${CMD_ARG_ELT} INPUT_ARG_FOUND -1 filename_input)
      #  message(STATUS "filename = ${filename_input}")
      #ELSE(INPUT_ARG_FOUND)
      #  message( FATAL_ERROR "Not found a input image into the list of arguments")
      #ENDIF(INPUT_ARG_FOUND)
    

      string(COMPARE EQUAL ${CMD_ARG_ELT} "-i" INPUT_ARG_FOUND)
      IF(INPUT_ARG_FOUND)
        SET(INPUT_ARG_POS ${ARG_POS})
        SET(INPUT_ARG_FOUND 0)
      ENDIF(INPUT_ARG_FOUND)
      
      string(COMPARE EQUAL ${CMD_ARG_ELT} "-o" OUTPUT_ARG_FOUND)
      IF(OUTPUT_ARG_FOUND)
        SET(OUTPUT_ARG_POS ${ARG_POS})
        SET(OUTPUT_ARG_FOUND 0)
      ENDIF(OUTPUT_ARG_FOUND)
    
      string(REPLACE "input/" "${INPUT_NR}/" CMD_ARG_ELT ${CMD_ARG_ELT})
      string(REPLACE "temp/" "${TEMP}/" CMD_ARG_ELT ${CMD_ARG_ELT})
    
      list (APPEND CMD_ARG_LIST_2 ${CMD_ARG_ELT})
  
    ENDFOREACH(CMD_ARG_ELT)
  
    list(GET CMD_ARG_LIST_2 ${INPUT_ARG_POS} INPUT_FILENAME)
    get_filename_component(INPUT_FILENAME_NAME ${INPUT_FILENAME} NAME)
    list(GET CMD_ARG_LIST_2 ${OUTPUT_ARG_POS} OUTPUT_FILENAME)
    get_filename_component(OUTPUT_FILENAME_NAME_WE ${OUTPUT_FILENAME} NAME_WE)
    
      
    # Encode an image into the jpeg2000 format
    ADD_TEST(NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-encode
      ${EXECUTABLE_OUTPUT_PATH}/${EXE_NAME}
      ${CMD_ARG_LIST_2}
    )
      
    # Dump the encoding file 
    ADD_TEST(NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-dump
      ${EXECUTABLE_OUTPUT_PATH}/j2k_dump
      -i ${OUTPUT_FILENAME}
      -o ${OUTPUT_FILENAME}-ENC-${it_test}.txt
    )
    SET_TESTS_PROPERTIES(NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-dump
                         PROPERTIES DEPENDS 
                         NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-encode) 
    
    # Compare the dump file with the baseline 
    ADD_TEST(NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-compare_dump2base
      ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
      -b ${BASELINE_NR}/opj_${OUTPUT_FILENAME_NAME_WE}-ENC-${it_test}.txt
      -t ${OUTPUT_FILENAME}-ENC-${it_test}.txt
    )
    
    SET_TESTS_PROPERTIES(NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-compare_dump2base
                         PROPERTIES DEPENDS 
                         NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-dump) 

    # Decode the encoding file with kakadu expand command
    IF (NOT REF_DECODER_NOTFOUND)
      IF (NOT WIN32)
        # Need a bash script to export the path of the kakadu library into PATH  
        ADD_TEST( NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-decode-ref
           bash ${CMAKE_CURRENT_BINARY_DIR}/opj_ref_decode_cmd.sh
          -i ${OUTPUT_FILENAME}
          -o ${OUTPUT_FILENAME}.raw
        )
      ELSE (NOT WIN32)
        ADD_TEST(NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-decode-ref
          ${REF_DECODER_BIN_PATH}/kdu_expand
          -i ${OUTPUT_FILENAME}
          -o ${OUTPUT_FILENAME}.raw
        )
      ENDIF (NOT WIN32)
    
      SET_TESTS_PROPERTIES(NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-decode-ref
                           PROPERTIES DEPENDS 
                           NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-encode) 
    
      # Compare the decoding file with baseline generated from the kdu_expand and baseline.j2k 
      ADD_TEST(NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-compare_dec-ref-out2base
        ${EXECUTABLE_OUTPUT_PATH}/compareRAWimages
        #-b ${BASELINE_NR}/opj_${OUTPUT_FILENAME_NAME_WE}-ENC-${it_test}.raw
        -b ${BASELINE_NR}/opj_${OUTPUT_FILENAME_NAME_WE}-ENC-${it_test}.raw
        -t ${OUTPUT_FILENAME}.raw
      )
    
      SET_TESTS_PROPERTIES(NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-compare_dec-ref-out2base
                           PROPERTIES DEPENDS 
                           NR-ENC-${INPUT_FILENAME_NAME}-${it_test}-decode-ref)
    
    ENDIF (NOT REF_DECODER_NOTFOUND)
    
  ENDIF(NOT IGNORE_LINE_FOUND)

ENDFOREACH(OPJ_TEST_ENC_CMD_LINE)




