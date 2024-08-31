set(INTERSECT_FCL_SOURCES src/intersect_fcl.cc)

FetchContent_Declare(
  fcl GIT_REPOSITORY https://github.com/flexible-collision-library/fcl.git)

FetchContent_GetProperties(fcl)
if(NOT cmark_POPULATED)
  FetchContent_Populate(fcl)
  add_subdirectory(${fcl_SOURCE_DIR} ${fcl_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(intersect_fcl ${INTERSECT_FCL_SOURCES})
target_link_libraries(intersect_fcl throttle fcl)
if(Boost_FOUND)
  target_link_libraries(intersect_fcl Boost::program_options)
endif()

install(TARGETS intersect_fcl DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(BASH_PROGRAM)
  add_test(
    NAME test.intersect_fcl
    COMMAND
      ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
      "$<TARGET_FILE:intersect_fcl>" ${CMAKE_CURRENT_SOURCE_DIR}
      "$<TARGET_FILE:comp>")
endif()
