#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete/overwrite any data in the ./libs directory
#### that it will download and build.
#####################################################################





CMAKE_GENERATOR="MSYS Makefiles" bash ./scripts/download-and-build-googletest.sh

CMAKE_GENERATOR="MSYS Makefiles" bash ./scripts/download-and-build-glm.sh

CMAKE_GENERATOR="MSYS Makefiles" bash ./scripts/download-and-build-libs-with-no-pkg.sh

CMAKE_GENERATOR="MSYS Makefiles" bash ./scripts/download-unittest-data.sh

rm -rf ./build
mkdir -p build && cd build

cmake -G"MSYS Makefiles" ..
cmake --build .

./unittests
