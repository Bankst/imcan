cmake_minimum_required(VERSION 3.18)
project(imcan LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20 REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Clang-Tidy
find_program(CLANG_TIDY_EXE NAMES "clang-tidy" REQUIRED)
set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}")

# External deps
add_subdirectory(external/allwpilib)
message(STATUS "Adding dbcppp")
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
	dbcppp
	portable_file_dialogs
	rt
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")