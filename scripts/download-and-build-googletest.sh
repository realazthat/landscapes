#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete the library if it is already in the libs directory.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD






#############################################################################
## get/build googletest
#############################################################################


cd "$PROJECT_PATH"
cd libs

mkdir -p googletest && cd googletest

rm -rf ./googletest/
git clone https://github.com/google/googletest.git
cd googletest
git checkout "ddb8012"
cd googletest

mkdir -p build
cd build
cmake -G"$CMAKE_GENERATOR" ..
cmake . -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
cmake . -DCMAKE_VERBOSE_MAKEFILE=1
if [ $(echo "$MSYSTEM" | grep -io MINGW) = "MINGW" ]; then
    cmake . -Dgtest_disable_pthreads=1
else
fi
cmake --build .

