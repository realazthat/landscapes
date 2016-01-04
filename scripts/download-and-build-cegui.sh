#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete the library if it is already in the libs directory.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD






#############################################################################
## get/build cegui
#############################################################################


cd "$PROJECT_PATH"
cd libs

mkdir -p cegui && cd cegui

rm -rf ./cegui-source/
hg clone https://bitbucket.org/cegui/cegui cegui-source
cd cegui-source
hg update -C "7109578e2fa792bae3e306d74fcbce0660f583fd"

mkdir -p build
cd build

cmake -G"$CMAKE_GENERATOR" ..
cmake . -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
cmake . -DCMAKE_VERBOSE_MAKEFILE=1
cmake --build .

