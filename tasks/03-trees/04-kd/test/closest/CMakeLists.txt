set(CL_SOURCES src/closest.cc)

set(OD_SOURCES src/onedim.cc)

add_executable(closest ${CL_SOURCES})
target_link_libraries(closest throttle range-v3::range-v3 ${Boost_LIBRARIES})

add_executable(onedim ${OD_SOURCES})
target_link_libraries(onedim throttle range-v3::range-v3)

install(TARGETS closest onedim DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(BASH_PROGRAM)
  add_test(NAME test.closest
           COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
                   "$<TARGET_FILE:closest>" ${CMAKE_CURRENT_SOURCE_DIR})
endif()
