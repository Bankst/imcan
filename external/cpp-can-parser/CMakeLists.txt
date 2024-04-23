cmake_minimum_required(VERSION 3.18)
project(cpp-can-parser LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
	src/CANDatabase.cpp
	src/CANFrame.cpp
	src/CANFrameAnalysis.cpp
	src/CANSignal.cpp
	src/DBCParser.cpp
	src/ParsingUtils.cpp
	src/Tokenizer.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
	inc/
)