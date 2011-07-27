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

FOREACH(filepath ${OPJ_DATA_NR_LIST})
  GET_FILENAME_COMPONENT(filename ${filepath} NAME)
  GET_FILENAME_COMPONENT(filenameSub ${filename} NAME_WE)
  message("${filenameSub}")
  
  ADD_TEST(NR-${filename}-dump
      ${EXECUTABLE_OUTPUT_PATH}/j2k_dump
     -i ${filepath}
     -o ${TEMP}/${filename}.txt
     )
     
  ADD_TEST(NR-${filename}-compare_dump2base
      ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
     -b ${BASELINE_NR}/opj_${filenameSub}.txt
     -t ${TEMP}/${filename}.txt
     )
     
  SET_TESTS_PROPERTIES(NR-${filename}-compare_dump2base  
                       PROPERTIES DEPENDS 
                       NR-${filename}-dump)     
   
  ADD_TEST(NR-${filename}-decode
      ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
      -i ${filepath}
      -o ${TEMP}/${filename}.pgx
      )
      
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