cmake_minimum_required(VERSION 3.11)

project(
  graphs
  VERSION 1.0.0
  LANGUAGES CXX)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(GRAPHS_BUILD_TESTS OFF)
if(GRAPHS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
