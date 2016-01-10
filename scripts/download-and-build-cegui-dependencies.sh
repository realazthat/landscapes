#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete the library if it is already in the libs directory.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD






#############################################################################
## get/build cegui-dependencies
#############################################################################


cd "$PROJECT_PATH"
cd libs

mkdir -p cegui && cd cegui


rm -rf ./cegui-dependencies
hg clone https://bitbucket.org/cegui/cegui-dependencies cegui-dependencies
cd cegui-dependencies
hg update v0-8


#if [ $(echo "$MSYSTEM" | grep -io MINGW) = "MINGW" ]; then

    #hg update mingw

#fi


mkdir -p build
cd build

cmake -G"$CMAKE_GENERATOR" .. \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCMAKE_VERBOSE_MAKEFILE=1 -DCEGUI_BUILD_FREEIMAGE=1 -DCEGUI_BUILD_FREETYPE2=1 -DCEGUI_BUILD_SILLY=1 \
    -DCEGUI_BUILD_EXPAT=0 -DCEGUI_BUILD_TINYXML=1 \
    -DCEGUI_BUILD_GLFW=0 -DCEGUI_BUILD_GLM=0 -DCEGUI_BUILD_ZLIB=1 -DCEGUI_BUILD_SUFFIX='' -DCMAKE_DEBUG_POSTFIX=''
    
cmake --build .

cp -f "./dependencies/bin/"/* "$PROJECT_PATH/build/."

