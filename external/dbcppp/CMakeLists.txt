# include(FetchContent)

# FetchContent_Declare(
# 	dbcppp
# 	GIT_REPOSITORY https://github.com/xR3b0rn/dbcppp
# 	GIT_TAG e5a193797949f9e39e6eb15c3c5ff51ab814fd66
# )
# FetchContent_MakeAvailable(dbcppp)

configure_file(CMakeLists.txt.in dbcppp-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dbcppp-download)

if(result)
  message(FATAL_ERROR "CMake step for dbcppp failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dbcppp-download)

if(result)
  message(FATAL_ERROR "Build step for dbcppp failed: ${result}")
endif()

set(Boost_NO_SYSTEM_PATHS OFF CACHE BOOL "" FORCE)
set(build_tests OFF CACHE BOOL "" FORCE)
set(build_tests OFF CACHE BOOL "" FORCE)
set(build_examples OFF CACHE BOOL "" FORCE)
set(build_kcd OFF CACHE BOOL "" FORCE)

message(STATUS "Now Adding DBCPPP")

# Add allwpilib directly to our build.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/dbcppp-src
                 ${CMAKE_CURRENT_BINARY_DIR}/dbcppp-build
                 EXCLUDE_FROM_ALL)