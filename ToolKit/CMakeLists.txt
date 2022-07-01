cmake_minimum_required(VERSION 3.0.0)

include_directories(
	"${CMAKE_SOURCE_DIR}/ToolKit"
	"${CMAKE_SOURCE_DIR}/Dependency"
	"${CMAKE_SOURCE_DIR}/Dependency/OpenAL/include"
	"${CMAKE_SOURCE_DIR}/Dependency/RapidXml"
	"${CMAKE_SOURCE_DIR}/Dependency/stb"
	"${CMAKE_SOURCE_DIR}/Dependency/zlib-1.2.12/dist/include"
)

add_definitions(-include stdafx.h)

SET(TK_CXX_FLAGS " -std=c++17")
if (NOT TK_CXX_EXTRA STREQUAL "")
	SET(TK_CXX_FLAGS "${TK_CXX_FLAGS} ${TK_CXX_EXTRA}")
endif()

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/ToolKit/*.cpp")
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TK_CXX_FLAGS}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
add_library(ToolKit STATIC ${SOURCES})
