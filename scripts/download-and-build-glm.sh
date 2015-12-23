#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script assumes that these libraries are already there.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD






#############################################################################
## get/build glm
#############################################################################

#CMAKE_GENERATOR_FLAG='-GMSYS Makefiles'
#CMAKE_GTEST_FLAG='-Dgtest_disable_pthreads=1'
 

cd "$PROJECT_PATH"
cd libs

mkdir -p glm && cd glm

rm -rf ./glm/
git clone https://github.com/g-truc/glm.git
cd glm
git checkout "78f686b"




