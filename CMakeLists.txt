cmake_minimum_required(VERSION 3.0...3.25)
project(
  yeet
  VERSION 0.1.0
	LANGUAGES CXX C
)

include(FetchContent)

FetchContent_Declare(
  webgpu
  GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
  GIT_TAG        wgpu
)
FetchContent_MakeAvailable(webgpu)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
)
FetchContent_MakeAvailable(glfw)

add_executable(yeet_exe src/entry.cpp)
if (MSVC)
	target_compile_options(yeet_exe PRIVATE /W4)
else()
  target_compile_options(yeet_exe PRIVATE -Wall -Wextra -Wno-c++20-extensions -pedantic)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(yeet_exe PRIVATE -O3)
else()
  target_link_options(yeet_exe PUBLIC -fsanitize=address -lasan)
  target_compile_options(yeet_exe PRIVATE -O0 -g)
endif()

target_link_libraries(yeet_exe
  PRIVATE
    glfw
    webgpu
)

target_copy_webgpu_binaries(yeet_exe)
