cmake_minimum_required(VERSION 3.13)
project(Miyuki)

set(CMAKE_CXX_STANDARD 14)
link_libraries(embree3 boost_system boost_filesystem glfw3 opengl32 boost_unit_test_framework)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
add_executable(transform_test core/transform.cc unittest/transform_test.cc unittest/unittest.cc)
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
        core/material.cc
        core/transform.h
        core/transform.cc
        integrator/bdpt.h
        integrator/bdpt.cc
        integrator/pssmlt.h
        integrator/pssmlt.cc
        core/distribution.h
        core/distribution.cc
        core/stats.hpp
        core/texture.h
        core/texture.cc
        lib/rapidjson/document.h
        lib/rapidjson/rapidjson.h
        core/rendersystem.hpp
        core/rendersystem.cc)
#add_executable(Miyuki-GUI
#        lib/fmt/format.cc
#        lib/lodepng/lodepng.cpp
#        lib/tiny_obj_loader/tiny_obj_loader.cc
#        lib/rand48/rand48.c core/util.h
#        core/geometry.h
#        core/geometry.cc
#        core/util.cc
#        core/scene.cc
#        core/scene.h
#        core/integrator.h
#        core/Film.cc
#        core/Film.h
#        core/mesh.h
#        core/mesh.cc
#        core/spectrum.h
#        core/spectrum.cc
#        core/sampler.h
#        core/light.h
#        core/intersection.h
#        core/intersection.cc
#        core/ray.h
#        core/ray.cc
#        sampler/random.h
#        sampler/random.cc
#        core/interaction.h
#        core/interaction.cc
#        core/light.cc
#        integrator/ao.h
#        integrator/ao.cc
#        integrator/pathtracer.h
#        integrator/pathtracer.cc
#        core/material.h
#        core/material.cc
#        core/transform.h
#        core/transform.cc
#        integrator/bdpt.h
#        integrator/bdpt.cc
#        integrator/pssmlt.h
#        integrator/pssmlt.cc
#        core/distribution.h
#        core/distribution.cc
#        core/stats.hpp
#        core/texture.h
#        core/texture.cc
#        lib/rapidjson/document.h
#        lib/rapidjson/rapidjson
#        lib/imgui/imgui.cpp
#        lib/imgui/imgui_impl_glfw.cpp
#        lib/imgui/imgui_impl_opengl2.cpp
#        lib/imgui/imgui_draw.cpp
#        lib/imgui/imgui_widgets.cpp
#        lib/imgui/imgui_demo.cpp
#        core/miyuki-gui.cc
#        core/rendersystem.hpp
#        core/rendersystem.cc)