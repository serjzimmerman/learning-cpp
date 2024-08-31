find_package(Boost COMPONENTS program_options)

if(Boost_FOUND)
  add_compile_definitions(BOOST_FOUND__)
endif()

add_subdirectory(benchmark)
add_subdirectory(queries)

enable_testing()
