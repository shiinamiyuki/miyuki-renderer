cmake_minimum_required(VERSION 3.1)
project(Miyuki)

set(CMAKE_CXX_STANDARD 14)
link_libraries(embree3 boost_system boost_filesystem glfw3 opengl32 boost_unit_test_framework)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
add_executable(test_transform
        core/transform.cc
        core/interaction.cc
        unittest/unittest.cc
        unittest/test_transform.cc )
add_executable(test_memory
        core/memory.cc
        thirdparty/fmt/format.cc
        unittest/unittest.cc
        unittest/test_memory.cc)
add_executable(Miyuki
        thirdparty/fmt/format.cc
        thirdparty/lodepng/lodepng.cpp
        thirdparty/tiny_obj_loader/tiny_obj_loader.cc
        thirdparty/rand48/rand48.c core/util.h
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
        thirdparty/rapidjson/document.h
        thirdparty/rapidjson/rapidjson.h
        core/rendersystem.hpp
        core/rendersystem.cc sampler/stratified.h sampler/stratified.cc core/memory.h core/memory.cc core/microfacet.h core/microfacet.cc core/reflection.cc core/reflection.h material/matte.cc material/matte.h core/materialfactory.cc core/materialfactory.h)
#add_executable(Miyuki-GUI
#        thirdparty/fmt/format.cc
#        thirdparty/lodepng/lodepng.cpp
#        thirdparty/tiny_obj_loader/tiny_obj_loader.cc
#        thirdparty/rand48/rand48.c core/util.h
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
#        thirdparty/rapidjson/document.h
#        thirdparty/rapidjson/rapidjson
#        thirdparty/imgui/imgui.cpp
#        thirdparty/imgui/imgui_impl_glfw.cpp
#        thirdparty/imgui/imgui_impl_opengl2.cpp
#        thirdparty/imgui/imgui_draw.cpp
#        thirdparty/imgui/imgui_widgets.cpp
#        thirdparty/imgui/imgui_demo.cpp
#        core/miyuki-gui.cc
#        core/rendersystem.hpp
#        core/rendersystem.cc)