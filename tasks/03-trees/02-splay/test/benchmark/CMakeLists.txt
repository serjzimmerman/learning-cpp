set(BENCHMARK_SOURCES src/benchmark.cc)

option(LINEAR OFF)

add_executable(benchmark ${BENCHMARK_SOURCES})
target_link_libraries(benchmark throttle)
if(Boost_FOUND)
  target_link_libraries(benchmark Boost::program_options)
endif()
if(LINEAR)
  target_compile_definitions(benchmark PRIVATE -DLINEAR_COMPLEXITY)
endif()

set(ANSGEN_SOURCES src/ansgen.cc)

set(TINY_SOURCES src/tinybenchmark.cc)

add_executable(tinybenchmark ${TINY_SOURCES})
target_link_libraries(tinybenchmark throttle)

add_executable(ansgen ${ANSGEN_SOURCES})
install(TARGETS ansgen benchmark tinybenchmark
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(BASH_PROGRAM)
  add_custom_target(generate_ans)
  add_custom_command(
    TARGET generate_ans
    COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/ansgen.sh
            "$<TARGET_FILE:ansgen>" ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(BASH_PROGRAM)
  add_test(NAME test.benchmark
           COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
                   "$<TARGET_FILE:benchmark>" ${CMAKE_CURRENT_SOURCE_DIR})
endif()
