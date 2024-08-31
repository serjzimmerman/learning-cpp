set(QUERIES_SOURCES src/queries.cc)

add_executable(queries ${QUERIES_SOURCES})
target_link_libraries(queries throttle)
if(Boost_FOUND)
  target_link_libraries(queries Boost::program_options)
endif()

install(TARGETS queries DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(BASH_PROGRAM)
  add_test(NAME test.queries
           COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
                   "$<TARGET_FILE:queries>" ${CMAKE_CURRENT_SOURCE_DIR})
endif()
