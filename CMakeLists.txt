# CMakeList.txt : CMake project for MiyukiRenderer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(Miyuki)

if(MSVC)
	SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /std:c++17")
	SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP")
	SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /D_ENABLE_EXTENDED_ALIGNED_STORAGE /DMIYUKI_ON_WINDOWS")
else()
	set(CMAKE_CXX_STANDARD 17)
endif()

# Add source to this project's executable.
include_directories("include/")
include_directories("include/miyuki/")
link_libraries(glfw3 opengl32 embree3)
file(GLOB thirdpartySRC
        "include/lodepng/lodepng.cpp"
        "include/tiny_obj_loader/tiny_obj_loader.cc"
        "include/fmt/format.cc"
        "include/fmt/posix/cc"
        "include/hilbert/hilbert_curve.cpp")

file(GLOB guiSRC
	"include/imgui/*.c"
	"include/imgui/*.cpp"
	"src/ui/*.cc"
	"src/hw/*.cc")

file(GLOB coreSRC
	"src/core/*.cc"
	"src/math/*.cc"
	"src/utils/*.cc"
	"src/io/*.cc"
	"src/engine/*.cc"
	"src/core/samplers/*.cc"
	"src/core/materials/*.cc"
	"src/core/bsdfs/*.cc"
	"src/core/lights/*.cc"
	"src/core/cameras/*.cc"
	"src/core/integrators/*.cc")

add_executable (mykgui "src/main/main.cc"
	${thirdpartySRC}
	${guiSRC}
	${coreSRC})

# TODO: Add tests and install targets if needed.
