cmake_minimum_required(VERSION 3.10)
project(Yeet)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  webgpu
  GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
  GIT_TAG wgpu)
FetchContent_MakeAvailable(webgpu)

FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  glfw3webgpu
  GIT_REPOSITORY https://github.com/eliemichel/glfw3webgpu.git
  GIT_TAG main)
FetchContent_MakeAvailable(glfw3webgpu)

find_package(Threads REQUIRED)

function(get_subdirs DIRECTORY OUTPUT)
  file(
    GLOB SUBDIRS
    RELATIVE ${DIRECTORY}
    "${DIRECTORY}/*")
  set(DIRS "")
  foreach(SUBDIR ${SUBDIRS})
    if(IS_DIRECTORY "${DIRECTORY}/${SUBDIR}")
      list(APPEND DIRS "${SUBDIR}")
    endif()
  endforeach()
  set(${OUTPUT}
      ${DIRS}
      PARENT_SCOPE)
endfunction()

function(add_module_to_interface MODULE_NAME INTERFACE_NAME)
  message(STATUS "Adding module ${MODULE_NAME} to interface ${INTERFACE_NAME}")
  set(TARGET_NAME ${MODULE_NAME})

  file(GLOB_RECURSE MODULE_SOURCES
       "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}*.cpp")

  add_library(${TARGET_NAME} STATIC ${MODULE_SOURCES})

  # Include the directories of all fetched dependencies and other modules
  target_include_directories(
    ${MODULE_NAME}
    PUBLIC lib/${MODULE_NAME}/include
    PRIVATE ${webgpu_SOURCE_DIR}/include ${glfw_SOURCE_DIR}/include
            ${glfw3webgpu_SOURCE_DIR})

  # Add the directory of glfw3webgpu library
  link_directories(${glfw3webgpu_SOURCE_DIR}/build)

  # Iterate through other modules and add their include directories
  foreach(OTHER_MODULE_NAME ${MODULE_NAMES})
    if(NOT ${OTHER_MODULE_NAME} STREQUAL ${MODULE_NAME})
      target_include_directories(${MODULE_NAME}
                                 PRIVATE lib/${OTHER_MODULE_NAME}/include)
    endif()
  endforeach()

  target_link_libraries(${INTERFACE_NAME} INTERFACE ${MODULE_NAME} glfw3webgpu
  )# Add glfw3webgpu to the target_link_libraries
endfunction()

add_library(yeet INTERFACE)
get_subdirs("${CMAKE_SOURCE_DIR}/lib" MODULE_NAMES)
foreach(MODULE_NAME ${MODULE_NAMES})
  add_module_to_interface(${MODULE_NAME} yeet)
endforeach()

enable_testing()
add_executable(yeet_exe src/entry.cpp)

# Adjust the order of linked libraries
target_link_libraries(
  yeet_exe
  PRIVATE yeet
          gtest_main
          fmt::fmt
          Threads::Threads
          glfw
          webgpu)
target_link_libraries(yeet_exe PRIVATE $<TARGET_FILE:glfw3webgpu>)

set_target_properties(yeet_exe PROPERTIES OUTPUT_NAME "yeet")

if(MSVC)
  target_compile_options(yeet_exe PRIVATE /W4)
else()
  target_compile_options(yeet_exe PRIVATE -Wall -Wextra -Wno-c++20-extensions
                                          -fpermissive -pedantic)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_link_options(yeet_exe PUBLIC -fsanitize=address -lasan)
  target_compile_options(yeet_exe PRIVATE -O0 -g)
else()
  target_compile_options(yeet_exe PRIVATE -O3)
endif()
