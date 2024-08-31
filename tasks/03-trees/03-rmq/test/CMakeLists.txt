find_package(Boost COMPONENTS program_options)

if(Boost_FOUND)
  add_compile_definitions(BOOST_FOUND__)
endif()

add_subdirectory(rmq-queries)
add_subdirectory(benchmark)

enable_testing()
