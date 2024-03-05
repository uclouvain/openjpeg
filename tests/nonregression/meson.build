    # NON-REGRESSION TESTS ON THIS DATASET LOCATED ${OPJ_DATA_ROOT}/input/nonregression
    # file(['MAKE_DIRECTORY', cmake_current_binary_dir, '/Temporary'])
    temp = [cmake_current_binary_dir, '/Temporary']
    baseline_nr = [opj_data_root, '/baseline/nonregression']
    input_nr = [opj_data_root, '/input/nonregression']
    input_nr_path = input_nr
    temp_path = temp
    input_conf_path = [opj_data_root, '/input/conformance']
    # need kdu_expand if possible
    KAKADU_dep = dependency('KAKADU')
    # need jpylyzer if possible
    JPYLYZER_dep = dependency('JPYLYZER')
    #########################################################################
    # GENERATION OF THE TEST SUITE (DUMP)
    # Dump all files with the selected extension inside the input directory
    # technically opj_dump should simply parse these one, since syntax is ok.
    blacklist_jpeg2000_tmp = ['2539.pdf.SIGFPE.706.1712.jp2', '0290cb77c5df21828fa74cf2ab2c84d8.SIGFPE.d25.31.jp2', '26ccf3651020967f7778238ef5af08af.SIGFPE.d25.527.jp2', '4035.pdf.SIGSEGV.d8b.3375.jp2', '3635.pdf.asan.77.2930.jp2', 'issue165.jp2', 'edf_c2_1178956.jp2', 'edf_c2_1000290.jp2', 'edf_c2_1377017.jp2', 'edf_c2_1002767.jp2', 'edf_c2_10025.jp2', 'edf_c2_1000234.jp2', 'edf_c2_225881.jp2', 'edf_c2_1000671.jp2', 'edf_c2_1015644.jp2', 'edf_c2_101463.jp2', 'edf_c2_1674177.jp2', 'edf_c2_1673169.jp2', 'issue418.jp2', 'issue429.jp2', 'issue427-null-image-size.jp2', 'issue427-illegal-tile-offset.jp2', 'issue495.jp2', 'issue820.jp2']
    # Define a list of file which should be gracefully rejected:
    blacklist_jpeg2000 = [blacklist_jpeg2000_tmp, 'broken1.jp2', 'broken2.jp2', 'broken3.jp2', 'broken4.jp2', 'gdal_fuzzer_assert_in_opj_j2k_read_SQcd_SQcc.patch.jp2', 'gdal_fuzzer_check_comp_dx_dy.jp2', 'gdal_fuzzer_check_number_of_tiles.jp2', 'gdal_fuzzer_unchecked_numresolutions.jp2', 'mem-b2ace68c-1381.jp2', '1851.pdf.SIGSEGV.ce9.948.jp2', '1888.pdf.asan.35.988.jp2', 'issue362-2863.jp2', 'issue362-2866.jp2', 'issue362-2894.jp2', 'issue400.jp2', 'issue364-38.jp2', 'issue364-903.jp2', 'issue393.jp2', 'issue408.jp2', 'issue420.jp2', '27ac957758a35d00d6765a0c86350d9c.SIGFPE.d25.537.jpc', '3672da2f1f67bbecad27d7181b4e9d7c.SIGFPE.d25.805.jpc', 'issue475.jp2', 'issue413.jp2', 'issue823.jp2']
    # file(['GLOB_RECURSE', 'OPJ_DATA_NR_LIST', '${INPUT_NR}/*.j2k', '${INPUT_NR}/*.j2c', '${INPUT_NR}/*.jp2', '${INPUT_NR}/*.jpc'])
    # foreach(['INPUT_FILENAME', opj_data_nr_list])
    # get_filename_component(['INPUT_FILENAME_NAME', input_filename, 'NAME'])
    #get_filename_component(INPUT_FILENAME_NAME_WE ${INPUT_FILENAME_NAME} NAME_WE)
    # cannot use longest extension function, since some name contains multiples
    # dots. Instead write out own shortest extension function:
    # string(['FIND', input_filename_name, '.', 'SHORTEST_EXT_POS', 'REVERSE'])
    # string(['SUBSTRING', input_filename_name, '0', shortest_ext_pos, 'INPUT_FILENAME_NAME_WE'])
    # string(['REGEX', 'MATCH', input_filename_name, 'bad_jpeg2000', blacklist_jpeg2000])
    # Dump the input image
    test('NAME', 'NR-${INPUT_FILENAME_NAME}-dump', 'COMMAND', 'opj_dump', '-i', input_filename, '-o', temp, '/${INPUT_FILENAME_NAME}.txt', '-v')
    if 'bad_jpeg2000'
      # set_tests_properties(['NR-${INPUT_FILENAME_NAME}-dump', 'PROPERTIES', 'WILL_FAIL', 'TRUE'])
    else
      # Compare the dump output with the baseline
      test('NAME', 'NR-${INPUT_FILENAME_NAME}-compare_dump2base', 'COMMAND', 'compare_dump_files', '-b', baseline_nr, '/opj_v2_${INPUT_FILENAME_NAME_WE}.txt', '-t', temp, '/${INPUT_FILENAME_NAME}.txt')
      # set_tests_properties(['NR-${INPUT_FILENAME_NAME}-compare_dump2base', 'PROPERTIES', 'DEPENDS', 'NR-${INPUT_FILENAME_NAME}-dump'])
    endif
    # endforeach()
    #########################################################################
    # GENERATION OF THE TEST SUITE (DECODE AND ENCODE)
    # Read one and more input file(s) (located in ${OPJ_DATA_ROOT}/input/nonregression)
    # to know which files processed and with which options.
    # Configure the test suite file:
    # configure_file(['test_suite.ctest.in', '${CMAKE_CURRENT_BINARY_DIR}/test_suite.ctest', '@ONLY'])
    # Read the file into a list
    # file(['STRINGS', cmake_current_binary_dir, '/test_suite.ctest', 'OPJ_TEST_CMD_LINE_LIST'])
    # Try to find and configure and read another test suite file
    # file(['GLOB', 'TEST_SUITE_FILES', '*.ctest.in'])
    if 'TEST_SUITE_FILES'
      # foreach(['TEST_SUITE_FILE', test_suite_files])
      # Avoid to process the official test suite
      file_already_read = '0'
      # get_filename_component(['TEST_SUITE_FILENAME', test_suite_file, 'NAME'])
      # string(['REGEX', 'MATCH', '^test_suite.ctest.in$', 'FILE_ALREADY_READ', test_suite_filename])
      if 'NOT', 'FILE_ALREADY_READ'
        # Configure the additional test suite file:
        # get_filename_component(['TEST_SUITE_FILE_SUB', test_suite_file, 'NAME_WE'])
        # configure_file(['${TEST_SUITE_FILE}', '${CMAKE_CURRENT_BINARY_DIR}/${TEST_SUITE_FILE_SUB}.ctest', '@ONLY'])
        # Read the additional file into a list
        # file(['STRINGS', cmake_current_binary_dir, '/${TEST_SUITE_FILE_SUB}.ctest', 'OPJ_TEST_CMD_LINE_LIST_TEMP'])
        # Append the list of command
        opj_test_cmd_line_list = [opj_test_cmd_line_list, opj_test_cmd_line_list_temp]
      endif
      # endforeach()
    else
      # message(['FATAL_ERROR', 'One test suite should be available (test_suite.ctest.in) !!!'])
    endif
    nonregression_filenames_used = 
    # Parse the command line found in the file(s)
    it_test_enc = '0'
    it_test_dec = '0'
    # foreach(['OPJ_TEST_CMD_LINE', opj_test_cmd_line_list])
    ignore_line_found = '0'
    # Replace space by ; to generate a list
    # string(['REPLACE', ' ', ';', 'CMD_ARG_LIST', opj_test_cmd_line])
    # Check if the first argument begin by the comment sign
    # list(['GET', 'CMD_ARG_LIST', '0', 'EXE_NAME'])
    if 'EXE_NAME'
      # string(['REGEX', 'MATCH', '^#', 'IGNORE_LINE_FOUND', exe_name])
    endif
    if 'IGNORE_LINE_FOUND', 'OR', 'NOT', 'EXE_NAME'
      #message( STATUS "Current line is ignored: ${OPJ_TEST_CMD_LINE}")
    else
      # Check if the first argument begin by the failed sign
      failed_test_found = '0'
      # string(['REGEX', 'MATCH', '^!', 'FAILED_TEST_FOUND', exe_name])
      if 'FAILED_TEST_FOUND'
        # Manage the different cases with the failed sign to remove the first argument which must be opj_compress
        failed_test_found_1 = '0'
        # string(['REGEX', 'MATCH', '^!opj_compress$|^!opj_decompress$', 'FAILED_TEST_FOUND_1', exe_name])
        if 'FAILED_TEST_FOUND_1'
          # list(['REMOVE_AT', 'CMD_ARG_LIST', '0'])
        else
          failed_test_found_2 = '0'
          # list(['GET', 'CMD_ARG_LIST', '1', 'EXE_NAME'])
          # string(['REGEX', 'MATCH', '^opj_compress$|^opj_decompress$', 'FAILED_TEST_FOUND_2', exe_name])
          if 'FAILED_TEST_FOUND_2'
            # list(['REMOVE_AT', 'CMD_ARG_LIST', '0'])
            # list(['REMOVE_AT', 'CMD_ARG_LIST', '0'])
          else
            # message(['FATAL_ERROR', '${EXE_NAME} is not the right executable name to encode file (try to use opj_compress or opj_decompress)'])
          endif
        endif
      else
        # Check if the first argument is equal to opj_compress
        # string(['REGEX', 'MATCH', '^opj_compress$|^opj_compress_no_raw$|^opj_compress_no_raw_lossless$|^opj_decompress$', 'EXE_NAME_FOUND', exe_name])
        if 'EXE_NAME_FOUND'
          # string(['REGEX', 'MATCH', '^opj_compress$|^opj_compress_no_raw$|^opj_compress_no_raw_lossless$', 'ENC_TEST_FOUND', exe_name])
          # string(['REGEX', 'MATCH', '^opj_compress_no_raw$|^opj_compress_no_raw_lossless$', 'NO_RAW', exe_name])
          # string(['REGEX', 'MATCH', 'opj_compress_no_raw_lossless', 'LOSSLESS', exe_name])
        else
          # message(['FATAL_ERROR', '${EXE_NAME} is not the right executable name to encode file (try to use opj_compress)'])
        endif
        # list(['REMOVE_AT', 'CMD_ARG_LIST', '0'])
      endif
      # Parse the argument list to find the input filename and output filename
      cmd_arg_list_2 = ''
      arg_pos = '0'
      input_arg_pos = '0'
      output_arg_pos = '0'
      # foreach(['CMD_ARG_ELT', cmd_arg_list])
      # math(['EXPR', 'ARG_POS', '${ARG_POS}+1'])
      # string(['COMPARE', 'EQUAL', cmd_arg_elt, '-i', 'INPUT_ARG_FOUND'])
      if 'INPUT_ARG_FOUND'
        input_arg_pos = arg_pos
        input_arg_found = '0'
      endif
      # string(['COMPARE', 'EQUAL', cmd_arg_elt, '-o', 'OUTPUT_ARG_FOUND'])
      if 'OUTPUT_ARG_FOUND'
        output_arg_pos = arg_pos
        output_arg_found = '0'
      endif
      # list(['APPEND', 'CMD_ARG_LIST_2', cmd_arg_elt])
      # endforeach()
      # list(['GET', 'CMD_ARG_LIST_2', input_arg_pos, 'INPUT_FILENAME'])
      # get_filename_component(['INPUT_FILENAME_NAME', input_filename, 'NAME'])
      # get_filename_component(['INPUT_FILENAME_NAME_WE', input_filename_name, 'NAME_WE'])
      # list(['GET', 'CMD_ARG_LIST_2', output_arg_pos, 'OUTPUT_FILENAME'])
      # get_filename_component(['OUTPUT_FILENAME_NAME_WE', output_filename, 'NAME_WE'])
      #-----
      # Now we can add the test suite corresponding to a line command in the file
      #-----
      # ENCODER TEST SUITE
      if 'ENC_TEST_FOUND'
        # math(['EXPR', 'IT_TEST_ENC', '${IT_TEST_ENC}+1'])
        # Encode an image into the jpeg2000 format
        test('NAME', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode', 'COMMAND', 'opj_compress', cmd_arg_list_2)
        if 'FAILED_TEST_FOUND'
          # set_tests_properties(['NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode', 'PROPERTIES', 'WILL_FAIL', 'TRUE'])
        else
          # Dump the encoding file
          test('NAME', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-dump', 'COMMAND', 'opj_dump', '-i', output_filename, '-o', output_filename, '-ENC-${IT_TEST_ENC}.txt')
          # set_tests_properties(['NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-dump', 'PROPERTIES', 'DEPENDS', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode'])
          # Compare the dump file with the baseline
          test('NAME', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dump2base', 'COMMAND', 'compare_dump_files', '-b', baseline_nr, '/opj_v2_${OUTPUT_FILENAME_NAME_WE}-ENC-${IT_TEST_ENC}.txt', '-t', output_filename, '-ENC-${IT_TEST_ENC}.txt')
          # set_tests_properties(['NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dump2base', 'PROPERTIES', 'DEPENDS', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-dump'])
          # Decode the encoding file with kakadu expand command
          if 'KDU_EXPAND_EXECUTABLE'
            test('NAME', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-decode-ref', 'COMMAND', kdu_expand_executable, '-i', output_filename, '-o', output_filename, '.raw')
            # set_tests_properties(['NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-decode-ref', 'PROPERTIES', 'DEPENDS', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode'])
            if 'NOT', 'NO_RAW'
              # Compare the decoding file with baseline generated from the kdu_expand and baseline.j2k
              test('NAME', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dec-ref-out2base', 'COMMAND', 'compare_raw_files', '-b', baseline_nr, '/opj_${OUTPUT_FILENAME_NAME_WE}-ENC-${IT_TEST_ENC}.raw', '-t', output_filename, '.raw')
              # set_tests_properties(['NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-compare_dec-ref-out2base', 'PROPERTIES', 'DEPENDS', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-decode-ref'])
            endif
          endif
          # Test the encoded file is a valid JP2 file
          if 'JPYLYZER_EXECUTABLE'
            if output_filename, 'MATCHES', '\\.jp2$'
              test('NAME', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-jpylyser', 'COMMAND', jpylyzer_executable, output_filename)
              # set_tests_properties(['NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-jpylyser', 'PROPERTIES', 'DEPENDS', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode', 'PASS_REGULAR_EXPRESSION', '<isValidJP2>True</isValidJP2>'])
            endif
          endif
          # If lossless compression (simple test is 4 arguments), decompress & compare
          # list(['LENGTH', 'CMD_ARG_LIST_2', 'ARG_COUNT'])
          if (  ARG_COUNT  EQUAL  4 ) OR LOSSLESS
            # can we compare with the input image ?
            if input_filename_name, 'MATCHES', '\\.tif$'
              test('NAME', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-lossless-decode', 'COMMAND', 'opj_decompress', '-i', output_filename, '-o', output_filename, '.lossless.tif')
              # set_tests_properties(['NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-lossless-decode', 'PROPERTIES', 'DEPENDS', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-encode'])
              test('NAME', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-lossless-compare', 'COMMAND', 'compare_images', '-b', input_filename, '-t', output_filename, '.lossless.tif', '-n', '1', '-d')
              # set_tests_properties(['NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-lossless-compare', 'PROPERTIES', 'DEPENDS', 'NR-ENC-${INPUT_FILENAME_NAME}-${IT_TEST_ENC}-lossless-decode'])
            endif
          endif
        endif
        # DECODER TEST SUITE
      else
        # string(['FIND', input_filename, 'nonregression', 'nr_pos'])
        if nr_pos, 'GREATER', '0'
          # list(['APPEND', 'nonregression_filenames_used', input_filename_name])
        endif
        # math(['EXPR', 'IT_TEST_DEC', '${IT_TEST_DEC}+1'])
        # Decode the input image
        test('NAME', 'NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-decode', 'COMMAND', 'opj_decompress', cmd_arg_list_2)
        if 'FAILED_TEST_FOUND'
          # set_tests_properties(['NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-decode', 'PROPERTIES', 'WILL_FAIL', 'TRUE'])
        else
          # if not failed, check against registered md5:
          test('NAME', 'NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-decode-md5', 'COMMAND', cmake_command, '-DREFFILE:STRING=${CMAKE_CURRENT_SOURCE_DIR}/md5refs.txt', '-DOUTFILENAME:STRING=${OUTPUT_FILENAME}', '-P', cmake_current_source_dir, '/checkmd5refs.cmake')
          # set_tests_properties(['NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-decode-md5', 'PROPERTIES', 'DEPENDS', 'NR-DEC-${INPUT_FILENAME_NAME}-${IT_TEST_DEC}-decode'])
          # FIXME: add a compare2base function base on raw which
          # can output png diff files if necessary
          #  add_test(NR-${filename}-compare2base
          #      ${EXECUTABLE_OUTPUT_PATH}/compare_images
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
        endif
      endif
    endif
    # endforeach()
    existing_filenames = 
    # foreach(['f', opj_data_nr_list])
    # get_filename_component(['ff', f, 'NAME'])
    # list(['APPEND', 'existing_filenames', ff])
    # endforeach()
    if 'existing_filenames'
      # list(['REMOVE_ITEM', 'existing_filenames', nonregression_filenames_used])
    endif
    # keep track of new addition:
    # if we reach here, then a J2K files was added but no test is present in
    # test_suite.ctest.in:
    # foreach(['found_but_no_test', existing_filenames])
    test('NAME', 'Found-But-No-Test-${found_but_no_test}', 'COMMAND', cmake_command, '-E', 'echo', '${found_but_no_test}')
    # set_tests_properties(['Found-But-No-Test-${found_but_no_test}', 'PROPERTIES', 'WILL_FAIL', 'TRUE'])
    # endforeach()
