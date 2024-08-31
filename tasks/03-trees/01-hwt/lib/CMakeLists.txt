set(LIBRARY_SOURCES src/rb_tree_ranged.cc)

add_library(throttle ${LIBRARY_SOURCES})
target_include_directories(throttle PUBLIC include)

set(RBT_RANGED_SOURCES test/test_private.cc)

if(ENABLE_GTEST)
  add_executable(rbt_ranged_test ${RBT_RANGED_SOURCES})
  target_include_directories(rbt_ranged_test PRIVATE src include)
  target_link_libraries(rbt_ranged_test throttle ${GTEST_BOTH_LIBRARIES})
  gtest_discover_tests(rbt_ranged_test)
endif()
