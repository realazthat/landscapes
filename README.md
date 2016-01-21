landscapes
---

Sparse Voxel Octree implementation. Based on the paper
[Efficient Sparse Voxel Octrees – Analysis, Extensions, and Implementation](https://mediatech.aalto.fi/~samuli/publications/laine2010tr1_paper.pdf)
by Samuli Laine and Tero Karras of NVIDIA Research.



Branch  | Status
---     | ---
master  | [![Build Status](https://travis-ci.org/realazthat/landscapes.svg?branch=master)](https://travis-ci.org/realazthat/landscapes)
develop | [![Build Status](https://travis-ci.org/realazthat/landscapes.svg?branch=develop)](https://travis-ci.org/realazthat/landscapes)


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

Requires C++14 compatible compiler. Tested with:

* windows/msys2/mingw-w64/gcc (windows 7)
* linux/gcc (ubuntu)


Build system:

* CMake >= 3.1 (if using CMake)
* git (only if building dependencies from source)
* mercurial (only if building dependencies from source)

Libraries:

* To build core landscapes:
    * [corner-cases/cubelib](https://github.com/realazthat/corner-cases)
        * A library for reasoning about a cube, the faces, corners, and edges of a cube.
        * Tested with commit [e1f6a4d](https://github.com/realazthat/corner-cases/commit/e1f6a4d61fc31a674588225fd06569492b007b44)
            ([browse files](https://github.com/realazthat/corner-cases/tree/e1f6a4d61fc31a674588225fd06569492b007b44))
        * [MIT License](https://opensource.org/licenses/MIT)
    * [glm](http://glm.g-truc.net/)
        * High-perf math library, useful for vectorized ops required for graphics, and also useful for
            opencl-compatible shim/layer
        * Tested with commit [78f686b](https://github.com/g-truc/glm/commit/78f686b4be6c623df829db58b974bf8d79461987)
            ([browse files](https://github.com/g-truc/glm/tree/78f686b4be6c623df829db58b974bf8d79461987))
        * "[the Happy Bunny License (Modified MIT) or the MIT License](http://glm.g-truc.net/copying.txt)"
    * [bprinter](https://github.com/dattanchu/bprinter/wiki)
        * Pretty-printed tables, useful for debugging
        * Tested with commit [29531dc](https://github.com/dattanchu/bprinter/commit/29531dcecceb99d759a196f5e44b4729abe18bed)
            ([browse files](https://github.com/dattanchu/bprinter/tree/29531dcecceb99d759a196f5e44b4729abe18bed))
        * BSD License
    * [cppformat](https://github.com/cppformat/cppformat)
        * Formatting strings for C++, mostly useful for debugging, error messages etc.
        * Tested with commit [4797ca0](https://github.com/cppformat/cppformat/commit/4797ca025eef17b8df42edd8c9bde83c43806bf1)
            ([browse files](https://github.com/cppformat/cppformat/tree/4797ca025eef17b8df42edd8c9bde83c43806bf1))
        * BSD License
        


* To build landscapes-mc
    * landscapes::svo
    * [cNBT](https://github.com/FliPPeh/cNBT)
        * Tested with commit [a3a42c9](https://github.com/FliPPeh/cNBT/commit/a3a42c94eb5eb774987edf3121d4d6bc6781981f)
            ([browse files](https://github.com/FliPPeh/cNBT/tree/a3a42c94eb5eb774987edf3121d4d6bc6781981f))
        * `THE BEER-WARE LICENSE`
    * libz
    * [ThreadPool](https://github.com/progschj/ThreadPool)
        * Tested with commit [9a42ec1](https://github.com/progschj/ThreadPool/commit/9a42ec1329f259a5f4881a291db1dcb8f2ad9040)
            ([browse files](https://github.com/progschj/ThreadPool/tree/9a42ec1329f259a5f4881a291db1dcb8f2ad9040))
        * [zlib License](https://github.com/progschj/ThreadPool/blob/master/COPYING)

* For project landscapes-mc-demo
    * landscapes-mc
    * gfxapi
        * MathGeoLib
        * glut/freeglut
            * Tested with freeglut ([git mirror](https://github.com/dcnieho/FreeGLUT))
                * Tested with commit [575feb5](https://github.com/dcnieho/FreeGLUT/commit/575feb52b0842469d434d815bc8adb241df30879)
                    ([browse files](https://github.com/dcnieho/FreeGLUT/tree/575feb52b0842469d434d815bc8adb241df30879))
                * [X Consortium License](https://github.com/dcnieho/FreeGLUT/blob/git_master/freeglut/freeglut/COPYING) (variant of the MIT License with a promotion clause)
        * glu
        * glfw
            * Tested with tag [3.1.2](https://github.com/glfw/glfw/tree/3.1.2)
            * [zlib License](https://github.com/glfw/glfw/blob/3.1.2/COPYING.txt)
        * corner-cases/cubelib
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

    #install dependencies, there are some bash scripts provided in the ./scripts/ directory
    # that will download and build many of the dependencies and put them in the ./libs directory
    # the scripts are meant for the continuous integration system, but you can run them yourself
    # or read them for assistance.
    
    #optionally install or download and build googletest (only matters if you are going to run the unittests)
    #note, choose the appropriate generator
    #see the bash scripts for for more details
    CMAKE_GENERATOR="MSYS Makefiles" CMAKE_BUILD_TYPE="Debug" bash ./scripts/download-and-build-googletest.sh
    #... etc. install dependencies


    #make a build directory
    mkdir -p build && cd build

    
    #note, choose the appropriate generator
    cmake -G"MSYS Makefiles" .. -DCMAKE_BUILD_TYPE="Debug"
    
    #if you built the dependencies and put them in the libs directory, then you are good to build
    
    #if you installed the dependencies to the system, you are prolly good to build
    
    #if you built the dependencies yourself outside the expected ./libs subdirectories, then you will need to
    # define/override the paths to the projects (which by default point to the ./libs directory)
    # for example, like so:
    cmake -L # list all the user-definable variables
    cmake . -DGLFW3_INCLUDE_DIR=/path/to/cppformat/include -DGLFW3_LIB=glfw3 -DGLFW3_LIB_DIR=/path/to/glfw/build/src
    # .. and so on for each dependency that is not in the ./libs directory and not installed in the system
    #alternatively, we can set these via the GUI
    cmake-gui .
    
    #build the targets you want to build
    cmake --build . --target landscapes
    cmake --build . --target landscapes-mc
    cmake --build . --target landscapes-mc-demo
    cmake --build . --target landscapes-mc-zorder-genconsts
    cmake --build . --target unittests
    # ... etc.

    #execute the unittests if you want to
    ./unittests

    #see ./scripts/rebuild.it.all.for.msys.sh for how the integration tests do it

```
