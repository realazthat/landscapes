landscapes
---

Sparse Voxel Octree implementation. Based on the paper
[Efficient Sparse Voxel Octrees – Analysis, Extensions, and Implementation](https://mediatech.aalto.fi/~samuli/publications/laine2010tr1_paper.pdf)
by Samuli Laine and Tero Karras of NVIDIA Research.

[![Build Status](https://travis-ci.org/realazthat/landscapes.svg?branch=master)](https://travis-ci.org/realazthat/landscapes)


{Long Description}





License
----

Libraries have their own license.

Some specific files might have their own license included in them; in this case those files fall under
that license. Some directories might have their own license; in that case those directories will have
a README, LICENSE, COPYING or similar file related to licensing.

As for the rest of the project, code written by me in this project is released under the
[MIT License](https://opensource.org/licenses/MIT).



Dependencies
----

Build system:

* CMake >= 3.1 (if using CMake)
* git (only if building dependencies from source)
* mercurial (only if building dependencies from source)

Libraries:

* To build core landscapes:
    * cubelib
    * [glm](http://glm.g-truc.net/)
        * High-perf math library, useful for vectorized ops required for graphics, and also useful for
            opencl-compatible shim/layer
        * Tested with commit [78f686b](https://github.com/g-truc/glm/tree/78f686b4be6c623df829db58b974bf8d79461987)
        * "[the Happy Bunny License (Modified MIT) or the MIT License](http://glm.g-truc.net/copying.txt)"
    * [bprinter](https://github.com/dattanchu/bprinter/wiki)
        * Pretty-printed tables, useful for debugging
        * Tested with commit [29531dc](https://github.com/dattanchu/bprinter/tree/29531dcecceb99d759a196f5e44b4729abe18bed)
        * BSD License
    * [cppformat](https://github.com/cppformat/cppformat)
        * Formatting strings for C++, mostly useful for debugging, error messages etc.
        * Tested with commit [4797ca0](https://github.com/cppformat/cppformat/tree/4797ca025eef17b8df42edd8c9bde83c43806bf1)
        * BSD License
        


* To build landscapes-mc
    * landscapes::svo
    * [cNBT](https://github.com/FliPPeh/cNBT)
        * Tested with commit [5850e3e](https://github.com/FliPPeh/cNBT/tree/5850e3eace0e07c73a1a370abd69b7448da702d6)
        * `THE BEER-WARE LICENSE`
    * libz
    * [ThreadPool](https://github.com/progschj/ThreadPool)
        * Tested with commit [9a42ec1](https://github.com/progschj/ThreadPool/tree/9a42ec1329f259a5f4881a291db1dcb8f2ad9040)
        * [zlib License](https://github.com/progschj/ThreadPool/blob/master/COPYING)

* For project landscapes-mc-demo
    * landscapes-mc
    * gfxapi
        * MathGeoLib
        * glut
        * glu
        * glfw
        * cubelib
    * [xdsopl/sma](https://github.com/xdsopl/sma/tree/master) (modified/included in project at `<project-root>/include/xdsopl-sma`)
        * Simple Moving Average
        * License: [CC0 Public Domain Dedication ](http://creativecommons.org/publicdomain/zero/1.0/)
    * [tclap](http://tclap.sourceforge.net/)
        * Command-line argument parser
        * [MIT License](http://opensource.org/licenses/mit-license.php)



Building
---





```

    git clone https://github.com/realazthat/landscapes.git
    cd landscapes


    #optionally download and build googletest (only matters if you are going to run the unittests)
    #note, choose the appropriate generator
    CMAKE_GENERATOR="MSYS Makefiles" bash ./scripts/download-and-build-googletest.sh

    #optionally download and build dependencies
    #note, choose the appropriate generator
    CMAKE_GENERATOR="MSYS Makefiles" bash ./scripts/download-and-build-libs-with-no-pkg.sh



    rm -rf ./build
    mkdir -p build && cd build

    cmake -G"MSYS Makefiles" ..
    cmake --build . --target landscapes
    cmake --build . --target landscapes-mc
    # ... etc.




    #optionally build and execut the unittests
    cmake --build . --target unittests
    ./unittests


```
