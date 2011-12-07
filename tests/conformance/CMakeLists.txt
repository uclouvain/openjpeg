# CONFORMANCE TESTS AND NON-REGRESSION ON THIS DATASET

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Temporary)

SET(TEMP ${CMAKE_CURRENT_BINARY_DIR}/Temporary)
SET(BASELINE_CONF ${OPJ_DATA_ROOT}/baseline/conformance)
SET(BASELINE_NR ${OPJ_DATA_ROOT}/baseline/nonregression)
SET(INPUT_CONF ${OPJ_DATA_ROOT}/input/conformance)


# List of components by file (normaly p0_13.j2k have 257 components but for this 
#set of test we consider only 4)
SET( CP0_nbC_list "not_used;1;1;1;3;4;4;3;3;1;3;1;1;4;3;1;1")
SET( CP0_ignore_list "not_used;0;0;1;1;1;1;1;1;0;1;0;0;1;1;1;0")
SET( CP1_nbC_list "not_used;1;3;4;1;3;3;2")
SET( CP1_ignore_list "not_used;0;1;1;0;1;1;1")

SET(COMMENTCODEVAR FALSE)


#--------------------------------------------------------------------------
# Tests about class 0 profile 0
# try to decode
# compare to ref file provided by the Executable Test Suite
# non regression comparison

# Parameters and tolerances given by Table C.1
SET( C0P0_ResFactor_list "not_used;0;0;0;3;3;3;0;5;2;0;0;0;0;2;0;0")
SET( C0P0_PEAK_list "not_used;0;0;0;33;54;109;10;7;4;10;0;0;0;0;0;0")
SET( C0P0_MSE_list "not_used;0;0;0;55.8;68;743;0.34;6.72;1.47;2.84;0;0;0;0;0;0")

FOREACH(numFileC0P0 RANGE 1 16) 

list(GET CP0_ignore_list ${numFileC0P0}  ignore_test_value)
IF(ignore_test_value EQUAL 0)

  # Build filenames
  IF(${numFileC0P0} LESS 10)
    SET( filenameInput p0_0${numFileC0P0}.j2k )
    SET( filenameRef   c0p0_0${numFileC0P0}.pgx )
  ELSE(${numFileC0P0} LESS 10)
    SET( filenameInput p0_${numFileC0P0}.j2k )
    SET( filenameRef   c0p0_${numFileC0P0}.pgx )
  ENDIF(${numFileC0P0} LESS 10)
  
  # Get corresponding tests parameters
  list(GET C0P0_ResFactor_list ${numFileC0P0}  ResFactor)
  list(GET CP0_nbC_list ${numFileC0P0}  nbComponents)
  list(GET C0P0_PEAK_list ${numFileC0P0}  PEAK_limit)
  list(GET C0P0_MSE_list ${numFileC0P0}  MSE_limit)
  
  # Manage cases which need to try different resolution reduction
  IF (numFileC0P0 EQUAL 3 OR numFileC0P0 EQUAL 15)
    get_filename_component(filenameRefSub ${filenameRef} NAME_WE)
    #r = 0
    
    ADD_TEST(ETS-C0P0-${filenameInput}-r0-decode 
        ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}-r0.pgx
        -r 0
        )
     
    ADD_TEST(ETS-C0P0-${filenameInput}-r0-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRefSub}r0.pgx
        -t ${TEMP}/c0${filenameInput}-r0.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )
      
    SET_TESTS_PROPERTIES(ETS-C0P0-${filenameInput}-r0-compare2ref  
                         PROPERTIES DEPENDS 
                         ETS-C0P0-${filenameInput}-r0-decode)    
                       
    ADD_TEST(NR-C0P0-${filenameInput}-r0-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRefSub}-r0.pgx
        -t ${TEMP}/c0${filenameInput}-r0.pgx
        -n ${nbComponents}
        -d 
        -s b_t_
        )
 
    SET_TESTS_PROPERTIES(NR-C0P0-${filenameInput}-r0-compare2base  
                         PROPERTIES DEPENDS 
                         ETS-C0P0-${filenameInput}-r0-decode)  
    
    #r = 1
    ADD_TEST(ETS-C0P0-${filenameInput}-r1-decode
        ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}-r1.pgx
        -r 1
        )
     
    ADD_TEST(ETS-C0P0-${filenameInput}-r1-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRefSub}r1.pgx
        -t ${TEMP}/c0${filenameInput}-r1.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )
      
    SET_TESTS_PROPERTIES(ETS-C0P0-${filenameInput}-r1-compare2ref  
                         PROPERTIES DEPENDS 
                         ETS-C0P0-${filenameInput}-r1-decode)    
                       
    ADD_TEST(NR-C0P0-${filenameInput}-r1-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRefSub}-r1.pgx
        -t ${TEMP}/c0${filenameInput}-r1.pgx
        -n ${nbComponents}
        -d 
        -s b_t_
        )
 
    SET_TESTS_PROPERTIES(NR-C0P0-${filenameInput}-r1-compare2base  
                         PROPERTIES DEPENDS 
                         ETS-C0P0-${filenameInput}-r1-decode) 

  ELSE(numFileC0P0 EQUAL 3 OR numFileC0P0 EQUAL 15)
  
    ADD_TEST(ETS-C0P0-${filenameInput}-decode
        ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}.pgx
        -r ${ResFactor}
        )
   
    ADD_TEST(ETS-C0P0-${filenameInput}-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRef}
        -t ${TEMP}/c0${filenameInput}.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )
      
    SET_TESTS_PROPERTIES(ETS-C0P0-${filenameInput}-compare2ref  
                         PROPERTIES DEPENDS 
                         ETS-C0P0-${filenameInput}-decode)    
                       
    ADD_TEST(NR-C0P0-${filenameInput}-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRef}
        -t ${TEMP}/c0${filenameInput}.pgx
        -n ${nbComponents}
        -d 
        -s b_t_
        )
 
    SET_TESTS_PROPERTIES(NR-C0P0-${filenameInput}-compare2base  
                         PROPERTIES DEPENDS 
                         ETS-C0P0-${filenameInput}-decode)    
      
  ENDIF(numFileC0P0 EQUAL 3 OR numFileC0P0 EQUAL 15)
  
  ENDIF(ignore_test_value EQUAL 0)

ENDFOREACH(numFileC0P0)

#--------------------------------------------------------------------------
# Tests about class 0 profile 1
# try to decode
# compare to ref file
# non regression comparison

# Parameters and tolerances given by Table C.4
SET( C0P1_ResFactor_list "not_used;0;3;3;0;4;1;0")
SET( C0P1_PEAK_list "not_used;0;35;28;2;128;128;0")
SET( C0P1_MSE_list "not_used;0;74;18.8;0.550;16384;16384;0")

FOREACH(numFileC0P1 RANGE 1 7) 

list(GET CP1_ignore_list ${numFileC0P1}  ignore_test_value2)
IF(ignore_test_value2 EQUAL 0)

  # Build filenames
  SET( filenameInput p1_0${numFileC0P1}.j2k )
  SET( filenameRef   c0p1_0${numFileC0P1}.pgx )

  # Get corresponding tests parameters 
  list(GET C0P1_ResFactor_list ${numFileC0P1}  ResFactor)
  list(GET CP1_nbC_list ${numFileC0P1}  nbComponents)
  list(GET C0P1_PEAK_list ${numFileC0P1}  PEAK_limit)
  list(GET C0P1_MSE_list ${numFileC0P1}  MSE_limit) 

  # Manage cases which need to try different resolution reduction
  IF (numFileC0P1 EQUAL 4 )
    get_filename_component(filenameRefSub ${filenameRef} NAME_WE)
    
    #r = 0
    ADD_TEST(ETS-C0P1-${filenameInput}-r0-decode 
        ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}-r0.pgx
        -r 0
        )
    
    ADD_TEST(ETS-C0P1-${filenameInput}-r0-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRefSub}r0.pgx
        -t ${TEMP}/c0${filenameInput}-r0.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )
      
    SET_TESTS_PROPERTIES(ETS-C0P1-${filenameInput}-r0-compare2ref  
                         PROPERTIES DEPENDS 
                         ETS-C0P1-${filenameInput}-r0-decode)    
                       
    ADD_TEST(NR-C0P1-${filenameInput}-r0-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRefSub}-r0.pgx
        -t ${TEMP}/c0${filenameInput}-r0.pgx
        -n ${nbComponents}
        -d 
        -s b_t_
        )
 
    SET_TESTS_PROPERTIES(NR-C0P1-${filenameInput}-r0-compare2base  
                         PROPERTIES DEPENDS 
                         ETS-C0P1-${filenameInput}-r0-decode)
    
    #r = 3
    ADD_TEST(ETS-C0P1-${filenameInput}-r3-decode
        ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}-r3.pgx
        -r 3
        )
        
    ADD_TEST(ETS-C0P1-${filenameInput}-r3-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRefSub}r3.pgx
        -t ${TEMP}/c0${filenameInput}-r3.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )
      
    SET_TESTS_PROPERTIES(ETS-C0P1-${filenameInput}-r3-compare2ref  
                         PROPERTIES DEPENDS 
                         ETS-C0P1-${filenameInput}-r3-decode)    
                       
    ADD_TEST(NR-C0P1-${filenameInput}-r3-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRefSub}-r3.pgx
        -t ${TEMP}/c0${filenameInput}-r3.pgx
        -n ${nbComponents}
        -d 
        -s b_t_
        )
 
    SET_TESTS_PROPERTIES(NR-C0P1-${filenameInput}-r3-compare2base  
                         PROPERTIES DEPENDS 
                         ETS-C0P1-${filenameInput}-r3-decode)
       
  ELSE(numFileC0P1 EQUAL 4)

    ADD_TEST(ETS-C0P1-${filenameInput}-decode
        ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}.pgx
        -r ${ResFactor}
        )
            
    ADD_TEST(ETS-C0P1-${filenameInput}-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRef}
        -t ${TEMP}/c0${filenameInput}.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )
      
    SET_TESTS_PROPERTIES(ETS-C0P1-${filenameInput}-compare2ref  
                         PROPERTIES DEPENDS 
                         ETS-C0P1-${filenameInput}-decode)    
                       
    ADD_TEST(NR-C0P1-${filenameInput}-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRef}
        -t ${TEMP}/c0${filenameInput}.pgx
        -n ${nbComponents}
        -d 
        -s b_t_
        )
 
    SET_TESTS_PROPERTIES(NR-C0P1-${filenameInput}-compare2base  
                         PROPERTIES DEPENDS 
                         ETS-C0P1-${filenameInput}-decode)
            
  ENDIF(numFileC0P1 EQUAL 4)
  
    ENDIF(ignore_test_value2 EQUAL 0)

ENDFOREACH(numFileC0P1)

#--------------------------------------------------------------------------
# Tests about class 1 profile 0
# try to decode
# compare to ref file
# non regression comparison

# Parameters and tolerances given by Table C.6
SET( C1P0_ResFactor_list "not_used;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0")
SET( C1P0_PEAK_list "not_used;0;0;0;5:4:6;2:2:2:0;635:403:378:0;0:0:0;0:0:0;0;0:0:0;0;0;0:0:0:0;0:0:0;0;0")
SET( C1P0_MSE_list  "not_used;0;0;0;0.776:0.626:1.070;0.302:0.307:0.269:0;11287:6124:3968:0;0:0:0;0:0:0;0;0:0:0;0;0;0:0:0:0;0:0:0;0;0")

FOREACH(numFileC1P0 RANGE 1 16) 

  # Build filenames
  IF(${numFileC1P0} LESS 10)
    SET( filenameInput p0_0${numFileC1P0}.j2k )
    SET( filenameRef   c1p0_0${numFileC1P0}.pgx )
  ELSE(${numFileC1P0} LESS 10)
    SET( filenameInput p0_${numFileC1P0}.j2k )
    SET( filenameRef   c1p0_${numFileC1P0}.pgx )
  ENDIF(${numFileC1P0} LESS 10)
  
  # Get corresponding tests parameters 
  list(GET CP0_nbC_list ${numFileC1P0}  nbComponents)
  list(GET C1P0_ResFactor_list ${numFileC1P0}  ResFactor)
  list(GET C1P0_PEAK_list ${numFileC1P0}  PEAK_limit)
  list(GET C1P0_MSE_list ${numFileC1P0}  MSE_limit)
  
  ADD_TEST(ETS-C1P0-${filenameInput}-decode
      ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
      -i ${INPUT_CONF}/${filenameInput}
      -o ${TEMP}/c1${filenameInput}.pgx
      -r ${ResFactor}
      )
      
  ADD_TEST(ETS-C1P0-${filenameInput}-compare2ref
      ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
     -b ${BASELINE_CONF}/${filenameRef}
     -t ${TEMP}/c1${filenameInput}.pgx
     -n ${nbComponents}
     -p ${PEAK_limit}
     -m ${MSE_limit}
     -s b_t_
     )
      
  SET_TESTS_PROPERTIES(ETS-C1P0-${filenameInput}-compare2ref  
                       PROPERTIES DEPENDS 
                       ETS-C1P0-${filenameInput}-decode)    
                       
  ADD_TEST(NR-C1P0-${filenameInput}-compare2base
      ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
     -b ${BASELINE_NR}/opj_${filenameRef}
     -t ${TEMP}/c1${filenameInput}.pgx
     -n ${nbComponents}
     -d 
     -s b_t_
     )
 
  SET_TESTS_PROPERTIES(NR-C1P0-${filenameInput}-compare2base  
                       PROPERTIES DEPENDS 
                       ETS-C1P0-${filenameInput}-decode)
  

    
ENDFOREACH(numFileC1P0)

#--------------------------------------------------------------------------
# Tests about class 1 profile 1
# try to decode
# compare to ref file
# non regression comparison

# Parameters and tolerances given by Table C.7
SET( C1P1_PEAK_list "not_used;0;5:4:6;2:2:1:0;624;40:40:40;2:2:2;0:0")
SET( C1P1_MSE_list  "not_used;0;0.765:0.616:1.051;0.3:0.210:0.200:0;3080;8.458:9.816:10.154;0.6:0.6:0.6;0:0")

FOREACH(numFileC1P1 RANGE 1 7) 

  # Build filenames
  SET( filenameInput p1_0${numFileC1P1}.j2k )
  SET( filenameRef   c1p1_0${numFileC1P1}.pgx )
  
  # Get corresponding tests parameters
  list(GET CP1_nbC_list ${numFileC1P1}  nbComponents)
  list(GET C1P1_PEAK_list ${numFileC1P1}  PEAK_limit)
  list(GET C1P1_MSE_list ${numFileC1P1}  MSE_limit)
  
  ADD_TEST(ETS-C1P1-${filenameInput}-decode
      ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
      -i ${INPUT_CONF}/${filenameInput}
      -o ${TEMP}/c1${filenameInput}.pgx
      -r 0
      )
 
  ADD_TEST(ETS-C1P1-${filenameInput}-compare2ref
      ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
     -b ${BASELINE_CONF}/${filenameRef}
     -t ${TEMP}/c1${filenameInput}.pgx
     -n ${nbComponents}
     -p ${PEAK_limit}
     -m ${MSE_limit}
     -s b_t_
     )
      
  SET_TESTS_PROPERTIES(ETS-C1P1-${filenameInput}-compare2ref  
                       PROPERTIES DEPENDS 
                       ETS-C1P1-${filenameInput}-decode)
                       
  ADD_TEST(NR-C1P1-${filenameInput}-compare2base
      ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
     -b ${BASELINE_NR}/opj_${filenameRef}
     -t ${TEMP}/c1${filenameInput}.pgx
     -n ${nbComponents}
     -d 
     -s b_t_
     )
      
  SET_TESTS_PROPERTIES(NR-C1P1-${filenameInput}-compare2base  
                       PROPERTIES DEPENDS 
                       ETS-C1P1-${filenameInput}-decode)
                       
ENDFOREACH(numFileC1P1)

#--------------------------------------------------------------------------
# Tests about JP2 file
# try to decode
# compare to ref file
# non regression comparison

# Tolerances given by Table G.1

FOREACH(numFileJP2 RANGE 1 9) 

  # Build filenames
  SET( filenameInput "file${numFileJP2}.jp2" )
  SET( filenameRef   jp2_${numFileJP2}.tif )
  
  # Get corresponding tests parameters
  list(GET JP2_PEAK_list ${numFileJP2}  PEAK_limit)
  
  ADD_TEST(ETS-JP2-${filenameInput}-decode
      ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image 
      -i ${INPUT_CONF}/${filenameInput}
      -o ${TEMP}/${filenameInput}.tif
      )
#
# FIXME: Need to implement a compare tif images function
#  
#  ADD_TEST(ETS-JP2-${filenameInput}-compare2ref
#      ${EXECUTABLE_OUTPUT_PATH}/compareTIFimages
#     -b ${BASELINE_CONF}/${filenameRef_tif}
#     -t ${TEMP}/${filenameInput}.pgx
#     -p 4
#     )
#      
#  SET_TESTS_PROPERTIES(ETS-JP2-${filenameInput}-compare2ref  
#                       PROPERTIES DEPENDS 
#                       ETS-JP2-${filenameInput}-decode)
#                       
#  ADD_TEST(NR-JP2-${filenameInput}-compare2base
#      ${EXECUTABLE_OUTPUT_PATH}/compareTIFimages
#     -b ${BASELINE_NR}/opj_${filenameRef}
#     -t ${TEMP}/${filenameInput}.tif
#     -d 
#     )
#      
#  SET_TESTS_PROPERTIES(NR-JP2-${filenameInput}-compare2base  
#                       PROPERTIES DEPENDS 
#                       ETS-JP2-${filenameInput}-decode)
                       
ENDFOREACH(numFileJP2)

#--------------------------------------------------------------------------#
#--------------------------------------------------------------------------#
#--------------------------------------------------------------------------#

#--------------------------------------------------------------------------
# Tests about dump of profile 0 file
# try to dump image and codestream informations into a file
# non regression comparison this file to the baseline

FOREACH(numFileP0 RANGE 1 16) 

  # Build filenames
  IF(${numFileP0} LESS 10)
    SET( filenameInput p0_0${numFileP0}.j2k )
  ELSE(${numFileP0} LESS 10)
    SET( filenameInput p0_${numFileP0}.j2k )
  ENDIF(${numFileP0} LESS 10)
  get_filename_component(filenameRefSub ${filenameInput} NAME_WE)


  ADD_TEST(NR-${filenameInput}-dump
      ${EXECUTABLE_OUTPUT_PATH}/j2k_dump
     -i ${INPUT_CONF}/${filenameInput}
     -o ${TEMP}/${filenameInput}.txt
     )
     
  ADD_TEST(NR-${filenameInput}-compare_dump2base
      ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
     -b ${BASELINE_NR}/opj_${filenameRefSub}.txt
     -t ${TEMP}/${filenameInput}.txt
     )
     
  SET_TESTS_PROPERTIES(NR-${filenameInput}-compare_dump2base  
                       PROPERTIES DEPENDS 
                       NR-${filenameInput}-dump)
                       
ENDFOREACH(numFileP0) 

#--------------------------------------------------------------------------
# Tests about dump of profile 1 file
# try to dump image and codestream informations into a file
# non regression comparison this file to the baseline

FOREACH(numFileP1 RANGE 1 7) 

  # Build filenames
  SET( filenameInput p1_0${numFileP1}.j2k )
  get_filename_component(filenameInputSub ${filenameInput} NAME_WE)

  ADD_TEST(NR-${filenameInput}-dump
      ${EXECUTABLE_OUTPUT_PATH}/j2k_dump
     -i ${INPUT_CONF}/${filenameInput}
     -o ${TEMP}/${filenameInput}.txt
     )
     
  ADD_TEST(NR-${filenameInput}-compare_dump2base
      ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
     -b ${BASELINE_NR}/opj_${filenameInputSub}.txt
     -t ${TEMP}/${filenameInput}.txt
     )
     
  SET_TESTS_PROPERTIES(NR-${filenameInput}-compare_dump2base
                       PROPERTIES DEPENDS 
                       NR-${filenameInput}-dump) 
                       
ENDFOREACH(numFileP1) 

#--------------------------------------------------------------------------
# Tests about dump of JP2 file
# try to dump image and codestream informations into a file
# non regression comparison this file to the baseline

FOREACH(numFileJP2 RANGE 1 9) 

  # Build filenames
  SET( filenameInput "file${numFileJP2}.jp2" )
  get_filename_component(filenameInputSub ${filenameInput} NAME_WE)

  ADD_TEST(NR-${filenameInput}-dump
      ${EXECUTABLE_OUTPUT_PATH}/j2k_dump
     -i ${INPUT_CONF}/${filenameInput}
     -o ${TEMP}/${filenameInput}.txt
     )
     
  ADD_TEST(NR-${filenameInput}-compare_dump2base
      ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
     -b ${BASELINE_NR}/opj_${filenameInputSub}.txt
     -t ${TEMP}/${filenameInput}.txt
     )
     
  SET_TESTS_PROPERTIES(NR-${filenameInput}-compare_dump2base
                       PROPERTIES DEPENDS 
                       NR-${filenameInput}-dump) 
                       
ENDFOREACH(numFileJP2) 