# CMakeList.txt : CMake project for MiyukiRenderer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(Miyuki)
set(CMAKE_CXX_STANDARD 17)
if(MSVC)
	SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /std:c++17 /arch:AVX2")
	SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP")
	SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /D_ENABLE_EXTENDED_ALIGNED_STORAGE /DMIYUKI_ON_WINDOWS")

endif()

# Add source to this project's executable.
include_directories("include/")
include_directories("include/miyuki/")
include_directories("include/bcd/ext/eigen")
link_libraries(glfw3 opengl32 embree3 OpenImageDenoise)
file(GLOB thirdpartySRC
        "include/lodepng/lodepng.cpp"
        "include/tiny_obj_loader/tiny_obj_loader.cc"
        "include/fmt/format.cc"
        "include/fmt/posix/cc"
        "include/hilbert/hilbert_curve.cpp"
		"include/bcd/bcd/*.cpp"
		)

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
	"src/core/phases/*.cc"
	"src/core/medium/*.cc"
	"src/core/materials/*.cc"
	"src/core/bsdfs/*.cc"
	"src/core/shaders/*.cc"
	"src/core/lights/*.cc"
	"src/core/cameras/*.cc"
	"src/core/integrators/*.cc"
	"src/core/denoiser/*.cc"
	"src/core/rasterzier/*.cc")

add_executable (mykgui "src/main/main.cc"
	${thirdpartySRC}
	${guiSRC}
	${coreSRC})
add_executable (test_reflection "src/main/testrefl.cc"
	${thirdpartySRC})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# TODO: Add tests and install targets if needed.
