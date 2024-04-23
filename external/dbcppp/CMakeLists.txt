include(FetchContent)
set(Boost_NO_SYSTEM_PATHS OFF CACHE BOOL "Do not search system for Boost" )
set(build_tests OFF CACHE BOOL "Do not build dbcppp tests")
set(build_examples OFF CACHE BOOL "Do not build dbcppp examples")
FetchContent_Declare(
	dbcppp
	GIT_REPOSITORY https://github.com/xR3b0rn/dbcppp
	GIT_TAG e5a193797949f9e39e6eb15c3c5ff51ab814fd66
)
FetchContent_MakeAvailable(dbcppp)