cmake_minimum_required(VERSION 3.11)
project(vulkan)

option(INSOURCEBUILD OFF)

if((${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR}) AND NOT
                                                              ${INSOURCEBUILD})
  message(
    FATAL_ERROR
      "In-source building disabled. Provide -DINSOURCEBUILD option if you are sure about it."
  )
endif()

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)

# Possibly support windows and MSVC compiler
if(MSVC)
  add_compile_options(/W3)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Boost REQUIRED COMPONENTS program_options)

if(${Boost_FOUND})
  add_compile_definitions(BOOST_FOUND__)
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

set(HW3D_DISABLE_TESTS__
    ON
    CACHE BOOL "Disable hw3d tests" FORCE)
add_subdirectory(../01-hw3d ${CMAKE_CURRENT_BINARY_DIR}/hw3d EXCLUDE_FROM_ALL)

find_package(glm REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Vulkan 1.3 REQUIRED)
find_program(
  glslc
  NAMES glslc
  HINTS Vulkan::glslc REQUIRED)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG v1.89.2)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
endif()

FetchContent_MakeAvailable(imgui)

function(add_spirv_shader TARGET_NAME INPUT_FILE)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.spv
    COMMAND ${glslc} -c ${INPUT_FILE} -o
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.spv
    MAIN_DEPENDENCY ${INPUT_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_custom_target(${TARGET_NAME} ALL
                    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.spv)

  set(${TARGET_NAME}_OUTPUTS
      ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.spv
      PARENT_SCOPE)
endfunction()

set(VK_VALIDATION_LAYER
    OFF
    CACHE BOOL "Enables Vulkan validation layer")

set(EZVK_SOURCES src/ezvk/window.cc)

add_library(ezvk ${EZVK_SOURCES})
if(VK_VALIDATION_LAYER)
  message("Vulkan validation layer enabled")
  add_compile_definitions(VK_VALIDATION_LAYER)
endif()

target_include_directories(ezvk PUBLIC include/ezvk include/unified_includes)
target_link_libraries(ezvk PUBLIC spdlog::spdlog)

find_package(Threads REQUIRED)

set(APPLICATION_SOURCES
    app/triangles.cc
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    app/gui.cc
    app/application.cc)

add_spirv_shader(triangles_vert shaders/triangles.vert)
add_spirv_shader(triangles_frag shaders/triangles.frag)
add_spirv_shader(wireframe_vert shaders/wireframe.vert)
add_spirv_shader(wireframe_frag shaders/wireframe.frag)
add_custom_target(shaders ALL DEPENDS triangles_vert triangles_frag
                                      wireframe_vert wireframe_frag)

add_executable(triangles ${APPLICATION_SOURCES})
target_include_directories(
  triangles PUBLIC ${imgui_SOURCE_DIR} include/unified_includes include/app
                   app/)

add_custom_command(
  TARGET triangles
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory
          $<TARGET_FILE_DIR:triangles>/shaders
  COMMAND
    ${CMAKE_COMMAND} -E copy ${triangles_vert_OUTPUTS}
    ${triangles_frag_OUTPUTS} ${wireframe_vert_OUTPUTS}
    ${wireframe_frag_OUTPUTS} $<TARGET_FILE_DIR:triangles>/shaders/)

target_link_libraries(
  triangles
  PUBLIC glfw
         Vulkan::Vulkan
         ezvk
         glm::glm
         throttle
         Boost::program_options
         Threads::Threads)
add_dependencies(triangles shaders)
