cmake_minimum_required(VERSION 3.11)

project(
  ezvis
  VERSION 1.0.0
  LANGUAGES CXX)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(FetchContent)
find_package(ctti)

if(NOT ctti_FOUND)
  FetchContent_Declare(
    ctti
    GIT_REPOSITORY https://github.com/Manu343726/ctti
    GIT_TAG master)

  if(NOT ctti_POPULATED)
    FetchContent_Populate(ctti)
    add_subdirectory(${ctti_SOURCE_DIR} ${ctti_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()

  FetchContent_MakeAvailable(ctti)
endif()

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(${PROJECT_NAME} INTERFACE ctti)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

option(EZVIS_BUILD_TESTS "Build ezvis unit tests" OFF)

if(EZVIS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
