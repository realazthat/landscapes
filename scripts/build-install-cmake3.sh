#!/bin/bash

#####################################################################
####Run this with sudo
#####################################################################


#exit on failed line
set -exv


if [ "$(cmake --version | grep 'cmake version 3.3.2')" = "cmake version 3.3.2" ]; then
    echo "CMake is the proper version"
    exit 0
else
    echo "Invalid cmake version"
fi

PROJECT_PATH=$PWD


cd "$PROJECT_PATH"
cd ./libs
mkdir -p cmake3 && cd cmake3

rm -rf cmake-3.3.2.tar.gz
rm -rf ./cmake-3.3.2/
wget https://cmake.org/files/v3.3/cmake-3.3.2.tar.gz

tar xf cmake-3.3.2.tar.gz

cd cmake-3.3.2
./bootstrap
make
make install

if [ "$(cmake --version | grep 'cmake version 3.3.2')" = "cmake version 3.3.2" ]; then
    echo "CMake is the proper version"
else
    echo "Invalid cmake version"
    exit -1
fi

