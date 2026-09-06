# UNIT TESTS

include_directories(
  ${OPENJPEG_BINARY_DIR}/src/lib/openjp2 # opj_config.h
  ${OPENJPEG_SOURCE_DIR}/src/lib/openjp2
)

set(unit_test
  testempty0
  testempty1
  testempty2
)
foreach(ut ${unit_test})
  add_executable(${ut} ${ut}.c)
  target_link_libraries(${ut} openjp2)
  add_test(NAME ${ut} COMMAND ${ut})
endforeach()

add_executable(testjp2 testjp2.c)
target_link_libraries(testjp2 openjp2)
add_test(NAME testjp2 COMMAND testjp2 ${OPJ_DATA_ROOT})

if(BUILD_JPIP)
  add_executable(testjpipvbas testjpipvbas.c)
  target_include_directories(testjpipvbas PRIVATE
    ${OPENJPEG_SOURCE_DIR}/src/lib/openjpip)
  target_link_libraries(testjpipvbas openjpip)
  add_test(NAME testjpipvbas COMMAND testjpipvbas)
endif()
