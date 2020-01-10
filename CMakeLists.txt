cmake_minimum_required(VERSION 3.12)
project(MiyukiRenderer)


option(USE_EMBREE "Use Embree as ray intersection backend" ON)
option(USE_OIDN "Use OIDN as denoiser" ON)
option(EMBREE_FROM_SOURCE "Download and compile embree from source" OFF)

set(CMAKE_CXX_STANDARD 17)

if(${CMAKE_BUILD_TYPE} EQUAL "Release")
    add_compile_definitions(MYK_RELEASE)
endif()
if(MSVC)
	if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2 /std:c++17 /GL")
        set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /LTCG:INCREMENTAL")
        set(CMAKE_STATIC_LINKER_FLAGS  "${CMAKE_STATIC_LINKER_FLAGS} /LTCG:INCREMENTAL")
	endif()
endif()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    IF(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
        set(CXX_FS_LIBS)
    ENDIF()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -mavx2")
    set(CXX_FS_LIBS stdc++fs)
endif()

#find_package(Python3 REQUIRED)
#find_package(PythonLibs 3 REQUIRED)

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
include_directories(external/nanort/)

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
    IF(EMBREE_FROM_SOURCE)
        include(ExternalProject)
        include(${PROJECT_SOURCE_DIR}/cmake/cmake-embree)
		include_directories(${EMBREE_INCLUDE_DIRS})
        add_compile_definitions(MYK_USE_EMBREE)
    ELSE()
        IF(WIN32)
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Program Files/Intel/Embree3 x64")
        ENDIF()
        find_package(embree 3.6)
        IF(NOT embree_FOUND)
            message("Embree 3.6 not found; Falling back to custom BVH backend")
        ELSE()
            add_compile_definitions(MYK_USE_EMBREE)
            include_directories(${EMBREE_INCLUDE_DIRS})
        ENDIF()
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
target_link_libraries(core foundation ${CXX_FS_LIBS} ${OIDN_LIBRARIES}  ${EMBREE_LIBRARY})

add_executable(myk.cli src/standalone-renderer/main.cpp ${MiyukiAPI})
target_link_libraries(myk.cli core)


add_executable(mesh-importer src/mesh-importer/importer.cpp ${MiyukiAPI})
target_link_libraries(mesh-importer core)

file(GLOB serverSRC src/miyuki.server/*.*)
add_executable(miyuki.server ${serverSRC} ${MiyukiAPI})
if (WIN32)
    target_compile_definitions(miyuki.server PUBLIC _WIN32_WINNT=0x0601)
endif ()
target_include_directories(miyuki.server PRIVATE external/cpp-httplib)
target_link_libraries(miyuki.server asio wsock32 ws2_32 core)


add_executable(test-sdtree tests/test-sdtree.cpp)
target_link_libraries(test-sdtree core)

add_executable(test-adt tests/test-adt.cpp )
target_link_libraries(test-adt foundation)
