find_package(Boost COMPONENTS program_options)

if(Boost_FOUND)
  add_compile_definitions(BOOST_FOUND__)
endif()

add_subdirectory(intersect)
add_subdirectory(comp-unordered)

option(BUILD_FCL_REFERENCE OFF)
if(${BUILD_FCL_REFERENCE})
  add_subdirectory(fcl-reference)
endif()

enable_testing()
