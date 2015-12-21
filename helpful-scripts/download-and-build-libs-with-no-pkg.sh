#!/bin/sh

#####################################################################
####Run this from the project root directory
####This script assumes that these libraries are already there.
#####################################################################


PROJECT_PATH=$PWD

#############################################################################
## get/build cNBT
#############################################################################
cd $PROJECT_PATH
cd libs
mkdir -p cNBT

#retreive the library
git clone https://github.com/FliPPeh/cNBT.git
git checkout "5850e3e"

cd cNBT
mkdir -p build
cd build
cmake ..
cmake --build







#############################################################################
## get/build bprinter
#############################################################################

cd $PROJECT_PATH
cd libs

mkdir -p bprinter
git clone https://github.com/dattanchu/bprinter.git
git checkout "29531dc"

mkdir -p build
cd build
cmake ..
cmake --build


#############################################################################
## get/build cubelib
#############################################################################

cd $PROJECT_PATH
cd libs

mkdir -p cubelib
git clone https://github.com/realazthat/cubelib.git




#############################################################################
## get/build ThreadPool
#############################################################################

cd $PROJECT_PATH
cd libs

mkdir -p threadpool
git clone https://github.com/progschj/ThreadPool.git
cd ThreadPool
git checkout "9a42ec1"

mkdir -p build
cd build
cmake ..
cmake --build
cd $PROJECT_PATH


#############################################################################
## get/build cppformat
#############################################################################
cd $PROJECT_PATH
cd libs

mkdir -p cppformat
git clone https://github.com/cppformat/cppformat.git
git checkout "4797ca0"

mkdir -p build
cd build
cmake --build

