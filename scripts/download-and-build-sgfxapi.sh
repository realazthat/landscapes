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



mkdir -p build
cd build

cmake -G"$CMAKE_GENERATOR" ..
cmake . -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
        -DCMAKE_VERBOSE_MAKEFILE=1 \
        -DGLUT_INCLUDE_DIR="$PROJECT_PATH/libs/freeglut" -DGLUT_LIB_DIR="$PROJECT_PATH/libs/freeglut/" -DGLUT_LIB="freeglut" \
        -DGLFW3_INCLUDE_DIR="$PROJECT_PATH/libs/glfw3/glfw/include" -DGLFW3_LIB_DIR="$PROJECT_PATH/libs/glfw3/glfw/build/src/" \
        -DMGL_INCLUDE_DIR="$PROJECT_PATH/libs/mathgeolib/MathGeoLib/src/" -DGLFW3_LIB_DIR="$PROJECT_PATH/libs/mathgeolib/MathGeoLib/build/" \
        -DCUBELIB_INCLUDE_DIR="$PROJECT_PATH/libs/corner-cases/corner-cases/include"
        
cmake --build . --target sgfxapi
cmake --build . --target sgfxapi-drawutils

