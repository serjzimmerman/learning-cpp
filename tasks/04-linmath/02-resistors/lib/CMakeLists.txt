add_library(throttle INTERFACE)
target_include_directories(throttle INTERFACE include)

set(UNIT_TEST_SOURCES
    test/test_vector.cc test/test_contiguous_matrix.cc test/test_matrix.cc
    test/test_linear_solver.cc test/test_ud_assymetric_graph.cc test/main.cc)

if(ENABLE_GTEST AND NOT RESISTORS_NO_TESTING__)
  add_executable(unit_test ${UNIT_TEST_SOURCES})
  target_include_directories(unit_test PRIVATE src include)
  target_link_libraries(unit_test throttle ${GTEST_BOTH_LIBRARIES})
  gtest_discover_tests(unit_test)
endif()
