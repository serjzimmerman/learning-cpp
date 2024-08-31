set(QUERIES_SOURCES src/benchmark.cc)

add_executable(benchmark ${QUERIES_SOURCES})
target_link_libraries(benchmark throttle)
if(Boost_FOUND)
  target_link_libraries(benchmark Boost::program_options)
endif()

install(TARGETS benchmark DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
