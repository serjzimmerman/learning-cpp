set(LIBRARY_SOURCES src/bs_order_tree.cc)

add_library(throttle ${LIBRARY_SOURCES})
target_include_directories(throttle PUBLIC include)

set(SPLAY_TREE_TEST test/test_private.cc)

if(ENABLE_GTEST)
  add_executable(splay_order_test ${SPLAY_TREE_TEST})
  target_include_directories(splay_order_test PRIVATE src include)
  target_link_libraries(splay_order_test throttle ${GTEST_BOTH_LIBRARIES})
  gtest_discover_tests(splay_order_test)
endif()
