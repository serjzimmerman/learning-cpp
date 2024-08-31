find_package(Boost COMPONENTS program_options)

if(Boost_FOUND)
  add_compile_definitions(BOOST_FOUND__)
endif()

add_subdirectory(lfuc)
add_subdirectory(belady)
add_subdirectory(lfudac)

enable_testing()
