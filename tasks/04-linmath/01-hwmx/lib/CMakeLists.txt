add_library(throttle INTERFACE)
target_include_directories(throttle INTERFACE include)

set(UNIT_TEST_SOURCES test/test_vector.cc test/test_contiguous_matrix.cc
                      test/test_matrix.cc test/main.cc)

if(ENABLE_GTEST)
  add_executable(unit_test ${UNIT_TEST_SOURCES})
  target_include_directories(unit_test PRIVATE src include)
  target_link_libraries(unit_test throttle ${GTEST_BOTH_LIBRARIES})
  gtest_discover_tests(unit_test)
endif()

include(FetchContent)
find_package(range-v3)

if(NOT range-v3_FOUND)
  FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3
    GIT_TAG master)

  if(NOT range-v3_POPULATED)
    FetchContent_Populate(range-v3)
    add_subdirectory(${range-v3_SOURCE_DIR} ${range-v3_BINARY_DIR}
                     EXCLUDE_FROM_ALL)
  endif()

  FetchContent_MakeAvailable(range-v3)
endif()

target_link_libraries(throttle INTERFACE range-v3::range-v3)
