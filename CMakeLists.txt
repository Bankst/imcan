cmake_minimum_required(VERSION 3.18)
project(imcan LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20 REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)

find_package(fmt REQUIRED)

# External deps
add_subdirectory(external/allwpilib)

add_executable(${PROJECT_NAME})

if (CLANG_TIDY_EXE)
	message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
	set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}"
		"--header-filter=${CMAKE_CURRENT_LIST_DIR}/inc/*"
		"--config-file=${CMAKE_CURRENT_LIST_DIR}/.clang-tidy"
	)
	set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
else()
	message(STATUS "clang-tidy not found.")
endif()

add_subdirectory(dbcan)

target_sources(${PROJECT_NAME} PRIVATE
	src/main.cpp
	src/Dbc.cpp
	src/DbcMessageView.cpp
	src/GuiHelpers.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
	inc/
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	libglass
	dbcan
	portable_file_dialogs
	rt
	fmt
)