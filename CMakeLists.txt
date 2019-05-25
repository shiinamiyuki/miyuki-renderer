# CMakeList.txt : CMake project for MiyukiRenderer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(Miyuki)

# Add source to this project's executable.
include_directories("include/")
link_libraries(glfw3 opengl32)
file(GLOB thirdpartySRC
        "include/lodepng/lodepng.cpp"
        "include/tiny_obj_loader/tiny_obj_loader.cc"
        "include/rapidjson/rapidjson.h"
        "include/fmt/format.cc"
        "include/fmt/posix/cc"
        "include/hilbert/hilbert_curve.cpp")

file(GLOB guiSRC
	"include/imgui/*.cc")
file(GLOB coreSRC
	"src/math/*.cc")

add_executable (mykgui "src/main/main.cc"
	${thirdpartySRC}
	${guiSRC}
	${coreSRC})

# TODO: Add tests and install targets if needed.
