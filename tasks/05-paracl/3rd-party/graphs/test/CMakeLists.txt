find_package(GTest REQUIRED)

set(GRAPHS_TEST_SOURCES src/main.cc src/dag.cc src/directed_graph.cc)
add_executable(graphs_unit_test ${GRAPHS_TEST_SOURCES})
target_link_libraries(graphs_unit_test PRIVATE ${GTEST_BOTH_LIBRARIES} graphs)

gtest_discover_tests(graphs_unit_test)
