#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete/overwrite any libraries in the ./libs directory
#### that it will download and build.
####
####Usage (on linux):
####  CMAKE_GENERATOR="Unix Makefiles" bash ./scripts/download-and-build-libs-with-no-pkg.sh
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD




#############################################################################
## get/build tclap
#############################################################################
bash ./scripts/download-and-build-tclap.sh






#############################################################################
## get/build bprinter
#############################################################################

cd "$PROJECT_PATH"
cd libs

mkdir -p bprinter && cd bprinter

rm -rf ./bprinter/
git clone https://github.com/dattanchu/bprinter.git
cd bprinter
git checkout "29531dc"

mkdir -p build
cd build
cmake -G"$CMAKE_GENERATOR" ..
cmake . -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
cmake . -DCMAKE_VERBOSE_MAKEFILE=1
cmake . -DUSE_BOOST_KARMA=0
cmake . -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
cmake --build .



#############################################################################
## get/build ThreadPool
#############################################################################

cd "$PROJECT_PATH"
cd libs

mkdir -p ThreadPool && cd ThreadPool
rm -rf ./ThreadPool/
git clone https://github.com/progschj/ThreadPool.git
cd ThreadPool
git checkout "9a42ec1"

