    # UNIT TESTS
    # include_directories([openjpeg_binary_dir, '/src/lib/openjp2', openjpeg_source_dir, '/src/lib/openjp2'])
    unit_test = ['testempty0', 'testempty1', 'testempty2']
    # foreach(['ut', unit_test])
    ut_exe = executable(ut, ut, '.c')
    # target_link_libraries([ut, 'openjp2'])
    test('NAME', ut, 'COMMAND', ut)
    # endforeach()
