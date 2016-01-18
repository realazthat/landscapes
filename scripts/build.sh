#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete the library if it is already in the libs directory.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD






#############################################################################
## get/build landscapes
#############################################################################


cd "$PROJECT_PATH"

mkdir -p build && cd build
cmake -G "$CMAKE_GENERATOR" .. "-DGTEST_LIB=gtest;pthread" -DCMAKE_VERBOSE_MAKEFILE=1 -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
cmake --build . --target unittests
cmake --build . --target landscapes-zorder-genconsts
./unittests

