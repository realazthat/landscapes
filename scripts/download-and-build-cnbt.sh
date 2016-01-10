#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete the library if it is already in the libs directory.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD






#############################################################################
## get/build cNBT
#############################################################################
cd "$PROJECT_PATH"
cd libs
mkdir -p cNBT && cd cNBT

rm -rf ./cNBT/
#retreive the library
git clone https://github.com/FliPPeh/cNBT.git
cd cNBT
git checkout "5850e3e"

mkdir -p build && cd build
cmake -G"$CMAKE_GENERATOR" ..
cmake . -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
cmake . -DCMAKE_VERBOSE_MAKEFILE=1
cmake --build .


