cmake_minimum_required(VERSION 3.21.0)

if (CLANG_COMPILER)
	add_definitions(-include stdafx.h)

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

include_directories(
	"${TOOLKIT_DIR}/ToolKit"
	"${TOOLKIT_DIR}/Dependency"
	"${TOOLKIT_DIR}/Dependency/glm"
	"${TOOLKIT_DIR}/Dependency/glad"
	"${TOOLKIT_DIR}/Dependency/mini_audio"
	"${TOOLKIT_DIR}/Dependency/RapidXml"
	"${TOOLKIT_DIR}/Dependency/stb"
	"${TOOLKIT_DIR}/Dependency/minizip-ng"
	"${TOOLKIT_DIR}/Dependency/minizip-ng/dist/include"
	"${TOOLKIT_DIR}/Dependency/poolSTL/include"
)

file(GLOB SOURCES "${TOOLKIT_DIR}/ToolKit/*.cpp")

link_directories("${TOOLKIT_DIR}/Dependency/minizip-ng/dist/lib")

find_library(minizip libzstd_static ToolKitStatic)

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
	set(TK_LINK_LIB libzstd_static minizip)
	if (NOT ANDROID_BUILD)
		set(TK_LINK_LIB ${TK_LINK_LIB} OpenGL32)
	else()
		set(TK_LINK_LIB ${TK_LINK_LIB} GLESv3)
	endif()
endif()

add_library(ToolKitStatic STATIC ${SOURCES})
target_link_libraries(ToolKitStatic PRIVATE ${TK_LINK_LIB})

set_target_properties(ToolKitStatic PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY ${TOOLKIT_DIR}/Bin$<0:>
		LIBRARY_OUTPUT_DIRECTORY ${TOOLKIT_DIR}/Bin$<0:>
		RUNTIME_OUTPUT_DIRECTORY ${TOOLKIT_DIR}/Bin$<0:>
)

if (MSVC_COMPILER)
    target_precompile_headers(ToolKitStatic PRIVATE ${PCH_HEADER})
endif()