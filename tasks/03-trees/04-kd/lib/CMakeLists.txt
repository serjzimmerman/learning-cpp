set(LIBRARY_SOURCES src/bs_order_tree.cc)

add_library(throttle INTERFACE)
target_include_directories(throttle INTERFACE include)
target_link_libraries(throttle INTERFACE range-v3::range-v3)

set(THROTTLE_TEST test/main.cc)

if(ENABLE_GTEST)
  add_executable(unit-test-throttle ${THROTTLE_TEST})
  target_include_directories(unit-test-throttle PRIVATE src include)
  target_link_libraries(unit-test-throttle throttle ${GTEST_BOTH_LIBRARIES})
  gtest_discover_tests(unit-test-throttle)
endif()
