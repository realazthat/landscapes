#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete/overwrite any libraries in the ./libs directory
#### that it will download and build.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD

#############################################################################
## get/build cNBT
#############################################################################
cd $PROJECT_PATH
cd libs
mkdir -p cNBT && cd cNBT

rm -rf ./cNBT/
#retreive the library
git clone https://github.com/FliPPeh/cNBT.git
cd cNBT
git checkout "5850e3e"

mkdir -p build && cd build
cmake ..
cmake --build .







#############################################################################
## get/build bprinter
#############################################################################

cd $PROJECT_PATH
cd libs

mkdir -p bprinter && cd bprinter

rm -rf ./bprinter/
git clone https://github.com/dattanchu/bprinter.git
cd bprinter
git checkout "29531dc"

mkdir -p build
cd build
cmake ..
cmake --build .


#############################################################################
## get/build cubelib
#############################################################################

cd $PROJECT_PATH
bash ./scripts/download-and-build-cubelib.sh


#############################################################################
## get/build ThreadPool
#############################################################################

cd $PROJECT_PATH
cd libs

mkdir -p ThreadPool && cd ThreadPool
rm -rf ./ThreadPool/
git clone https://github.com/progschj/ThreadPool.git
cd ThreadPool
git checkout "9a42ec1"



#############################################################################
## get/build cppformat
#############################################################################
cd $PROJECT_PATH
cd libs

mkdir -p cppformat && cd cppformat
rm -rf ./cppformat/
git clone https://github.com/cppformat/cppformat.git
cd cppformat
git checkout "4797ca0"

mkdir -p build && cd build
cmake ..
cmake --build .

