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
hg update v0-8
#hg update -C "7109578e2fa792bae3e306d74fcbce0660f583fd"

mkdir -p build
cd build

cmake -G"$CMAKE_GENERATOR" .. \
        -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" -DCMAKE_VERBOSE_MAKEFILE=1 \
        -DCEGUI_SAMPLES_ENABLED=0 -DCEGUI_BUILD_RENDERER_DIRECT3D11=0 \
        -DCEGUI_BUILD_RENDERER_DIRECT3D10=0 -DCEGUI_BUILD_RENDERER_DIRECT3D9=0 -DCEGUI_BUILD_RENDERER_OPENGL=1 \
        -DCEGUI_BUILD_RENDERER_OPENGL3=1 -DCEGUI_BUILD_APPLICATION_TEMPLATES=0 -DCEGUI_BUILD_PYTHON_MODULES=0 \
        -DGLM_H_PATH="$PROJECT_PATH/libs/glm/glm/" \
        -DCMAKE_PREFIX_PATH:PATH="$PROJECT_PATH/libs/cegui/cegui-dependencies/build/dependencies"
cmake --build .

#create the directory in case it does not exist; when deps are built statically it might not exist
mkdir -p ./bin/ && touch ./bin/ignorethisfile
cp -f ./bin/* "$PROJECT_PATH/build/."
