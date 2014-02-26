# UNIT TESTS 

add_executable(testempty1 testempty1.c)
add_executable(testempty2 testempty2.c)
target_link_libraries(testempty1 openjpeg)
target_link_libraries(testempty2 openjpeg)

add_test(NAME testempty1 COMMAND testempty1)
add_test(NAME testempty2 COMMAND testempty2)
