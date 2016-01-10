#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete the library if it is already in the libs directory.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD






#############################################################################
## get/build glm
#############################################################################


cd "$PROJECT_PATH"
cd libs

mkdir -p glm && cd glm

rm -rf ./glm/
git clone https://github.com/g-truc/glm.git
cd glm
git checkout "78f686b"




