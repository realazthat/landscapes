#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete/overwrite any data in the ./libs directory
#### that it will download and build.
#####################################################################

#exit on failed line
set -exv

PROJECT_PATH=$PWD



#############################################################################
## get/build mca region(s) for testing
#############################################################################

#see https://github.com/twoolie/NBT/wiki/Tests


cd "$PROJECT_PATH"
cd libs

mkdir -p ./test-region/

mkdir -p NBT-Sample_World && cd NBT-Sample_World
rm -rf ./Sample_World.tar.gz
rm -rf ./Sample World/
wget https://github.com/downloads/twoolie/NBT/Sample_World.tar.gz
tar xf Sample_World.tar.gz

cd "Sample World"

cd region

cp "r.0.0.mca" "$PROJECT_PATH/libs/test-region/."

