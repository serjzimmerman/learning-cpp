set(LFUDAC_SOURCES src/lfudac.cc)

add_executable(lfudac ${LFUDAC_SOURCES})
if(Boost_FOUND)
  target_link_libraries(lfudac Boost::program_options)
endif()
target_link_libraries(lfudac caches)

install(TARGETS lfudac DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(BASH_PROGRAM)
  add_test(NAME test.lfuda
           COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
                   "$<TARGET_FILE:lfudac>" ${CMAKE_CURRENT_SOURCE_DIR})
endif()
