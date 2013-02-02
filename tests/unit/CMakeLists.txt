# UNIT TESTS 

include_directories(
  ${OPENJPEG_BINARY_DIR}/src/lib/openjp2 # opj_config.h
  ${OPENJPEG_SOURCE_DIR}/src/lib/openjp2
)

add_executable(testempty0 testempty0.c)
add_executable(testempty1 testempty1.c)
add_executable(testempty2 testempty2.c)

target_link_libraries(testempty0 openjp2)
target_link_libraries(testempty1 openjp2)
target_link_libraries(testempty2 openjp2)

add_test(testempty0 ${EXECUTABLE_OUTPUT_PATH}/testempty0)
add_test(testempty1 ${EXECUTABLE_OUTPUT_PATH}/testempty1)
add_test(testempty2 ${EXECUTABLE_OUTPUT_PATH}/testempty2)
