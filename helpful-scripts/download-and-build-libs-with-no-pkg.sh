#!/bin/sh

#####################################################################
####Run this from the project root directory
####This script assumes that these libraries are already there.
#####################################################################


#############################################################################
## get/build cNBT
#############################################################################
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
cd ../../..







#############################################################################
## get/build bprinter
#############################################################################

cd libs

mkdir -p bprinter
git clone https://github.com/dattanchu/bprinter.git
git checkout "29531dc"

mkdir build
cd build
cmake ..
cmake --build
cd ../../..


#############################################################################
## get/build cubelib
#############################################################################

cd libs

mkdir -p cubelib
git clone https://github.com/realazthat/cubelib.git


cd ../


#############################################################################
## get/build ThreadPool
#############################################################################

cd libs

mkdir -p threadpool
git clone https://github.com/progschj/ThreadPool.git
git checkout "9a42ec1"

mkdir build
cd build
cmake ..
cmake --build
cd ../../..

