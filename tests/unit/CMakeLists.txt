# UNIT TESTS 

add_executable(testempty1 testempty1.c)
add_executable(testempty2 testempty2.c)
target_link_libraries(testempty1 openjpeg)
target_link_libraries(testempty2 openjpeg)

add_test(testempty1 ${EXECUTABLE_OUTPUT_PATH}/testempty1)
add_test(testempty2 ${EXECUTABLE_OUTPUT_PATH}/testempty2)
