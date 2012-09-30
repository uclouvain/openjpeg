# CONFORMANCE TESTS AND NON-REGRESSION ON THIS DATASET

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Temporary)

set(TEMP ${CMAKE_CURRENT_BINARY_DIR}/Temporary)
set(BASELINE_CONF ${OPJ_DATA_ROOT}/baseline/conformance)
set(BASELINE_NR ${OPJ_DATA_ROOT}/baseline/nonregression)
set(INPUT_CONF ${OPJ_DATA_ROOT}/input/conformance)


# List of components by file (normaly p0_13.j2k have 257 components but for this
#set of test we consider only 4)
set( CP0_nbC_list "not_used;1;1;1;3;4;4;3;3;1;3;1;1;4;3;1;1")
set( CP1_nbC_list "not_used;1;3;4;1;3;3;2")

set(COMMENTCODEVAR FALSE)


#--------------------------------------------------------------------------
# Tests about class 0 profile 0
# try to decode
# compare to ref file provided by the Executable Test Suite
# non regression comparison

# Parameters and tolerances given by Table C.1
set( C0P0_ResFactor_list "not_used;0;0;0;3;3;3;0;5;2;0;0;0;0;2;0;0")
set( C0P0_PEAK_list "not_used;0;0;0;33;54;109;10;7;4;10;0;0;0;0;0;0")
set( C0P0_MSE_list "not_used;0;0;0;55.8;68;743;0.34;6.72;1.47;2.84;0;0;0;0;0;0")

foreach(numFileC0P0 RANGE 1 16)

  # Build filenames
  if(${numFileC0P0} LESS 10)
    set( filenameInput p0_0${numFileC0P0}.j2k )
    set( filenameRef   c0p0_0${numFileC0P0}.pgx )
  else()
    set( filenameInput p0_${numFileC0P0}.j2k )
    set( filenameRef   c0p0_${numFileC0P0}.pgx )
  endif()

  # Get corresponding tests parameters
  list(GET C0P0_ResFactor_list ${numFileC0P0}  ResFactor)
  list(GET CP0_nbC_list ${numFileC0P0}  nbComponents)
  list(GET C0P0_PEAK_list ${numFileC0P0}  PEAK_limit)
  list(GET C0P0_MSE_list ${numFileC0P0}  MSE_limit)

  # Manage cases which need to try different resolution reduction
  if (numFileC0P0 EQUAL 3 OR numFileC0P0 EQUAL 15)
    get_filename_component(filenameRefSub ${filenameRef} NAME_WE)
    #r = 0

    add_test(ETS-C0P0-${filenameInput}-r0-decode
        ${EXECUTABLE_OUTPUT_PATH}/opj_decompress
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}-r0.pgx
        -r 0
        )

    add_test(ETS-C0P0-${filenameInput}-r0-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRefSub}r0.pgx
        -t ${TEMP}/c0${filenameInput}-r0.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )

    set_tests_properties(ETS-C0P0-${filenameInput}-r0-compare2ref
                         PROPERTIES DEPENDS
                         ETS-C0P0-${filenameInput}-r0-decode)

    add_test(NR-C0P0-${filenameInput}-r0-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRefSub}-r0.pgx
        -t ${TEMP}/c0${filenameInput}-r0.pgx
        -n ${nbComponents}
        -d
        -s b_t_
        )

    set_tests_properties(NR-C0P0-${filenameInput}-r0-compare2base
                         PROPERTIES DEPENDS
                         ETS-C0P0-${filenameInput}-r0-decode)

    #r = 1
    add_test(ETS-C0P0-${filenameInput}-r1-decode
        ${EXECUTABLE_OUTPUT_PATH}/opj_decompress
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}-r1.pgx
        -r 1
        )

    add_test(ETS-C0P0-${filenameInput}-r1-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRefSub}r1.pgx
        -t ${TEMP}/c0${filenameInput}-r1.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )

    set_tests_properties(ETS-C0P0-${filenameInput}-r1-compare2ref
                         PROPERTIES DEPENDS
                         ETS-C0P0-${filenameInput}-r1-decode)

    add_test(NR-C0P0-${filenameInput}-r1-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRefSub}-r1.pgx
        -t ${TEMP}/c0${filenameInput}-r1.pgx
        -n ${nbComponents}
        -d
        -s b_t_
        )

    set_tests_properties(NR-C0P0-${filenameInput}-r1-compare2base
                         PROPERTIES DEPENDS
                         ETS-C0P0-${filenameInput}-r1-decode)

  else()

    add_test(ETS-C0P0-${filenameInput}-decode
        ${EXECUTABLE_OUTPUT_PATH}/opj_decompress
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}.pgx
        -r ${ResFactor}
        )

    add_test(ETS-C0P0-${filenameInput}-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRef}
        -t ${TEMP}/c0${filenameInput}.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )

    set_tests_properties(ETS-C0P0-${filenameInput}-compare2ref
                         PROPERTIES DEPENDS
                         ETS-C0P0-${filenameInput}-decode)

    add_test(NR-C0P0-${filenameInput}-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRef}
        -t ${TEMP}/c0${filenameInput}.pgx
        -n ${nbComponents}
        -d
        -s b_t_
        )

    set_tests_properties(NR-C0P0-${filenameInput}-compare2base
                         PROPERTIES DEPENDS
                         ETS-C0P0-${filenameInput}-decode)

  endif()

endforeach()

#--------------------------------------------------------------------------
# Tests about class 0 profile 1
# try to decode
# compare to ref file
# non regression comparison

# Parameters and tolerances given by Table C.4
set( C0P1_ResFactor_list "not_used;0;3;3;0;4;1;0")
set( C0P1_PEAK_list "not_used;0;35;28;2;128;128;0")
set( C0P1_MSE_list "not_used;0;74;18.8;0.550;16384;16384;0")

foreach(numFileC0P1 RANGE 1 7)

  # Build filenames
  set( filenameInput p1_0${numFileC0P1}.j2k )
  set( filenameRef   c0p1_0${numFileC0P1}.pgx )

  # Get corresponding tests parameters
  list(GET C0P1_ResFactor_list ${numFileC0P1}  ResFactor)
  list(GET CP1_nbC_list ${numFileC0P1}  nbComponents)
  list(GET C0P1_PEAK_list ${numFileC0P1}  PEAK_limit)
  list(GET C0P1_MSE_list ${numFileC0P1}  MSE_limit)

  # Manage cases which need to try different resolution reduction
  if (numFileC0P1 EQUAL 4 )
    get_filename_component(filenameRefSub ${filenameRef} NAME_WE)

    #r = 0
    add_test(ETS-C0P1-${filenameInput}-r0-decode
        ${EXECUTABLE_OUTPUT_PATH}/opj_decompress
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}-r0.pgx
        -r 0
        )

    add_test(ETS-C0P1-${filenameInput}-r0-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRefSub}r0.pgx
        -t ${TEMP}/c0${filenameInput}-r0.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )

    set_tests_properties(ETS-C0P1-${filenameInput}-r0-compare2ref
                         PROPERTIES DEPENDS
                         ETS-C0P1-${filenameInput}-r0-decode)

    add_test(NR-C0P1-${filenameInput}-r0-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRefSub}-r0.pgx
        -t ${TEMP}/c0${filenameInput}-r0.pgx
        -n ${nbComponents}
        -d
        -s b_t_
        )

    set_tests_properties(NR-C0P1-${filenameInput}-r0-compare2base
                         PROPERTIES DEPENDS
                         ETS-C0P1-${filenameInput}-r0-decode)

    #r = 3
    add_test(ETS-C0P1-${filenameInput}-r3-decode
        ${EXECUTABLE_OUTPUT_PATH}/opj_decompress
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}-r3.pgx
        -r 3
        )

    add_test(ETS-C0P1-${filenameInput}-r3-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRefSub}r3.pgx
        -t ${TEMP}/c0${filenameInput}-r3.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )

    set_tests_properties(ETS-C0P1-${filenameInput}-r3-compare2ref
                         PROPERTIES DEPENDS
                         ETS-C0P1-${filenameInput}-r3-decode)

    add_test(NR-C0P1-${filenameInput}-r3-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRefSub}-r3.pgx
        -t ${TEMP}/c0${filenameInput}-r3.pgx
        -n ${nbComponents}
        -d
        -s b_t_
        )

    set_tests_properties(NR-C0P1-${filenameInput}-r3-compare2base
                         PROPERTIES DEPENDS
                         ETS-C0P1-${filenameInput}-r3-decode)

  else()

    add_test(ETS-C0P1-${filenameInput}-decode
        ${EXECUTABLE_OUTPUT_PATH}/opj_decompress
        -i ${INPUT_CONF}/${filenameInput}
        -o ${TEMP}/c0${filenameInput}.pgx
        -r ${ResFactor}
        )

    add_test(ETS-C0P1-${filenameInput}-compare2ref
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_CONF}/${filenameRef}
        -t ${TEMP}/c0${filenameInput}.pgx
        -n ${nbComponents}
        -p ${PEAK_limit}
        -m ${MSE_limit}
        -s t_
        )

    set_tests_properties(ETS-C0P1-${filenameInput}-compare2ref
                         PROPERTIES DEPENDS
                         ETS-C0P1-${filenameInput}-decode)

    add_test(NR-C0P1-${filenameInput}-compare2base
        ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
        -b ${BASELINE_NR}/opj_${filenameRef}
        -t ${TEMP}/c0${filenameInput}.pgx
        -n ${nbComponents}
        -d
        -s b_t_
        )

    set_tests_properties(NR-C0P1-${filenameInput}-compare2base
                         PROPERTIES DEPENDS
                         ETS-C0P1-${filenameInput}-decode)

  endif()

endforeach()

#--------------------------------------------------------------------------
# Tests about class 1 profile 0
# try to decode
# compare to ref file
# non regression comparison

# Parameters and tolerances given by Table C.6
set( C1P0_ResFactor_list "not_used;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0")
set( C1P0_PEAK_list "not_used;0;0;0;5:4:6;2:2:2:0;635:403:378:0;0:0:0;0:0:0;0;0:0:0;0;0;0:0:0:0;0:0:0;0;0")
set( C1P0_MSE_list  "not_used;0;0;0;0.776:0.626:1.070;0.302:0.307:0.269:0;11287:6124:3968:0;0:0:0;0:0:0;0;0:0:0;0;0;0:0:0:0;0:0:0;0;0")

foreach(numFileC1P0 RANGE 1 16)

  # Build filenames
  if(${numFileC1P0} LESS 10)
    set( filenameInput p0_0${numFileC1P0}.j2k )
    set( filenameRef   c1p0_0${numFileC1P0}.pgx )
  else()
    set( filenameInput p0_${numFileC1P0}.j2k )
    set( filenameRef   c1p0_${numFileC1P0}.pgx )
  endif()

  # Get corresponding tests parameters
  list(GET CP0_nbC_list ${numFileC1P0}  nbComponents)
  list(GET C1P0_ResFactor_list ${numFileC1P0}  ResFactor)
  list(GET C1P0_PEAK_list ${numFileC1P0}  PEAK_limit)
  list(GET C1P0_MSE_list ${numFileC1P0}  MSE_limit)

  add_test(ETS-C1P0-${filenameInput}-decode
      ${EXECUTABLE_OUTPUT_PATH}/opj_decompress
      -i ${INPUT_CONF}/${filenameInput}
      -o ${TEMP}/c1${filenameInput}.pgx
      -r ${ResFactor}
      )

  add_test(ETS-C1P0-${filenameInput}-compare2ref
      ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
     -b ${BASELINE_CONF}/${filenameRef}
     -t ${TEMP}/c1${filenameInput}.pgx
     -n ${nbComponents}
     -p ${PEAK_limit}
     -m ${MSE_limit}
     -s b_t_
     )

  set_tests_properties(ETS-C1P0-${filenameInput}-compare2ref
                       PROPERTIES DEPENDS
                       ETS-C1P0-${filenameInput}-decode)

  add_test(NR-C1P0-${filenameInput}-compare2base
      ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
     -b ${BASELINE_NR}/opj_${filenameRef}
     -t ${TEMP}/c1${filenameInput}.pgx
     -n ${nbComponents}
     -d
     -s b_t_
     )

  set_tests_properties(NR-C1P0-${filenameInput}-compare2base
                       PROPERTIES DEPENDS
                       ETS-C1P0-${filenameInput}-decode)

endforeach()

#--------------------------------------------------------------------------
# Tests about class 1 profile 1
# try to decode
# compare to ref file
# non regression comparison

# Parameters and tolerances given by Table C.7
set( C1P1_PEAK_list "not_used;0;5:4:6;2:2:1:0;624;40:40:40;2:2:2;0:0")
set( C1P1_MSE_list  "not_used;0;0.765:0.616:1.051;0.3:0.210:0.200:0;3080;8.458:9.816:10.154;0.6:0.6:0.6;0:0")

foreach(numFileC1P1 RANGE 1 7)

  # Build filenames
  set( filenameInput p1_0${numFileC1P1}.j2k )
  set( filenameRef   c1p1_0${numFileC1P1}.pgx )

  # Get corresponding tests parameters
  list(GET CP1_nbC_list ${numFileC1P1}  nbComponents)
  list(GET C1P1_PEAK_list ${numFileC1P1}  PEAK_limit)
  list(GET C1P1_MSE_list ${numFileC1P1}  MSE_limit)

  add_test(ETS-C1P1-${filenameInput}-decode
      ${EXECUTABLE_OUTPUT_PATH}/opj_decompress
      -i ${INPUT_CONF}/${filenameInput}
      -o ${TEMP}/c1${filenameInput}.pgx
      -r 0
      )

  add_test(ETS-C1P1-${filenameInput}-compare2ref
      ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
     -b ${BASELINE_CONF}/${filenameRef}
     -t ${TEMP}/c1${filenameInput}.pgx
     -n ${nbComponents}
     -p ${PEAK_limit}
     -m ${MSE_limit}
     -s b_t_
     )

  set_tests_properties(ETS-C1P1-${filenameInput}-compare2ref
                       PROPERTIES DEPENDS
                       ETS-C1P1-${filenameInput}-decode)

  add_test(NR-C1P1-${filenameInput}-compare2base
      ${EXECUTABLE_OUTPUT_PATH}/comparePGXimages
     -b ${BASELINE_NR}/opj_${filenameRef}
     -t ${TEMP}/c1${filenameInput}.pgx
     -n ${nbComponents}
     -d
     -s b_t_
     )

  set_tests_properties(NR-C1P1-${filenameInput}-compare2base
                       PROPERTIES DEPENDS
                       ETS-C1P1-${filenameInput}-decode)

endforeach()

#--------------------------------------------------------------------------
# Tests about JP2 file
# try to decode
# compare to ref file
# non regression comparison

# Tolerances given by Table G.1

foreach(numFileJP2 RANGE 1 9)

  # Build filenames
  set( filenameInput "file${numFileJP2}.jp2" )
  set( filenameRef   jp2_${numFileJP2}.tif )

  # Get corresponding tests parameters
  list(GET JP2_PEAK_list ${numFileJP2}  PEAK_limit)

  add_test(ETS-JP2-${filenameInput}-decode
      ${EXECUTABLE_OUTPUT_PATH}/opj_decompress
      -i ${INPUT_CONF}/${filenameInput}
      -o ${TEMP}/${filenameInput}.tif
      )
#
# FIXME: Need to implement a compare tif images function
#
#  add_test(ETS-JP2-${filenameInput}-compare2ref
#      ${EXECUTABLE_OUTPUT_PATH}/compareTIFimages
#     -b ${BASELINE_CONF}/${filenameRef_tif}
#     -t ${TEMP}/${filenameInput}.pgx
#     -p 4
#     )
#
#  set_tests_properties(ETS-JP2-${filenameInput}-compare2ref
#                       PROPERTIES DEPENDS
#                       ETS-JP2-${filenameInput}-decode)
#
#  add_test(NR-JP2-${filenameInput}-compare2base
#      ${EXECUTABLE_OUTPUT_PATH}/compareTIFimages
#     -b ${BASELINE_NR}/opj_${filenameRef}
#     -t ${TEMP}/${filenameInput}.tif
#     -d
#     )
#
#  set_tests_properties(NR-JP2-${filenameInput}-compare2base
#                       PROPERTIES DEPENDS
#                       ETS-JP2-${filenameInput}-decode)

endforeach()

#--------------------------------------------------------------------------#
#--------------------------------------------------------------------------#
#--------------------------------------------------------------------------#

#--------------------------------------------------------------------------
# Tests about dump of profile 0 file
# try to dump image and codestream informations into a file
# non regression comparison this file to the baseline

foreach(numFileP0 RANGE 1 16)

  # Build filenames
  if(${numFileP0} LESS 10)
    set( filenameInput p0_0${numFileP0}.j2k )
  else()
    set( filenameInput p0_${numFileP0}.j2k )
  endif()
  get_filename_component(filenameRefSub ${filenameInput} NAME_WE)


  add_test(NR-${filenameInput}-dump
      ${EXECUTABLE_OUTPUT_PATH}/opj_dump
     -i ${INPUT_CONF}/${filenameInput}
     -o ${TEMP}/${filenameInput}.txt
     )

  add_test(NR-${filenameInput}-compare_dump2base
      ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
     -b ${BASELINE_NR}/opj_v2_${filenameRefSub}.txt
     -t ${TEMP}/${filenameInput}.txt
     )

  set_tests_properties(NR-${filenameInput}-compare_dump2base
                       PROPERTIES DEPENDS
                       NR-${filenameInput}-dump)

endforeach()

#--------------------------------------------------------------------------
# Tests about dump of profile 1 file
# try to dump image and codestream informations into a file
# non regression comparison this file to the baseline

foreach(numFileP1 RANGE 1 7)

  # Build filenames
  set( filenameInput p1_0${numFileP1}.j2k )
  get_filename_component(filenameInputSub ${filenameInput} NAME_WE)

  add_test(NR-${filenameInput}-dump
      ${EXECUTABLE_OUTPUT_PATH}/opj_dump
     -i ${INPUT_CONF}/${filenameInput}
     -o ${TEMP}/${filenameInput}.txt
     )

  add_test(NR-${filenameInput}-compare_dump2base
      ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
     -b ${BASELINE_NR}/opj_v2_${filenameInputSub}.txt
     -t ${TEMP}/${filenameInput}.txt
     )

  set_tests_properties(NR-${filenameInput}-compare_dump2base
                       PROPERTIES DEPENDS
                       NR-${filenameInput}-dump)

endforeach()

#--------------------------------------------------------------------------
# Tests about dump of JP2 file
# try to dump image and codestream informations into a file
# non regression comparison this file to the baseline

foreach(numFileJP2 RANGE 1 9)

  # Build filenames
  set( filenameInput "file${numFileJP2}.jp2" )
  get_filename_component(filenameInputSub ${filenameInput} NAME_WE)

  add_test(NR-${filenameInput}-dump
      ${EXECUTABLE_OUTPUT_PATH}/opj_dump
     -i ${INPUT_CONF}/${filenameInput}
     -o ${TEMP}/${filenameInput}.txt
     )

  add_test(NR-${filenameInput}-compare_dump2base
      ${EXECUTABLE_OUTPUT_PATH}/compare_dump_files
     -b ${BASELINE_NR}/opj_v2_${filenameInputSub}.txt
     -t ${TEMP}/${filenameInput}.txt
     )

  set_tests_properties(NR-${filenameInput}-compare_dump2base
                       PROPERTIES DEPENDS
                       NR-${filenameInput}-dump)

endforeach()
