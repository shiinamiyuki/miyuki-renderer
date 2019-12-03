cmake_minimum_required(VERSION 3.2)
project(MiyukiRenderer)


set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/miyuki.foundation)

file(GLOB SHADER_SRC shaders/*.*)


set(GENERATED_SHADER_SOURCE "generated/shadersources.cpp")

add_custom_command(OUTPUT ${GENERATED_SHADER_SOURCE}
	COMMAND python3  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/file_to_literal.py
	COMMAND ${CMAKE_COMMAND} -E touch ${GENERATED_SHADER_SOURCE}
	DEPENDS ${SHADER_SRC})

add_custom_target(BUILD_SHADERS DEPENDS ${GENERATED_SHADER_SOURCE})
find_package(VULKAN)

if(NOT Vulkan_FOUND)
	message(FATAL "Vulkan not found")
else()
	message("Vulkan include dirs: " ${Vulkan_INCLUDE_DIRS})
endif()

include_directories(Vulkan_INCLUDE_DIRS)
include_directories(include/)

file(GLOB RENDERER_SOURCE include/*/*/*.* include/*/*.* src/*.cpp src/core/*.cpp)


add_executable(miyuki.studio src/miyuki.studio/main.cpp ${RENDERER_SOURCE} ${GENERATED_SHADER_SOURCE})
target_link_libraries(miyuki.studio foundation ${Vulkan_LIBRARIES})
add_dependencies(miyuki.studio BUILD_SHADERS)
