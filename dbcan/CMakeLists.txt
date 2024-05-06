cmake_minimum_required(VERSION 3.18)
project(dbcan LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20 REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(dbcan STATIC)

target_sources(${PROJECT_NAME} PUBLIC
	src/DbcMessage.cpp
	src/DbcNetwork.cpp
	src/DbcSignal.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
	inc/
)

target_link_libraries(${PROJECT_NAME} PUBLIC
	fmt
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	-DUSE_CTRE
)

add_executable(${PROJECT_NAME}_test)
target_sources(${PROJECT_NAME}_test PUBLIC
	src/Test.cpp
)
target_link_libraries(${PROJECT_NAME}_test PRIVATE
	${PROJECT_NAME}
)