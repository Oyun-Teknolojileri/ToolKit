cmake_minimum_required(VERSION 3.0.0)

include_directories(
	"${CMAKE_SOURCE_DIR}/ToolKit"
	"${CMAKE_SOURCE_DIR}/Dependency"
	"${CMAKE_SOURCE_DIR}/Dependency/glad"
	"${CMAKE_SOURCE_DIR}/Dependency/mini_audio/"
	"${CMAKE_SOURCE_DIR}/Dependency/RapidXml"
	"${CMAKE_SOURCE_DIR}/Dependency/stb"
	"${CMAKE_SOURCE_DIR}/Dependency/zlib-1.2.12/dist/include"
)

if (CLANG_COMPILER)
	add_definitions(-include stdafx.h)

	# Huge amount of warning messages appears in the output when compiling
	# If there is an error, it is too difficult to notice.
	# Remove -w flag if you want to see the warnings
	SET(TK_CXX_FLAGS " -std=c++17 -w")
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

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/ToolKit/*.cpp")
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TK_CXX_FLAGS}")

set(CMAKE_DEBUG_POSTFIX _d)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	add_library(ToolKit STATIC ${SOURCES})
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	link_directories("${CMAKE_SOURCE_DIR}/Dependency/Glew/lib/Release/x64")
	link_directories("${CMAKE_SOURCE_DIR}/Dependency/zlib-1.2.12/dist/lib")

  add_library(ToolKit SHARED ${SOURCES})

	target_link_libraries(ToolKit zlibstatic minizip)
	target_link_libraries(ToolKit OpenGL32)
endif ()

set_target_properties(ToolKit PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin$<0:>
		LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin$<0:>
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin$<0:>
)

if (MSVC_COMPILER)
    target_precompile_headers(ToolKit PRIVATE ${PCH_HEADER})
endif()