set(COMP_SOURCES src/comp.cc)

add_executable(comp ${COMP_SOURCES})
target_link_libraries(comp throttle)
if(Boost_FOUND)
  target_link_libraries(comp Boost::program_options)
endif()

install(TARGETS comp DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
