cmake_minimum_required(VERSION 3.11)
project(opencl)

option(INSOURCEBUILD OFF)
if((${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR}) AND NOT
                                                              ${INSOURCEBUILD})
  message(
    FATAL_ERROR
      "In-source building disabled. Provide -DINSOURCEBUILD option ifyou are sure about it."
  )
endif()

set(CMAKE_CXX_STANDARD 20)
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
endif()

# Possibly support windows and MSVC compiler
if(MSVC)
  add_compile_options(/W3)
  add_definitions("/EHsc")
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
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
    ON
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

option(OPTIMIZE_FOR_NATIVE "Build with -march=native" ON)
if(OPTIMIZE_FOR_NATIVE)
  include(CheckCXXCompilerFlag)

  check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
  if(COMPILER_SUPPORTS_MARCH_NATIVE)
    message(STATUS "Compiling with -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
  else()
    message(WARNING "Compiler does not support -march=native")
  endif()
endif()

set(DCMAKE_EXPORT_COMPILE_COMMANDS ON)
find_program(BASH_PROGRAM bash)

# Add flags for GNU sampling profiler gprof in Debug build and RelWithDebInfo
option(PROFILE OFF)
if(PROFILE)
  add_compile_options(-pg)
  add_link_options(-pg)
endif()

option(SANITIZE OFF)
if(SANITIZE)
  add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=undefined -fno-omit-frame-pointer)
endif()

find_package(OpenCL REQUIRED)
find_package(OpenCLHeaders REQUIRED)
find_package(OpenCLHeadersCpp REQUIRED)

add_library(popl INTERFACE)
target_include_directories(popl INTERFACE 3rd-party/popl)

set(RESISTORS_NO_TESTING__
    ON
    CACHE BOOL "Disable testing" FORCE)
add_subdirectory(../../04-linmath/02-resistors/lib
                 ${CMAKE_CURRENT_BINARY_DIR}/linmath EXCLUDE_FROM_ALL)

find_package(
  Python3
  COMPONENTS Interpreter
  REQUIRED)

set(kernel2hpp ${CMAKE_CURRENT_SOURCE_DIR}/scripts/kernel2hpp.py)
set(KERNEL_HPP_DIR ${CMAKE_CURRENT_BINARY_DIR}/kernelhpp/kernelhpp)
set(KERNEL_HPP_INCLUDE ${CMAKE_CURRENT_BINARY_DIR}/kernelhpp)

function(add_opencl_program TARGET_NAME INPUT_FILES OPENCL_VERSION)
  add_executable(${TARGET_NAME} ${INPUT_FILES})
  target_link_libraries(${TARGET_NAME} PUBLIC OpenCL::OpenCL popl
                                              OpenCL::HeadersCpp)
  target_compile_definitions(
    ${TARGET_NAME} PUBLIC CL_HPP_TARGET_OPENCL_VERSION=${OPENCL_VERSION}
                          CL_TARGET_OPENCL_VERSION=${OPENCL_VERSION})
  target_include_directories(${TARGET_NAME} PUBLIC common include
                                                   ${KERNEL_HPP_INCLUDE})
endfunction()

set(TYPE
    "int"
    CACHE STRING "Type to compile with")
message(STATUS "Using type = ${TYPE}")
add_compile_definitions(TYPE__=${TYPE})

add_opencl_program(oclinfo oclinfo.cc 220)
add_opencl_program(vadd vadd.cc 220)

function(add_kernel TARGET_NAME INPUT_FILE)
  add_custom_command(
    OUTPUT ${KERNEL_HPP_DIR}/${TARGET_NAME}.hpp
    COMMAND Python3::Interpreter ${kernel2hpp} -i ${INPUT_FILE} -o
            ${KERNEL_HPP_DIR}/${TARGET_NAME}.hpp
    MAIN_DEPENDENCY ${INPUT_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_custom_target(${TARGET_NAME} ALL
                    DEPENDS ${KERNEL_HPP_DIR}/${TARGET_NAME}.hpp)
  set(${TARGET_NAME}_OUTPUTS
      ${KERNEL_HPP_DIR}/${TARGET_NAME}.hpp
      PARENT_SCOPE)
endfunction()

option(EIGEN_MAT_MULT "Compare matrix multiplication against Eigen" OFF)

add_opencl_program(matmult matmult.cc 220)
add_kernel(matmult_naive_kernel kernels/matmult_naive.cl)
add_kernel(matmult_tiled_kernel kernels/matmult_tiled.cl)
add_kernel(matmult_tiled_arb_kernel kernels/matmult_tiled_arb.cl)

add_custom_target(
  matmult_kernels ALL DEPENDS matmult_naive_kernel matmult_tiled_kernel
                              matmult_tiled_arb_kernel)
add_dependencies(matmult matmult_kernels)
target_link_libraries(matmult PUBLIC throttle)

if(EIGEN_MAT_MULT)
  find_package(Eigen3 3.3 REQUIRED NO_MODULE)
  target_link_libraries(matmult PUBLIC Eigen3::Eigen)
  target_compile_definitions(matmult PUBLIC EIGEN_MAT_MULT)
endif()

option(PAR_CPU_SORT "Use __gnu_parallel::sort to compare with. Requires OpenMP"
       OFF)

add_kernel(bitonic_naive_kernel kernels/bitonic_naive.cl)
add_kernel(bitonic_local_initial_kernel kernels/bitonic_local_initial.cl)

add_opencl_program(bitonic bitonic.cc 220)
add_custom_target(bitonic_kernels ALL DEPENDS bitonic_naive_kernel
                                              bitonic_local_initial_kernel)
add_dependencies(bitonic bitonic_kernels)

if(PAR_CPU_SORT)
  find_package(OpenMP REQUIRED)
  target_link_libraries(bitonic PUBLIC OpenMP::OpenMP_CXX)
  target_compile_definitions(bitonic PUBLIC PAR_CPU_SORT)
endif()

if(BASH_PROGRAM AND NOT BITONIC_NO_TESTING__)
  enable_testing()
  add_test(NAME test.network
           COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
                   "$<TARGET_FILE:bitonic>" ${CMAKE_CURRENT_SOURCE_DIR})
endif()
