cmake_minimum_required(VERSION 3.1)
project(MiyukiRenderer)
set(CMAKE_CXX_STANDARD 17)

find_package(Python3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)


include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(src)

add_subdirectory(external/pybind11)
add_subdirectory(external/fmt)
include_directories(external/pybind11/include)
include_directories(external/linalg)
include_directories(external/cereal/include)
include_directories(external/json/include)
include_directories(external/fmt/include)
include_directories(external/tinyobjloader/)
include_directories(external/stb/)
include_directories(external/lodepng/)
include_directories(external/cxxopts/include)

file(GLOB MiyukiAPI src/api/*.h src/api/*.hpp src/api/detail/*.hpp src/api/detail/*.h)
file(GLOB libcoreSource src/core/*.cpp
        src/core/accelerators/*.*
        src/core/shaders/*.*
        src/core/shapes/*.*
        src/core/cameras/*.*
        src/core/bsdfs/*.*
        src/core/integrators/*.*
        src/core/samplers/*.*
        external/tinyobjloader/tiny_obj_loader.cc
        external/lodepng/lodepng.cpp

        )
add_library(core ${MiyukiAPI} ${libcoreSource})
target_link_libraries(core fmt)
add_executable(myk-cli src/standalone-renderer/main.cpp ${MiyukiAPI})
add_executable(mesh-importer src/mesh-importer/importer.cpp ${MiyukiAPI})
target_link_libraries(mesh-importer core)
target_link_libraries(myk-cli core)
