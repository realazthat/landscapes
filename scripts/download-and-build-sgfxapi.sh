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

#build dependencies of sgfxapi
CMAKE_BUILD_TYPE="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" bash scripts/download-and-build-libs-with-no-pkg.sh


mkdir -p build
cd build

cmake -G"$CMAKE_GENERATOR" ..
cmake . -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
cmake . -DGLUT_LIB="freeglut"
cmake --build . --target sgfxapi
cmake --build . --target sgfxapi-drawutils

