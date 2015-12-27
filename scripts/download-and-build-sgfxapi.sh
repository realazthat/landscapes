#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete the library if it is already in the libs directory.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD






#############################################################################
## get/build sgfxapi
#############################################################################


cd "$PROJECT_PATH"
cd libs

mkdir -p sgfxapi && cd sgfxapi

rm -rf ./sgfxapi/
git clone https://github.com/realazthat/sgfxapi.git
cd sgfxapi
git checkout "ddb8012"

mkdir -p build
cd build

cmake -G"$CMAKE_GENERATOR" ..
cmake . -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
cmake --build .

