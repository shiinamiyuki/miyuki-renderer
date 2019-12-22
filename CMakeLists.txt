cmake_minimum_required(VERSION 3.1)
project(MiyukiRenderer)


option(USE_EMBREE "Use Embree as ray intersection backend" ON)
option(USE_OIDN "Use OIDN as denoiser" ON)
option(EMBREE_FROM_SOUCE "Download and compile embree from souce" OFF)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
	if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qpar /arch:AVX2 /std:c++17 /GL")
	endif()
endif()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    IF(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target x86_64-pc-windows-gnu -femulated-tls ")
        set(CXX_FS_LIBS stdc++fs)
    ENDIF()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CXX_FS_LIBS stdc++fs)
endif()

find_package(Python3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(include)
add_subdirectory(external/gl3w)
add_subdirectory(external/miyuki.foundation)

include_directories(external/sobol)
include_directories(external/pybind11/include)
include_directories(external/tinyobjloader/)
include_directories(external/cxxopts/include)
include_directories(external/gl3w/include)
include_directories(external/imgui/)
include_directories(external/PerlinNoise)
include_directories(external/miyuki.foundation/external/miyuki.serialize/include)

file(GLOB MiyukiAPI include/miyuki.renderer/*.h include/miyuki.renderer/*.hpp include/miyuki.renderer/detail/*.hpp include/miyuki.renderer/detail/*.h)
file(GLOB libcoreSource src/core/*.cpp
        src/core/accelerators/*.*
        src/core/shaders/*.*
        src/core/shapes/*.*
        src/core/cameras/*.*
        src/core/bsdfs/*.*
        src/core/lights/*.*
        src/core/integrators/*.*
        src/core/samplers/*.*
		src/core/mesh-importers/*.*
        src/core/lightdistributions/*.*
        src/core/denoisers/*.*
        external/tinyobjloader/tiny_obj_loader.cc
        external/lodepng/lodepng.cpp

        )


IF(USE_EMBREE)
    add_compile_definitions(MYK_USE_EMBREE)
    IF(EMBREE_FROM_SOUCE)
        include(ExternalProject)
        include(${PROJECT_SOURCE_DIR}/cmake/cmake-embree)
		include_directories(${EMBREE_INCLUDE_DIRS})
    ELSE()
        IF(WIN32)
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Program Files/Intel/Embree3 x64")
        ENDIF()
        find_package(embree 3.6 REQUIRED)
        IF(NOT embree_FOUND)
            message(FATAL_ERROR "Embree 3.6 not found")
        ENDIF()
        include_directories(${EMBREE_INCLUDE_DIRS})

    ENDIF()
ENDIF()

#
#IF(USE_OIDN)
#    link_libraries(tbb)
#    add_subdirectory(external/oidn)
#    include_directories(external/oidn/include)
#    add_compile_definitions(MYK_USE_OIDN)
#    set(OIDN_LIBRARIES OpenImageDenoise)
#ENDIF()

add_library(core ${MiyukiAPI} ${libcoreSource})
target_link_libraries(core foundation ${CXX_FS_LIBS} ${OIDN_LIBRARIES})
add_executable(myk.cli src/standalone-renderer/main.cpp ${MiyukiAPI})
add_executable(mesh-importer src/mesh-importer/importer.cpp ${MiyukiAPI})
target_link_libraries(mesh-importer core ${EMBREE_LIBRARY})
target_link_libraries(myk.cli core ${EMBREE_LIBRARY})

