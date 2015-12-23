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
## get/build cubelib
#############################################################################

cd "$PROJECT_PATH"
cd libs

mkdir -p cubelib && cd cubelib
rm -rf ./cubelib/
git clone https://github.com/realazthat/cubelib.git




