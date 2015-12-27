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
## get/build cubelib
#############################################################################

cd "$PROJECT_PATH"
bash ./scripts/download-and-build-cubelib.sh

#############################################################################
## get/build glm
#############################################################################

cd "$PROJECT_PATH"
bash ./scripts/download-and-build-glm.sh


#############################################################################
## get/build tclap
#############################################################################

cd "$PROJECT_PATH"
bash ./scripts/download-and-build-tclap.sh

#############################################################################
## get/build sgfxapi
#############################################################################

cd "$PROJECT_PATH"
bash ./scripts/download-and-build-sgfxapi.sh



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
cmake --build .







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
cmake . -DUSE_BOOST_KARMA=0
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



#############################################################################
## get/build cppformat
#############################################################################
cd "$PROJECT_PATH"
cd libs

mkdir -p cppformat && cd cppformat
rm -rf ./cppformat/
git clone https://github.com/cppformat/cppformat.git
cd cppformat
git checkout "4797ca0"

mkdir -p build && cd build
cmake -G"$CMAKE_GENERATOR" ..
cmake --build . --target cppformat

