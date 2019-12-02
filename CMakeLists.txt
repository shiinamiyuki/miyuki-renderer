cmake_minimum_required(VERSION 3.1)
project(MiyukiRenderer)


set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/miyuki.foundation)

file(GLOB SHADER_SRC shaders/*.*)

add_custom_target(COPY_SHADERS ALL 
	COMMAND ${CMAKE_COMMAND} -E copy_directory  ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:miyuki.studio>/shaders 
	DEPENDS ${SHADER_SRC})


add_executable(miyuki.studio src/miyuki.studio/main.cpp)
target_link_libraries(miyuki.studio foundation)
