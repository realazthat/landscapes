#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete the library if it is already in the libs directory.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD






#############################################################################
## get/build glfw3
#############################################################################


cd "$PROJECT_PATH"
cd libs

mkdir -p glfw3 && cd glfw3

rm -rf ./glfw/
git clone https://github.com/glfw/glfw.git
cd glfw
git checkout "3.1.2"

mkdir -p build
cd build
cmake -G"$CMAKE_GENERATOR" .. \
        -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" -DCMAKE_VERBOSE_MAKEFILE=1 -DBUILD_SHARED_LIBS=1 \
        -DGLFW_BUILD_TESTS=0 -DGLFW_BUILD_DOCS=0 -DGLFW_BUILD_EXAMPLES=0 -DGLFW_INSTALL=0

cmake --build . --target glfw


