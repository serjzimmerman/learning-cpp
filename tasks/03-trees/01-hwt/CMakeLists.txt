cmake_minimum_required(VERSION 3.11)
project(hwt)

option(INSOURCEBUILD OFF)

if((${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR}) AND NOT
                                                              ${INSOURCEBUILD})
  message(
    FATAL_ERROR
      "In-source building disabled. Provide -DINSOURCEBUILD option if you are sure about it."
  )
endif()

set(CMAKE_CXX_STANDARD 20)

# Possibly support windows and MSVC compiler
if(MSVC)
  add_compile_options(/W3)
else()
  add_compile_options(-Wall -Wextra)
endif()

find_package(GTest)
set(NOGTEST
    OFF
    CACHE BOOL "Disable GoogleTest")

enable_testing()
if(NOT NOGTEST
   AND GTEST_FOUND
   AND CMAKE_MINOR_VERSION GREATER_EQUAL 20)
  set(ENABLE_GTEST ON)
else()
  message(WARNING "Google Test disabled")
endif()

set(NOLINT
    OFF
    CACHE BOOL "Disable clang-tidy")

if(NOT ${NOLINT})
  find_program(CLANG_TIDY_COMMAND clang-tidy)
  if(CLANG_TIDY_COMMAND)

    # Clang-tidy for linting
    set(CMAKE_CXX_CLANG_TIDY clang-tidy; -checks=google-*,cppcoreguidelines-*)
    set(CMAKE_C_CLANG_TIDY clang-tidy; -checks=google-*,cppcoreguidelines-*)

  else()
    message(WARNING "Unable to find clang-tidy, linter disabled")
  endif()

else()
  message(WARNING "-DNOLINT option provided, linter disabled")
endif()

set(DCMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(BASH_PROGRAM bash)

# Add flags for GNU sampling profiler gprof in Debug build and RelWithDebInfo
add_compile_options("$<$<CONFIG:RelWithDebInfo>:-pg>")
add_link_options("$<$<CONFIG:RelWithDebInfo>:-pg>")
add_compile_options("$<$<CONFIG:Debug>:-pg>")
add_link_options("$<$<CONFIG:Debug>:-pg>")

add_subdirectory(lib)
add_subdirectory(test)
