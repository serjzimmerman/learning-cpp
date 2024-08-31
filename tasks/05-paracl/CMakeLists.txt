cmake_minimum_required(VERSION 3.14)
project(paracl)

option(INSOURCEBUILD OFF)

if((${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR}) AND NOT
                                                              ${INSOURCEBUILD})
  message(
    FATAL_ERROR
      "In-source building disabled. Provide -DINSOURCEBUILD option if you are sure about it."
  )
endif()

set(CONSTEXPR_DEPTH_LIMIT 4096)
# Possibly support windows and MSVC compiler
if(MSVC)
  add_compile_options(/W2)
else()
  add_compile_options(-Wall -Wextra -fconstexpr-depth=${CONSTEXPR_DEPTH_LIMIT})
endif()

option(LINT OFF)
if(${LINT})
  find_program(CLANG_TIDY_EXECUTABLE "clang-tidy")

  set(CLANG_TIDY_COMMAND
      "${CLANG_TIDY_EXECUTABLE}"
      "-checks=-*,modernize-*,-modernize-use-trailing-return-type*,readability-*,-readability-implicit-bool-conversion*,\
        -readability-identifier-length*,portability-*,bugprone-*")

  function(target_enable_linter TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES CXX_CLANG_TIDY
                                                    "${CLANG_TIDY_COMMAND}")
  endfunction()
else()
  function(target_enable_linter TARGET_NAME)
    # Do nothing
  endfunction()
endif()

option(PROFILE OFF)
if(PROFILE)
  add_compile_options(-pg)
  add_link_options(-pg)
endif()

option(SANITIZE OFF)
if(SANITIZE)
  add_compile_options(-fsanitize=undefined,address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=undefined,address -fno-omit-frame-pointer)
endif()

# Tools
find_package(FLEX REQUIRED)
find_package(BISON 3.5 REQUIRED)

# External libraries
find_package(fmt REQUIRED)
add_subdirectory(3rd-party/popl EXCLUDE_FROM_ALL)
add_subdirectory(3rd-party/graphs EXCLUDE_FROM_ALL)
add_subdirectory(ezvis)

# Compiler source
set(BYTECODE_VM_SOURCES src/bytecode_vm/decl_vm.cc)

add_library(bytecode_vm STATIC ${BYTECODE_VM_SOURCES})
target_include_directories(bytecode_vm PUBLIC include)
target_compile_features(bytecode_vm PUBLIC cxx_std_20)
target_link_libraries(bytecode_vm PUBLIC fmt::fmt)

add_executable(pcldis src/pcldis.cc)
target_link_libraries(pcldis popl bytecode_vm)
target_enable_linter(pcldis)

add_executable(pclvm src/pclvm.cc)
target_link_libraries(pclvm popl bytecode_vm)
target_enable_linter(pclvm)

bison_target(
  parser src/frontend/parser/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.y.cc
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/bison_paracl_parser.hpp)

flex_target(scanner src/frontend/parser/scanner.l
            ${CMAKE_CURRENT_BINARY_DIR}/scanner.l.cc)

add_flex_bison_dependency(scanner parser)

set(PARACL_COMPILER_SOURCES
    src/frontend/dumper.cc src/frontend/analysis/function_explorer.cc
    src/frontend/analysis/semantic_analyzer.cc src/frontend/ast_copier.cc)

add_library(
  paracl_compiler STATIC ${PARACL_COMPILER_SOURCES} ${BISON_parser_OUTPUTS}
                         ${FLEX_scanner_OUTPUTS})

target_include_directories(paracl_compiler PUBLIC include
                                                  ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(paracl_compiler PUBLIC ezvis graphs fmt::fmt)
target_enable_linter(paracl_compiler)

add_executable(pclc src/pclc.cc)
target_link_libraries(pclc PRIVATE ezvis popl paracl_compiler bytecode_vm)
target_enable_linter(pclc)

set(SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
find_program(BASH_PROGRAM bash)
if(BASH_PROGRAM)
  enable_testing()
  add_subdirectory(test)
endif()
