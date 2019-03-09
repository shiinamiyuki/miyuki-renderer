cmake_minimum_required(VERSION 3.1)
project(Miyuki)

set(CMAKE_CXX_STANDARD 17)
include_directories("src/")
link_libraries(embree3 boost_system boost_filesystem boost_unit_test_framework glfw3 opengl32)
file(GLOB coreSRC
        "src/core/*.cc"
        "src/math/*.cc"
        "src/io/*.cc"
        "src/cameras/*.cc"
        "src/samplers/*.cc"
        "src/bsdfs/*.cc"
        "src/lights/*.cc"
        "src/materials/*.cc")
file(GLOB thirdpartySRC
        "src/thirdparty/lodepng/lodepng.cpp"
        "src/thirdparty/tiny_obj_loader/tiny_obj_loader.cc"
        "src/thirdparty/rand48/rand48.c"
        "src/thirdparty/rapidjson/rapidjson.h"
        "src/thirdparty/fmt/format.cc"
        "src/thirdparty/fmt/posix/cc"
        "src/thirdparty/sobol/sobol.cpp"
        "src/thirdparty/hilbert/hilbert_curve.cpp")
file(GLOB utilsSRC
        "src/utils/*.cc"
        "src/utils/*.hpp")
file(GLOB guiSRC
        "src/thirdparty/imgui/imgui_demo.cpp"
        "src/thirdparty/imgui/imgui_impl_glfw.cpp"
        "src/thirdparty/imgui/imgui.cpp"
        "src/thirdparty/imgui/imgui_widgets.cpp"
        "src/thirdparty/imgui/imgui_draw.cpp"
        "src/thirdparty/imgui/imgui_impl_opengl2.cpp"
        "src/thirdparty/imgui/gl3w.c")
file(GLOB integratorSRC
        "src/integrators/*.cc"
        "src/integrators/volpath/*.cc")
add_executable(MiyukiGUI
        "src/miyuki.h"
        ${thirdpartySRC}
        ${utilsSRC}
        ${coreSRC}
        ${integratorSRC}
        ${guiSRC}
        src/renderer/miyuki-gui.cc
        src/renderer/renderengine.cc)
add_executable(Miyuki
        "src/miyuki.h"
        ${thirdpartySRC}
        ${utilsSRC}
        ${coreSRC}
        ${integratorSRC}
        src/renderer/miyuki.cc
        src/renderer/renderengine.cc)
add_executable(obj2json
        "src/miyuki.h"
        "src/tools/obj2json.cc"
        "src/io/io.cc"
        "src/io/serialize.cc"
        ${thirdpartySRC}
        ${utilsSRC})
add_executable(unittest
        "src/miyuki.h"
        ${thirdpartySRC}
        ${utilsSRC}
        ${coreSRC}
        ${integratorSRC}
        "src/unittests/parameter_test.cc"
        "src/unittests/unittest.cc")