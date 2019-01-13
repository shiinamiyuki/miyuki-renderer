cmake_minimum_required(VERSION 3.13)
project(Miyuki)

set(CMAKE_CXX_STANDARD 14)
link_libraries(embree3)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -march=native")
add_executable(Miyuki
        lib/fmt/format.cc
        lib/lodepng/lodepng.cpp
        lib/tiny_obj_loader/tiny_obj_loader.cc
        lib/rand48/rand48.c core/util.h
        bench_test.hpp
        core/miyuki.cc
        core/geometry.h
        core/geometry.cc core/primitive.h core/util.cc core/scene.cc core/scene.h core/integrator.h core/Film.cc core/Film.h)