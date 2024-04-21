cmake_minimum_required(VERSION 3.21.0)

if (CLANG_COMPILER)
	add_definitions(-include stdafx.h)

	SET(TK_CXX_FLAGS " -std=c++17")
	if (NOT TK_CXX_EXTRA STREQUAL "")
		SET(TK_CXX_FLAGS "${TK_CXX_FLAGS} ${TK_CXX_EXTRA}")
	endif()
endif()

if(MSVC_COMPILER)
	set(CMAKE_CXX_STANDARD 17)
	set(PCH_HEADER "stdafx.h")
	set(PCH_SOURCE "stdafx.cpp")
	add_compile_options(/D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(
	"${CMAKE_SOURCE_DIR}/ToolKit"
	"${CMAKE_SOURCE_DIR}/Dependency"
	"${CMAKE_SOURCE_DIR}/Dependency/glm"
	"${CMAKE_SOURCE_DIR}/Dependency/glad"
	"${CMAKE_SOURCE_DIR}/Dependency/mini_audio/"
	"${CMAKE_SOURCE_DIR}/Dependency/RapidXml"
	"${CMAKE_SOURCE_DIR}/Dependency/stb"
	"${CMAKE_SOURCE_DIR}/Dependency/zlib-1.2.12/dist/include"
)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/ToolKit/*.cpp")

link_directories("${CMAKE_SOURCE_DIR}/Dependency/zlib-1.2.12/dist/lib")

set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TK_CXX_FLAGS}")
set(CMAKE_DEBUG_POSTFIX _d)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	message("Release configuration")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
	message("Debug configuration")
	add_definitions(-DTK_DEBUG)
endif ()

if (NOT WEB_BUILD)
	set(TK_LINK_LIB zlibstatic minizip OpenGL32)
endif()

add_library(ToolKitStatic STATIC ${SOURCES})
target_link_libraries(ToolKitStatic PRIVATE ${TK_LINK_LIB})

set_target_properties(ToolKitStatic PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin$<0:>
		LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin$<0:>
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin$<0:>
)

if (MSVC_COMPILER)
    target_precompile_headers(ToolKitStatic PRIVATE ${PCH_HEADER})
endif()