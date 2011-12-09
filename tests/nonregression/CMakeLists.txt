# NON-REGRESSION TESTS ON THIS DATASET LOCATED ${OPJ_DATA_ROOT}/input/nonregression

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Temporary)

SET(TEMP ${CMAKE_CURRENT_BINARY_DIR}/Temporary)
SET(BASELINE_NR ${OPJ_DATA_ROOT}/baseline/nonregression)
SET(INPUT_NR ${OPJ_DATA_ROOT}/input/nonregression)

SET(INPUT_NR_PATH ${INPUT_NR})
SET(TEMP_PATH ${TEMP})

# need kdu_expand if possible
find_package(KAKADU)

#########################################################################
# GENERATION OF THE TEST SUITE
# Read one and more input file(s) (located in ${OPJ_DATA_ROOT}/input/nonregression) 
# to know which files processed and with which options.

# Configure the test suite file:
CONFIGURE_FILE("test_suite.ctest.in"
               "${CMAKE_CURRENT_BINARY_DIR}/test_suite.ctest"
               @ONLY)

# Read the file into a list
FILE(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/test_suite.ctest OPJ_TEST_CMD_LINE_LIST)

# Try to find and configure and read another test suite file
FILE(GLOB TEST_SUITE_FILES *.ctest.in)
IF (TEST_SUITE_FILES)
  FOREACH(TEST_SUITE_FILE ${TEST_SUITE_FILES})
    
    # Avoid to process the official test suite
    SET(FILE_ALREADY_READ 0)
    STRING(REGEX MATCH "test_suite.ctest.in$" FILE_ALREADY_READ ${TEST_SUITE_FILE})
   
    IF(NOT FILE_ALREADY_READ) 
      # Configure the additional test suite file:
      GET_FILENAME_COMPONENT(TEST_SUITE_FILE_SUB ${TEST_SUITE_FILE} NAME_WE)
      CONFIGURE_FILE("${TEST_SUITE_FILE}"
                     "${CMAKE_CURRENT_BINARY_DIR}/${TEST_SUITE_FILE_SUB}.ctest"
                     @ONLY)
      # Read the additional file into a list
      FILE(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SUITE_FILE_SUB}.ctest OPJ_TEST_CMD_LINE_LIST_TEMP)
      # Append the list of command
      SET(OPJ_TEST_CMD_LINE_LIST ${OPJ_TEST_CMD_LINE_LIST} ${OPJ_TEST_CMD_LINE_LIST_TEMP}) 
    
    ENDIF(NOT FILE_ALREADY_READ) 

  ENDFOREACH(TEST_SUITE_FILE)
  
ELSE(TEST_SUITE_FILES)

  MESSAGE(FATAL_ERROR "One test suite should be available (test_suite.ctest.in) !!!")
  
ENDIF(TEST_SUITE_FILES)


# Parse the command line found in the file(s)
SET(IT_TEST_ENC 0)
SET(IT_TEST_DEC 0)
FOREACH(OPJ_TEST_CMD_LINE ${OPJ_TEST_CMD_LINE_LIST})
  
  SET(IGNORE_LINE_FOUND 0)

  # Replace space by ; to generate a list
  STRING(REPLACE " " ";" CMD_ARG_LIST ${OPJ_TEST_CMD_LINE})
  
  # Check if the first argument begin by the comment sign
  LIST(GET CMD_ARG_LIST 0 EXE_NAME)
  STRING(REGEX MATCH "^#" IGNORE_LINE_FOUND ${EXE_NAME})
 
  IF(IGNORE_LINE_FOUND) 
    
    #MESSAGE( STATUS "Current line is ignored: ${OPJ_TEST_CMD_LINE}")
   
  ELSE(IGNORE_LINE_FOUND)
  
    # Check if the first argument begin by the failed sign
    SET(FAILED_TEST_FOUND 0)
    STRING(REGEX MATCH "^!" FAILED_TEST_FOUND ${EXE_NAME})
   
    IF (FAILED_TEST_FOUND)
      # Manage the different cases with the failed sign to remove the first argument which must be image_to_j2k
      SET(FAILED_TEST_FOUND_1 0)
      STRING(REGEX MATCH "^!image_to_j2k$|^!j2k_to_image$" FAILED_TEST_FOUND_1 ${EXE_NAME})
      
      IF (FAILED_TEST_FOUND_1)
        
        LIST(REMOVE_AT CMD_ARG_LIST 0)
      
      ELSE (FAILED_TEST_FOUND_1)
        
        SET(FAILED_TEST_FOUND_2 0)
        LIST(GET CMD_ARG_LIST 1 EXE_NAME)
        STRING(REGEX MATCH "^image_to_j2k$|^j2k_to_image$" FAILED_TEST_FOUND_2 ${EXE_NAME})
      
        IF (FAILED_TEST_FOUND_2)
        
          LIST(REMOVE_AT CMD_ARG_LIST 0)
          LIST(REMOVE_AT CMD_ARG_LIST 0)
      
        ELSE (FAILED_TEST_FOUND_2)
        
          MESSAGE( FATAL_ERROR "${EXE_NAME} is not the right executable name to encode file (try to use image_to_j2k or j2k_to_image)")
          
        ENDIF (FAILED_TEST_FOUND_2)
      ENDIF (FAILED_TEST_FOUND_1)
      
    ELSE (FAILED_TEST_FOUND)
      # Check if the first argument is equal to image_to_j2k
      STRING(REGEX MATCH "^image_to_j2k$|^j2k_to_image$" EXE_NAME_FOUND ${EXE_NAME})
      
      IF(EXE_NAME_FOUND)
      
        STRING(REGEX MATCH "image_to_j2k" ENC_TEST_FOUND ${EXE_NAME})
     
      ELSE(EXE_NAME_FOUND)
        
        MESSAGE( FATAL_ERROR "${EXE_NAME} is not the right executable name to encode file (try to use image_to_j2k)")
        
      ENDIF(EXE_NAME_FOUND)
      
      LIST(REMOVE_AT CMD_ARG_LIST 0)
      
    ENDIF (FAILED_TEST_FOUND)
  
    # Parse the argument list to find the input filename and output filename
    SET(CMD_ARG_LIST_2 "")
    SET(ARG_POS 0)
    SET(INPUT_ARG_POS 0)
    SET(OUTPUT_ARG_POS 0)
        
    FOREACH(CMD_ARG_ELT ${CMD_ARG_LIST})
      
      math(EXPR ARG_POS "${ARG_POS}+1" ) 

      STRING(COMPARE EQUAL ${CMD_ARG_ELT} "-i" INPUT_ARG_FOUND)
      IF(INPUT_ARG_FOUND)
        SET(INPUT_ARG_POS ${ARG_POS})
        SET(INPUT_ARG_FOUND 0)
      ENDIF(INPUT_ARG_FOUND)
      
      STRING(COMPARE EQUAL ${CMD_ARG_ELT} "-o" OUTPUT_ARG_FOUND)
      IF(OUTPUT_ARG_FOUND)
        SET(OUTPUT_ARG_POS ${ARG_POS})
        SET(OUTPUT_ARG_FOUND 0)
      ENDIF(OUTPUT_ARG_FOUND)
      
      LIST (APPEND CMD_ARG_LIST_2 ${CMD_ARG_ELT})
  
    ENDFOREACH(CMD_ARG_ELT)
  
    LIST(GET CMD_ARG_LIST_2 ${INPUT_ARG_POS} INPUT_FILENAME)
    GET_FILENAME_COMPONENT(INPUT_FILENAME_NAME ${INPUT_FILENAME} NAME)
    GET_FILENAME_COMPONENT(INPUT_FILENAME_NAME_WE ${INPUT_FILENAME_NAME} NAME_WE)
    LIST(GET CMD_ARG_LIST_2 ${OUTPUT_ARG_POS} OUTPUT_FILENAME)
    GET_FILENAME_COMPONENT(OUTPUT_FILENAME_NAME_WE ${OUTPUT_FILENAME} NAME_WE)
    
    #-----
    # Now we can add the test suite corresponding to a line command in the file
    #-----
    
    # ENCODER TEST SUITE
    IF(ENC_TEST_FOUND)
        math(EXPR IT_TEST_ENC "${IT_TEST_ENC}+1" )
        
        # Encode an image into the jpeg2000 format
        ADD_TEST(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode
          ${EXECUTABLE_OUTPUT_PATH}/image_to_j2k
          ${CMD_ARG_LIST_2}
        )
        
        IF(FAILED_TEST_FOUND)
            SET_TESTS_PROPERTIES(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode PROPERTIES WILL_FAIL TRUE)
        ELSE(FAILED_TEST_FOUND)
          
          # Dump the encoding file 
          ADD_TEST(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-dump
            ${EXECUTABLE_OUTPUT_PATH}/j2k_dump
            -i ${OUTPUT_FILENAME}
            -o ${OUTPUT_FILENAME}-ENC-${IT_TEST_ENC}.txt
          )
          SET_TESTS_PROPERTIES(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-dump
                               PROPERTIES DEPENDS 
                               NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode) 
        
          # Compare the dump file with the baseline 
          ADD_TEST(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dump2base
            ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
            -b ${BASELINE_NR}/opj_${OUTPUT_FILENAME_NAME_WE}-ENC-${IT_TEST_ENC}.txt
            -t ${OUTPUT_FILENAME}-ENC-${IT_TEST_ENC}.txt
          )
        
          SET_TESTS_PROPERTIES(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dump2base
                               PROPERTIES DEPENDS 
                               NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-dump) 
    
          # Decode the encoding file with kakadu expand command
          IF (KDU_EXPAND_EXECUTABLE)
            ADD_TEST(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-decode-ref
              ${KDU_EXPAND_EXECUTABLE}
              -i ${OUTPUT_FILENAME}
              -o ${OUTPUT_FILENAME}.raw
              )
        
            SET_TESTS_PROPERTIES(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-decode-ref
                                 PROPERTIES DEPENDS 
                                 NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode) 
        
            # Compare the decoding file with baseline generated from the kdu_expand and baseline.j2k 
            ADD_TEST(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dec-ref-out2base
              ${EXECUTABLE_OUTPUT_PATH}/compareRAWimages
              -b ${BASELINE_NR}/opj_${OUTPUT_FILENAME_NAME_WE}-ENC-${IT_TEST_ENC}.raw
              -t ${OUTPUT_FILENAME}.raw
            )
        
            SET_TESTS_PROPERTIES(NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dec-ref-out2base
                                 PROPERTIES DEPENDS 
                                 NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-decode-ref)
        
          ENDIF()
      ENDIF(FAILED_TEST_FOUND)     
    
    # DECODER TEST SUITE
    ELSE(ENC_TEST_FOUND)
      math(EXPR IT_TEST_DEC "${IT_TEST_DEC}+1" )
      
      # Decode the input image
      ADD_TEST(NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-decode
        ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
        ${CMD_ARG_LIST_2}
      )
      
      IF(FAILED_TEST_FOUND)
      
        SET_TESTS_PROPERTIES(NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-decode PROPERTIES WILL_FAIL TRUE)
     
      ELSE(FAILED_TEST_FOUND)
  
# FIXME: add a compare2base function base on raw which 
# can output png diff files if necesary 
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
    
        # Dump the input image
        ADD_TEST(NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-dump
          ${EXECUTABLE_OUTPUT_PATH}/j2k_dump
          -i ${INPUT_FILENAME}
          -o ${TEMP}/${INPUT_FILENAME_NAME}.txt
        )
     
        # Compare the dump output with the baseline
        ADD_TEST(NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-compare_dump2base
          ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
         -b ${BASELINE_NR}/opj_${INPUT_FILENAME_NAME_WE}.txt
         -t ${TEMP}/${INPUT_FILENAME_NAME}.txt
        )
     
        SET_TESTS_PROPERTIES(NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-compare_dump2base  
                             PROPERTIES DEPENDS 
                             NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-dump)     
  

      ENDIF(FAILED_TEST_FOUND)    
  
    ENDIF(ENC_TEST_FOUND)
   
  ENDIF(IGNORE_LINE_FOUND)

ENDFOREACH(OPJ_TEST_CMD_LINE)

