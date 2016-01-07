cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(landscapes)

#relative link directories are to be turned into full paths
cmake_policy(SET CMP0015 NEW)


set(GLM_INCLUDE_DIR "./libs/glm/glm"
    CACHE STRING "Paths to glm includes")
set(GLM_LIB_DIR ""
    CACHE STRING "Paths to glm libs")
set(GLM_LIB ""
    CACHE STRING "Libs for glm")


set(CEGUI_INCLUDE_DIR "./libs/cegui/cegui-source/cegui/include;./libs/cegui/cegui-source/build/cegui/include"
    CACHE STRING "Paths to CEGUI includes")
set(CEGUI_LIB_DIR "./libs/cegui/cegui-source/build/lib"
    CACHE STRING "Paths to CEGUI libs")
set(CEGUI_LIB "CEGUIBase-0;CEGUIOpenGLRenderer-0;CEGUIExpatParser;CEGUICoreWindowRendererSet;CEGUICommonDialogs-0"
    CACHE STRING "Libs for CEGUI")



set(MGL_INCLUDE_DIR "./libs/mathgeolib/MathGeoLib/src"
    CACHE STRING "Paths to MathGeoLib includes")
set(MGL_LIB_DIR "./libs/mathgeolib/MathGeoLib/build"
    CACHE STRING "Paths to MathGeoLib libs")
set(MGL_LIB "MathGeoLib"
    CACHE STRING "Libs for MathGeoLib")



set(GLFW3_INCLUDE_DIR "./libs/glfw3/glfw/include"
    CACHE STRING "Paths to glfw3 includes")
set(GLFW3_LIB_DIR "./libs/glfw3/glfw/build"
    CACHE STRING "Paths to glfw3 libs")
set(GLFW3_LIB "glfw3"
    CACHE STRING "Libs for glfw3")

set(CPPFORMAT_INCLUDE_DIR "./libs/cppformat/cppformat"
    CACHE STRING "Paths to cppformat includes")
set(CPPFORMAT_LIB_DIR "./libs/cppformat/cppformat/build"
    CACHE STRING "Paths to cppformat libs")
set(CPPFORMAT_LIB "cppformat"
    CACHE STRING "Libs for cppformat")

set(CNBT_INCLUDE_DIR "./libs/cNBT/cNBT/"
    CACHE STRING "Paths to cNBT includes")
set(CNBT_LIB_DIR "./libs/cNBT/cNBT/build"
    CACHE STRING "Paths to cNBT libs")
set(CNBT_LIB "nbt"
    CACHE STRING "Libs for cNBT")

set(CUBELIB_INCLUDE_DIR "./libs/corner-cases/corner-cases/include" CACHE STRING "Paths to corner-cases/cubelib includes")
set(CUBELIB_LIB_DIR "" CACHE STRING "Paths to corner-cases/cubelib libs")
set(CUBELIB_LIB "" CACHE STRING "Libs for corner-cases/cubelib")


set(BPRINTER_INCLUDE_DIR "./libs/bprinter/bprinter/include" CACHE STRING "Paths to bprinter includes")
set(BPRINTER_LIB_DIR "./libs/bprinter/bprinter/build" CACHE STRING "Paths to bprinter libs")
set(BPRINTER_LIB "bprinter" CACHE STRING "Libs for bprinter")


set(THREADPOOL_INCLUDE_DIR "./libs/ThreadPool/ThreadPool" CACHE STRING "Paths to ThreadPool includes")

set(TCLAP_INCLUDE_DIR "./libs/tclap/tclap-code/include" CACHE STRING "Paths to TCLAP includes")


set(GTEST_INCLUDE_DIR "./libs/googletest/googletest/googletest/include" CACHE STRING "Paths to googletest includes")
set(GTEST_LIB_DIR "./libs/googletest/googletest/googletest/build" CACHE STRING "Paths to googletest libs")
set(GTEST_LIB "gtest" CACHE STRING "Libs for googletest")


set(SGFXAPI_INCLUDE_DIR "./libs/sgfxapi/sgfxapi/include" CACHE STRING "Paths to sgfxapi includes")
set(SGFXAPI_LIB_DIR "./libs/sgfxapi/sgfxapi/build" CACHE STRING "Paths to sgfxapi libs")
set(SGFXAPI_LIB "sgfxapi;sgfxapi-drawutils" CACHE STRING "Libs for sgfxapi")

set(GLEW_INCLUDE_DIR "" CACHE STRING "Paths to glew includes")
set(GLEW_LIB_DIR "" CACHE STRING "Paths to glew libs")
set(GLEW_LIB "glew32" CACHE STRING "Lib for glew")

set(GLU_INCLUDE_DIR "" CACHE STRING "Paths to glut includes")
set(GLU_LIB_DIR "" CACHE STRING "Paths to glut libs")
set(GLU_LIB "glu32" CACHE STRING "Paths to glut libs")


#include(OptimizeForArchitecture)
#OptimizeForArchitecture()

if (MSVC)
    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Vc_ARCHITECTURE_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CXX_WARNINGS "-Wall -Wextra -Wpointer-arith -Wcast-align -fstrict-aliasing -Wno-unused-local-typedefs")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_WARNINGS} -fvisibility-inlines-hidden")
endif()


include_directories(
    ./include
    ./src
    ${GLM_INCLUDE_DIR}
    ${CEGUI_INCLUDE_DIR}
    ${GLFW3_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${MGL_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} #for generated headers
    
    ${CNBT_INCLUDE_DIR}
    ${CPPFORMAT_INCLUDE_DIR}
    ${BPRINTER_INCLUDE_DIR}
    ${CUBELIB_INCLUDE_DIR}
    ${THREADPOOL_INCLUDE_DIR}
    ${TCLAP_INCLUDE_DIR}
    
    ${SGFXAPI_INCLUDE_DIR}
    
    ${GTEST_INCLUDE_DIR}
    #lib/cNBT
    #lib/cppformat
    #lib/tclap-1.2.1/include
    #lib/ThreadPool
    #lib/bprinter-master/bprinter-master/include
    )
link_directories(
    ${CEGUI_LIB_DIR}
    ${GLFW3_LIB_DIR}
    ${MGL_LIB_DIR}
    ${CPPFORMAT_LIB_DIR}
    ${CNBT_LIB_DIR}
    ${BPRINTER_LIB_DIR}
    ${CUBELIB_LIB_DIR}
    ${SGFXAPI_LIB_DIR}
    ${GTEST_LIB_DIR}
    )



add_library(landscapes
    src/landscapes/svo_tree.cpp
    src/landscapes/svo_tree.block_mgmt.cpp
    src/landscapes/svo_tree.slice_mgmt.cpp
    src/landscapes/svo_buffer.cpp
    src/landscapes/svo_tree.sanity.cpp
    src/landscapes/svo_serialization.v1.cpp
    src/landscapes/svo_formatters.cpp
    src/landscapes/svo_render.cpp
    src/pempek_assert.cpp
    #cubelib.clgen.h
    )
set_property(TARGET landscapes PROPERTY CXX_STANDARD 11)

################################################################################

add_library(landscapes-mc
    src/landscapes/mcloader.cpp
    )
set_property(TARGET landscapes-mc PROPERTY CXX_STANDARD 11)

################################################################################



add_executable(landscapes-mc-demo
    src/landscapes/mc-demo.cpp
    )
set_property(TARGET landscapes-mc-demo PROPERTY CXX_STANDARD 11)
target_compile_definitions(landscapes-mc-demo PUBLIC -DCEGUI_USE_GLEW=1)

target_link_libraries(landscapes-mc-demo
    landscapes
    landscapes-mc
    
    ${MGL_LIB}
    ${CBNT_LIB}
    z
    ${CPPFORMAT_LIB}
    ${BPRINTER_LIB}
    ${SGFXAPI_LIB}
    ${CEGUI_LIB}
    #CEGUIBase-0 CEGUIOpenGLRenderer-0 CEGUIExpatParser CEGUICoreWindowRendererSet CEGUICommonDialogs-0

    ${GLFW3_LIB} ${GLU_LIB} ${GLEW_LIB} ${GLUT_LIB} gdi32 opengl32)

################################################################################

#target_link_libraries(sandbox
#   MathGeoLib nbt z
#   cppformat
#   bprinter
#   CEGUIBase-0 CEGUIOpenGLRenderer-0 CEGUIExpatParser CEGUICoreWindowRendererSet CEGUICommonDialogs-0
#   glfw3 opengl32 glu32 glew32 glut)

#set_property(TARGET sandbox PROPERTY CXX_STANDARD 11)

#-------------------
#add_executable(cubelib-clgen
#    src/cubelib.cpp
#    )


#target_link_libraries(cubelib-clgen

#)

#-------------------
#ADD_CUSTOM_COMMAND(
#    OUTPUT cubelib.clgen.h
#    COMMAND cubelib-clgen > cubelib.clgen.h
#    DEPENDS cubelib-clgen
#    )

#-------------------
add_executable(unittests
    src/unittests/entree_slices.cpp
    src/unittests/load_mca_region.cpp
    src/unittests/serialization.cpp
    src/unittests/camera_mapping.cpp
    src/unittests/main.cpp
    
    src/landscapes/mgl2glm.cpp
    )


target_link_libraries(unittests
    landscapes
    landscapes-mc
    ${CNBT_LIB}
    ${MGL_LIB}
    z
    ${CPPFORMAT_LIB}
    ${BPRINTER_LIB}
    
    ${GTEST_LIB}
    )


set_property(TARGET unittests PROPERTY CXX_STANDARD 11)


#-------------------
