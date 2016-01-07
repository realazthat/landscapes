#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete the library if it is already in the libs directory.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD






#############################################################################
## get/build mathgeolib
#############################################################################


cd "$PROJECT_PATH"
cd libs

mkdir -p mathgeolib && cd mathgeolib

rm -rf ./mathgeolib/
git clone https://github.com/juj/MathGeoLib.git
cd MathGeoLib
git checkout "915501a"

mkdir -p build
cd build

cmake -G"$CMAKE_GENERATOR" ..
cmake . -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
cmake . -DCMAKE_VERBOSE_MAKEFILE=1
cmake --build .

