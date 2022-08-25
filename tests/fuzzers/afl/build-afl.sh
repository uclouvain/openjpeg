#/bin/sh
#
# this creates builds which can be used to fuzz with afl
#
# by Paul Dreik 20220825

set -eux

here=$(dirname $0)
gitroot=$(git -C $here rev-parse --show-toplevel)


###################################
# afl clang
export AFL_USE_ASAN=1
export AFL_USE_UBSAN=1

target=$here/build-afl-clang

cmake \
-DCMAKE_C_COMPILER=afl-clang-fast \
-S $gitroot -B $target

cmake --build $target -j $(nproc)

###################################
# afl clang, with asserts disabled

target=$here/build-afl-clang-ndebug

cmake \
-DCMAKE_C_COMPILER=afl-clang-fast \
-DCMAKE_C_FLAGS="-g -DNDEBUG" \
-S $gitroot -B $target

cmake --build $target  -j $(nproc)

###################################
# sanitizer build with asserts disabled
target=$here/build-clang-release-replay
cmake \
-DCMAKE_C_COMPILER=clang-14 \
-DCMAKE_C_FLAGS="-g -fsanitize=address,undefined -O3 -DNDEBUG" \
-S $gitroot -B $target

cmake --build $target -j $(nproc)

###################################
# sanitizer build with asserts enabled
target=$here/build-clang-debug-replay
cmake \
-DCMAKE_C_COMPILER=clang-14 \
-DCMAKE_C_FLAGS="-g -fsanitize=address,undefined -O3" \
-S $gitroot -B $target

cmake --build $target -j $(nproc)
