cmake_minimum_required(VERSION 3.0.0)

include_directories(
	"${CMAKE_SOURCE_DIR}/Source"
	"${CMAKE_SOURCE_DIR}/Dependency"
	"${CMAKE_SOURCE_DIR}/Dependency/OpenAL/include"
	"${CMAKE_SOURCE_DIR}/Dependency/RapidXml"
	"${CMAKE_SOURCE_DIR}/Dependency/SDL2/include"
	"${CMAKE_SOURCE_DIR}/Dependency/stb"
)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/Source/*.cpp")
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O0 -g")

add_library(ToolKit STATIC ${SOURCES})