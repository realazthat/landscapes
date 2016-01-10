#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete/overwrite any data in the ./libs directory
#### that it will download and build.
#####################################################################


#exit on failed line
set -exv


PROJECT_PATH=$PWD

#clear the build directory
rm -rf ./build
mkdir -p build && cd build

cd "$PROJECT_PATH"

CMAKE_DEPS_BUILD_TYPE="Release"
CMAKE_BUILD_TYPE="Debug"
CMAKE_GENERATOR="MSYS Makefiles"

CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-and-build-googletest.sh



CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-and-build-glm.sh
#CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-and-build-cegui-dependencies.sh
#CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-and-build-cegui.sh


#CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-and-build-tclap.sh
CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-and-build-cnbt.sh
CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-and-build-corner-cases.sh
#CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-and-build-glfw3.sh
#CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-and-build-freeglut.sh
CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-and-build-mathgeolib.sh
#CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" bash ./scripts/download-and-build-sgfxapi.sh

CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-and-build-libs-with-no-pkg.sh
CMAKE_GENERATOR="$CMAKE_GENERATOR" CMAKE_BUILD_TYPE="$CMAKE_DEPS_BUILD_TYPE" bash ./scripts/download-unittest-data.sh

cd "$PROJECT_PATH/build"

cmake -G"$CMAKE_GENERATOR" ..
cmake . -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
cmake . -DCMAKE_VERBOSE_MAKEFILE=1
cmake --build . --target unittests

./unittests
