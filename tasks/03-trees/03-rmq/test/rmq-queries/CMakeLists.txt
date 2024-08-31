set(QUERIES_SOURCES src/rmq-queries.cc)

add_executable(rmq-queries ${QUERIES_SOURCES})
target_link_libraries(rmq-queries throttle)
if(Boost_FOUND)
  target_link_libraries(rmq-queries Boost::program_options)
endif()

install(TARGETS rmq-queries DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(BASH_PROGRAM)
  add_test(NAME test.rmq-queries
           COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
                   "$<TARGET_FILE:rmq-queries>" ${CMAKE_CURRENT_SOURCE_DIR})
endif()
