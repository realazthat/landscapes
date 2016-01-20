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


set(GLFW3_LIB_DEFAULT "glfw")
if (WIN32)
    set(GLFW3_LIB_DEFAULT "glfw3")
endif()
set(GLFW3_INCLUDE_DIR "./libs/glfw3/glfw/include"
    CACHE STRING "Paths to glfw3 includes")
set(GLFW3_LIB_DIR "./libs/glfw3/glfw/build/src"
    CACHE STRING "Paths to glfw3 libs")
set(GLFW3_LIB "${GLFW3_LIB_DEFAULT}"
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

set(GLEW_LIB_DEFAULT "GLEW")
if (WIN32)
    set(GLEW_LIB_DEFAULT "glew32")
endif()
set(GLEW_INCLUDE_DIR "" CACHE STRING "Paths to GLEW includes")
set(GLEW_LIB_DIR "" CACHE STRING "Paths to GLEW libs")
set(GLEW_LIB "${GLEW_LIB_DEFAULT}" CACHE STRING "Lib for GLEW")

set(GLU_LIB_DEFAULT "GLU")
if (WIN32)
    set(GLU_LIB_DEFAULT "glu32")
endif()
set(GLU_INCLUDE_DIR "" CACHE STRING "Paths to GLU includes")
set(GLU_LIB_DIR "" CACHE STRING "Paths to GLU libs")
set(GLU_LIB "${GLU_LIB_DEFAULT}" CACHE STRING "Libs for GLU")


set(GLUT_INCLUDE_DIR "" CACHE STRING "Paths to glut includes")
set(GLUT_LIB_DIR "" CACHE STRING "Paths to glut libs")
set(GLUT_LIB "glut" CACHE STRING "Libs for glut")


set(OPENGL_LIB_DEFAULT "GL")
if (WIN32)
    set(OPENGL_LIB_DEFAULT "opengl32")
endif()
set(OPENGL_INCLUDE_DIR "" CACHE STRING "Paths to opengl includes")
set(OPENGL_LIB_DIR "" CACHE STRING "Paths to opengl libs")
set(OPENGL_LIB "${OPENGL_LIB_DEFAULT}" CACHE STRING "Libs for opengl")



set(PLATFORM_LIB_DEFAULT "")
if (WIN32)
    set(PLATFORM_LIB_DEFAULT "gdi32")
endif()
set(PLATFORM_INCLUDE_DIR "" CACHE STRING "Paths to Platform-specific includes")
set(PLATFORM_LIB_DIR "" CACHE STRING "Paths to Platform-specific libs")
set(PLATFORM_LIB "${PLATFORM_LIB_DEFAULT}" CACHE STRING "Platform-specific libs")






if (MSVC)
    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif()

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
    src/landscapes/svo_slice.comparison.cpp
    )
set_property(TARGET landscapes PROPERTY CXX_STANDARD 14)
set_property(TARGET landscapes PROPERTY CXX_STANDARD_REQUIRED ON)

################################################################################

add_library(landscapes-mc
    src/landscapes/mcloader.cpp
    )
set_property(TARGET landscapes-mc PROPERTY CXX_STANDARD 14)
set_property(TARGET landscapes-mc PROPERTY CXX_STANDARD_REQUIRED ON)

################################################################################

add_executable(landscapes-zorder-genconsts
    src/landscapes/svo_curves.genconsts.cpp
    )
set_property(TARGET landscapes-zorder-genconsts PROPERTY CXX_STANDARD 14)
set_property(TARGET landscapes-zorder-genconsts PROPERTY CXX_STANDARD_REQUIRED ON)


################################################################################
add_executable(landscapes-mc-demo
    src/landscapes/mc-demo.cpp
    )

target_link_libraries(landscapes-mc-demo
    landscapes
    landscapes-mc
    
    ${MGL_LIB}
    ${CNBT_LIB}
    z
    ${CPPFORMAT_LIB}
    ${BPRINTER_LIB}
    ${SGFXAPI_LIB}
    ${CEGUI_LIB}

    ${GLFW3_LIB} ${GLU_LIB} ${GLEW_LIB} ${GLUT_LIB} ${OPENGL_LIB} ${PLATFORM_LIB})

set_property(TARGET landscapes-mc-demo PROPERTY CXX_STANDARD 14)
set_property(TARGET landscapes-mc-demo PROPERTY CXX_STANDARD_REQUIRED ON)
################################################################################
add_executable(landscapes-mc-converter
    src/landscapes/mc-converter.cpp
    )

target_link_libraries(landscapes-mc-converter
    landscapes
    landscapes-mc
    
    ${CNBT_LIB}
    z
    ${CPPFORMAT_LIB}
    ${BPRINTER_LIB}
    ${PLATFORM_LIB})

set_property(TARGET landscapes-mc-converter PROPERTY CXX_STANDARD 14)
set_property(TARGET landscapes-mc-converter PROPERTY CXX_STANDARD_REQUIRED ON)
################################################################################


add_executable(unittests
    src/unittests/entree_slices.cpp
    src/unittests/load_mca_region.cpp
    src/unittests/serialization.cpp
    src/unittests/camera_mapping.cpp
    src/unittests/overlap_open_close_range.cpp
    src/unittests/z-order.cpp
    src/unittests/constants.cpp
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
    ${PLATFORM_LIB}
    )


set_property(TARGET unittests PROPERTY CXX_STANDARD 14)
set_property(TARGET unittests PROPERTY CXX_STANDARD_REQUIRED ON)


################################################################################
