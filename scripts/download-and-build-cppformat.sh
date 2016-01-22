#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete the library if it is already in the libs directory.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD







#############################################################################
## get/build cppformat
#############################################################################
cd "$PROJECT_PATH"
cd libs

mkdir -p cppformat && cd cppformat
rm -rf ./cppformat/
git clone https://github.com/cppformat/cppformat.git
cd cppformat
git checkout "4797ca0"

mkdir -p build && cd build
cmake -G"$CMAKE_GENERATOR" ..
cmake . -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
cmake . -DCMAKE_VERBOSE_MAKEFILE=1
cmake --build . --target cppformat


