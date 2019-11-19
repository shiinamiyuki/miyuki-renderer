cmake_minimum_required(VERSION 3.1)
project(MiyukiRenderer)


option(USE_EMBREE "Use Embree as ray intersection backend" ON)
option(EMBREE_FROM_SOUCE "Download and compile embree from souce" OFF)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
	if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qpar /arch:AVX2 /std:c++17 /GL")
	endif()
endif()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    IF(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    ENDIF()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

find_package(Python3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(src)
add_subdirectory(external/glfw)
add_subdirectory(external/gl3w)
add_subdirectory(external/fmt)

include_directories(external/sobol)
include_directories(external/pybind11/include)
include_directories(external/linalg)
include_directories(external/cereal/include)
include_directories(external/json/include)
include_directories(external/fmt/include)
include_directories(external/tinyobjloader/)
include_directories(external/stb/)
include_directories(external/lodepng/)
include_directories(external/cxxopts/include)
include_directories(external/gl3w/include)
include_directories(external/glfw/include)
include_directories(external/imgui/)

file(GLOB MiyukiAPI src/api/*.h src/api/*.hpp src/api/detail/*.hpp src/api/detail/*.h)
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
add_library(core ${MiyukiAPI} ${libcoreSource})
target_link_libraries(core fmt)
add_executable(myk-cli src/standalone-renderer/main.cpp ${MiyukiAPI})
add_executable(mesh-importer src/mesh-importer/importer.cpp ${MiyukiAPI})
target_link_libraries(mesh-importer core ${EMBREE_LIBRARY})
target_link_libraries(myk-cli core ${EMBREE_LIBRARY})

file(GLOB IMGUI_SRC external/imgui/*.cpp)

add_library(imgui 	${IMGUI_SRC}
	 external/imgui/examples/imgui_impl_opengl3.cpp
	 external/imgui/examples/imgui_impl_glfw.cpp
)

target_link_libraries(imgui gl3w)

file(GLOB UI_SRC src/ui/*.*)

add_executable(miyuki.studio src/miyuki.studio/main.cpp ${UI_SRC}  ${MiyukiAPI})
target_link_libraries(miyuki.studio core ${EMBREE_LIBRARY} imgui glfw)