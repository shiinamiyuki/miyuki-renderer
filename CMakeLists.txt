cmake_minimum_required(VERSION 3.1)
project(Miyuki)

set(CMAKE_CXX_STANDARD 14)
link_libraries(embree3 boost_system boost_filesystem boost_unit_test_framework glfw3 opengl32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
file(GLOB samplerSRC "src/samplers/*.h" "src/samplers/*.cc")
file(GLOB rendererSRC "src/renderer/*.h" "src/renderer/*.hpp")
file(GLOB lightSRC "src/lights/*.h" "src/lights/*.hpp" "src/lights/*.cc")
file(GLOB coreSRC "src/core/*.h" "src/core/*.hpp" "src/core/*.cc")
file(GLOB bsdfSRC "src/bsdfs/*.h" "src/bsdfs/*.hpp" "src/bsdfs/*.cc")
file(GLOB cameraSRC "src/cameras/*.h" "src/cameras/*.hpp" "src/cameras/*.cc")
file(GLOB mathSRC "src/math/*.h" "src/math/*.hpp" "src/math/*.cc")
file(GLOB utilSRC "src/utils/*.h" "src/utils/*.hpp" "src/utils/*.cc")
file(GLOB integratorSRC
        "src/integrators/*.h"
        "src/integrators/*.hpp"
        "src/integrators/*.cc"
        "src/integrators/ao/*.h"
        "src/integrators/ao/*.cc"
        "src/integrators/pathtracer/*.h"
        "src/integrators/pathtracer/*.cc"
        "src/integrators/bdpt/*.h"
        "src/integrators/bdpt/*.cc"
        "src/integrators/mmlt/*.cc"
        )
file(GLOB thirdpartySRC
        "thirdparty/lodepng/lodepng.cpp"
        "thirdparty/tiny_obj_loader/tiny_obj_loader.cc"
        "thirdparty/rand48/rand48.c"
        "thirdparty/rapidjson/rapidjson.h"
        "thirdparty/fmt/format.cc"
        "thirdparty/fmt/posix/cc"
        "thirdparty/sobol/sobol.cpp")
file(GLOB guiSRC
        "thirdparty/imgui/imgui_demo.cpp"
        "thirdparty/imgui/imgui_impl_glfw.cpp"
        "thirdparty/imgui/imgui.cpp"
        "thirdparty/imgui/imgui_widgets.cpp"
        "thirdparty/imgui/imgui_draw.cpp"
        "thirdparty/imgui/imgui_impl_opengl2.cpp"
        "thirdparty/imgui/gl3w.c")
add_executable(Miyuki
        ${rendererSRC}
        ${lightSRC}
        ${coreSRC}
        ${bsdfSRC}
        ${cameraSRC}
        ${thirdpartySRC}
        ${mathSRC}
        ${integratorSRC}
        ${utilSRC}
        ${samplerSRC}
        src/renderer/miyuki.cc
        src/renderer/rendersystem.cc)
add_executable(MiyukiGUI
        ${rendererSRC}
        ${lightSRC}
        ${coreSRC}
        ${bsdfSRC}
        ${cameraSRC}
        ${thirdpartySRC}
        ${mathSRC}
        ${integratorSRC}
        ${utilSRC}
        ${samplerSRC}
        ${guiSRC}
        src/renderer/miyuki-gui.cc
        src/renderer/rendersystem.cc)
add_executable(obj2json
        ${thirdpartySRC}
        ${utilSRC}
        src/tools/obj2json.cc)