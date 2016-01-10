#!/bin/bash

#####################################################################
####Run this from the project root directory
####This script will delete the library if it is already in the libs directory.
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD






#############################################################################
## get/build tclap
#############################################################################


cd "$PROJECT_PATH"
cd libs

mkdir -p tclap && cd tclap

rm -rf ./tclap-code/
git clone git://git.code.sf.net/p/tclap/code tclap-code
cd tclap-code
git checkout "3627d9402e529770df9b0edf2aa8c0e0d6c6bb41"




