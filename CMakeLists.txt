cmake_minimum_required(VERSION 3.13)
project(Miyuki)

set(CMAKE_CXX_STANDARD 14)
link_libraries(embree3 boost_system boost_filesystem)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
add_executable(Miyuki
        lib/fmt/format.cc
        lib/lodepng/lodepng.cpp
        lib/tiny_obj_loader/tiny_obj_loader.cc
        lib/rand48/rand48.c core/util.h
        core/miyuki.cc
        core/geometry.h
        core/geometry.cc
        core/util.cc
        core/scene.cc
        core/scene.h
        core/integrator.h
        core/Film.cc
        core/Film.h
        core/mesh.h
        core/mesh.cc
        core/spectrum.h
        core/spectrum.cc
        core/sampler.h
        core/light.h
        core/intersection.h
        core/intersection.cc
        core/ray.h
        core/ray.cc
        sampler/random.h
        sampler/random.cc
        core/interaction.h
        core/interaction.cc
        core/light.cc
        integrator/ao.h
        integrator/ao.cc
        integrator/pathtracer.h
        integrator/pathtracer.cc
        core/material.h
        core/material.cc core/transform.h core/transform.cc)