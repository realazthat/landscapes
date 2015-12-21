#!/bin/sh

#####################################################################
####Run this with sudo
#####################################################################


#exit on failed line
set -exv

apt-get update
apt-get install libglm-dev
apt-get install libcppformat1-dev
apt-get install libgtest0 libgtest-dev


