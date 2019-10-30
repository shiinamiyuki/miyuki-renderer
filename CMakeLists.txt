project(MiyukiRenderer)

cmake_minimum_required(VERSION 3.1)
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

file(GLOB MiyukiAPI src/api/*.h src/api/*.hpp src/api/detail/*.hpp src/api/detail/*.h)
file(GLOB libcoreSource src/core/*.cpp
        src/core/accelerators/*.*
        src/core/shaders/*.*
        src/core/shapes/*.*
        src/core/cameras/*.*
        src/core/bsdfs/*.*
        external/tinyobjloader/tiny_obj_loader.cc
        external/lodepng/lodepng.cpp

        )
add_library(core ${MiyukiAPI} ${libcoreSource})
target_link_libraries(core fmt)
add_executable(miyuki-renderer src/standalone-renderer/main.cpp ${MiyukiAPI})
target_link_libraries(miyuki-renderer core)
