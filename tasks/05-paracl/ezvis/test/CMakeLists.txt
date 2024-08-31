set(TEST_SOURCES ezvis.cc)
find_package(GTest REQUIRED)

add_executable(ezvis-test ${TEST_SOURCES})
target_link_libraries(ezvis-test PRIVATE ${GTEST_BOTH_LIBRARIES} ezvis)
gtest_discover_tests(ezvis-test)
