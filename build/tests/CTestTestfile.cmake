# CMake generated Testfile for 
# Source directory: /media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/tests
# Build directory: /media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(tte0 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/test_tile_encoder")
add_test(tte1 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/test_tile_encoder" "3" "2048" "2048" "1024" "1024" "8" "1" "tte1.j2k")
add_test(tte2 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/test_tile_encoder" "3" "2048" "2048" "1024" "1024" "8" "1" "tte2.jp2")
add_test(tte3 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/test_tile_encoder" "1" "2048" "2048" "1024" "1024" "8" "1" "tte3.j2k")
add_test(tte4 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/test_tile_encoder" "1" "256" "256" "128" "128" "8" "0" "tte4.j2k")
add_test(tte5 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/test_tile_encoder" "1" "512" "512" "256" "256" "8" "0" "tte5.j2k")
add_test(ttd0 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/test_tile_decoder")
set_tests_properties(ttd0 PROPERTIES  DEPENDS "tte0")
add_test(ttd1 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/test_tile_decoder" "0" "0" "1024" "1024" "tte1.j2k")
set_tests_properties(ttd1 PROPERTIES  DEPENDS "tte1")
add_test(ttd2 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/test_tile_decoder" "0" "0" "1024" "1024" "tte2.jp2")
set_tests_properties(ttd2 PROPERTIES  DEPENDS "tte2")
add_test(rta1 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/j2k_random_tile_access" "tte1.j2k")
set_tests_properties(rta1 PROPERTIES  DEPENDS "tte1")
add_test(rta2 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/j2k_random_tile_access" "tte2.jp2")
set_tests_properties(rta2 PROPERTIES  DEPENDS "tte2")
add_test(rta3 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/j2k_random_tile_access" "tte3.j2k")
set_tests_properties(rta3 PROPERTIES  DEPENDS "tte3")
add_test(rta4 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/j2k_random_tile_access" "tte4.j2k")
set_tests_properties(rta4 PROPERTIES  DEPENDS "tte4")
add_test(rta5 "/media/jiapei/Data/Downloads/imageprocessing/codecs/openjpeg/build/bin/j2k_random_tile_access" "tte5.j2k")
set_tests_properties(rta5 PROPERTIES  DEPENDS "tte5")
subdirs(conformance)
subdirs(nonregression)
subdirs(unit)
