project(MiyukiRenderer)

cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 20)

find_package(Python3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)


include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(src)

add_subdirectory(external/pybind11)
include_directories(external/pybind11/include)

file(GLOB MiyukiAPI src/api/miyuki/*.h src/api/detail/*.hpp)

add_library(core SHARED src/libcore/core.cpp ${MiyukiAPI})
add_executable(miyuki-renderer src/standalone-renderer/main.cpp ${MiyukiAPI})
target_link_libraries(miyuki-renderer core)
