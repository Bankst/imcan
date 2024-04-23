cmake_minimum_required(VERSION 3.18)
project(imcan LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20 REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external/allwpilib)
add_subdirectory(external/dbcppp)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
	src/main.cpp
	src/Dbc.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
	inc/
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	libglass
	# cpp-can-parser
	portable_file_dialogs
	rt
)
