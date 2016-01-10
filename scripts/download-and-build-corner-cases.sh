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
## get/build corner-cases/cubelib
#############################################################################

cd "$PROJECT_PATH"
cd libs

mkdir -p corner-cases && cd corner-cases
rm -rf ./corner-cases/
git clone https://github.com/realazthat/corner-cases.git




