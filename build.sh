#!/bin/sh
mkdir -p build
(cd build && cmake ..)
(cd build && make VERBOSE=1 -j ${nprocs})