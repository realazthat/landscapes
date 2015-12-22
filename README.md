landscapes
---

Sparse Voxel Octree implementation. Based on the paper
[Efficient Sparse Voxel Octrees – Analysis, Extensions, and Implementation](https://mediatech.aalto.fi/~samuli/publications/laine2010tr1_paper.pdf)
by Samuli Laine and Tero Karras of NVIDIA Research.


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

Libraries:

* To build core landscapes:
    * cubelib
    * [glm](http://glm.g-truc.net/)
        * High-perf math library, useful for vectorized ops required for graphics, and also useful for
            opencl-compatible shim/layer
        * "[the Happy Bunny License (Modified MIT) or the MIT License](http://glm.g-truc.net/copying.txt)"
    * [pempek-assert](https://github.com/gpakosz/Assert) (included in the project)
        * Asserts with formatted error messages
        * `released under the WTFPL v2 license, by Gregory Pakosz (@gpakosz)`
    * [bprinter](https://github.com/dattanchu/bprinter/wiki)
        * Pretty-printed tables, useful for debugging
        * BSD License
    * [cppformat](https://github.com/cppformat/cppformat)
        * Formatting strings for C++, mostly useful for debugging, error messages etc.
        * BSD License
        


* To build landscapes-mc
    * landscapes::svo
    * [cNBT](https://github.com/FliPPeh/cNBT)
        * `THE BEER-WARE LICENSE`
    * libz
    * [ThreadPool](https://github.com/progschj/ThreadPool)
        * [zlib License](https://github.com/progschj/ThreadPool/blob/master/COPYING)

* For project landscapes-mc-demo
    * landscapes-mc
    * gfxapi
        * MathGeoLib
        * glut
        * glu
        * glfw
        * cubelib



