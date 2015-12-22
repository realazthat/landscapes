#!/bin/bash

#####################################################################
####Run this with sudo
#####################################################################


#exit on failed line
set -exv

apt-get update
apt-get install libglm-dev
apt-get install libgtest-dev


