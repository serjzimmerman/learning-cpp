set(QUERIES_SOURCES src/intersect.cc)

add_executable(intersect ${QUERIES_SOURCES})
target_link_libraries(intersect throttle)
if(Boost_FOUND)
  target_link_libraries(intersect Boost::program_options)
endif()

install(TARGETS intersect DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(BASH_PROGRAM AND NOT HW3D_DISABLE_TESTS__)
  add_test(
    NAME test.intersect
    COMMAND
      ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
      "$<TARGET_FILE:intersect>" ${CMAKE_CURRENT_SOURCE_DIR}
      "$<TARGET_FILE:comp>")
endif()
