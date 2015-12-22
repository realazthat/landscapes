
#!/bin/sh

#####################################################################
####Run this with sudo
#####################################################################


#exit on failed line
set -exv

PROJECT_PATH=$PWD

if [ "$(cmake --version | grep 'cmake version 3.3.2')" = "cmake version 3.3.2" ]; then
    echo "CMake is the proper version"
    exit 0
else
    echo "Invalid cmake version"
fi


cd ./libs
mkdir -p cmake3 && cd cmake3

rm -rf cmake-3.2.2-Linux-x86_64.tar.gz
rm -rf ./cmake-3.2.2-Linux-x86_64/
wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-x86_64.tar.gz

tar xf cmake-3.2.2-Linux-x86_64.tar.gz

sudo cp -r cmake-3.2.2-Linux-x86_64/* /usr


if [ "$(cmake --version | grep 'cmake version 3.3.2')" = "cmake version 3.3.2" ]; then
    echo "CMake is the proper version"
else
    echo "Invalid cmake version"
    exit -1
fi







