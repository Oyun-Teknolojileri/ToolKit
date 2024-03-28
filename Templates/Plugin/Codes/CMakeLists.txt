cmake_minimum_required(VERSION 3.21.0)

set(PROJECT_NAME "__projectname__" CACHE STRING "What is the project name ?")
set(TOOLKIT_DIR "__tkdir__" CACHE FILEPATH "Where is the ToolKit source directory ?")

#if not a path is provided, try fetching it from the path file.
if("${TOOLKIT_DIR}" STREQUAL "__tkdir__")
		if(DEFINED ENV{APPDATA})
			cmake_path(SET CFG_FILE_PATH "$ENV{APPDATA}/ToolKit/Config/Path.txt")
		if(NOT EXISTS ${CFG_FILE_PATH})
			message(SEND_ERROR "Set TOOLKIT_DIR or provide ${CFG_FILE_PATH} containing path to ToolKit.")
		else()
			file(STRINGS ${CFG_FILE_PATH} TOOLKIT_DIR)
		endif()
	endif()
endif()

project (${PROJECT_NAME})

string(TOLOWER ${CMAKE_CXX_COMPILER_ID} TK_COMPILER)
message("Compiling with ${TK_COMPILER}")

# Check if the compiler is Clang
if (${TK_COMPILER} STREQUAL "clang")
	set(CLANG_COMPILER TRUE)
else()
	set(CLANG_COMPILER FALSE)
endif()

# Check if the compiler is MSVC
if (${TK_COMPILER} STREQUAL "msvc")
	set(MSVC_COMPILER TRUE)
else()
	set(MSVC_COMPILER FALSE)
endif()

if(NOT EXISTS ${TOOLKIT_DIR}/ToolKit/ToolKit.h)
	message(SEND_ERROR "Can't find ToolKit.h in ${TOOLKIT_DIR}")
endif()

set(CMAKE_DEBUG_POSTFIX _d)

include_directories(
	"${TOOLKIT_DIR}"
	"${TOOLKIT_DIR}/ToolKit"
	"${TOOLKIT_DIR}/Dependency"
	"${TOOLKIT_DIR}/Dependency/glm"
	"${TOOLKIT_DIR}/Dependency/glad"
	"${TOOLKIT_DIR}/Dependency/RapidXml"
	"${TOOLKIT_DIR}/Dependency/stb"
	"${CMAKE_SOURCE_DIR}"
)

# Plugin source begin
set(
	SOURCE 
	PluginMain.cpp
)

set(
	HEADERS 
	PluginMain.h
)
# Plugin source end

# GLM begin
add_definitions(-DTK_GLM)
add_definitions(-DGLM_FORCE_QUAT_DATA_XYZW)
add_definitions(-DGLM_FORCE_XYZW_ONLY)
add_definitions(-DGLM_FORCE_CTOR_INIT)
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)
add_definitions(-DGLM_FORCE_ALIGNED_GENTYPES)
add_definitions(-DGLM_FORCE_INTRINSICS)

set(
  PCH
 "${TOOLKIT_DIR}/Dependency/glm/glm/glm.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/glm/ext.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/glm/gtx/scalar_relational.hpp"
 )
# GLM end

set(OUTDIR "${CMAKE_SOURCE_DIR}/Bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTDIR})

if (CLANG_COMPILER)
	set(TK_CXX_FLAGS " -std=c++17 -w")
endif ()

if (MSVC_COMPILER)
	set(CMAKE_CXX_STANDARD 17)
endif ()

if (NOT TK_CXX_EXTRA STREQUAL "")
	set(TK_CXX_FLAGS "${TK_CXX_FLAGS} ${TK_CXX_EXTRA}")
endif()

# Plugin includes end defines begin
set(TKLibName ToolKit)
set(TKDebugLibName ToolKit_d)
set(TKEditorLibName Editor)
set(TKDebugEditorLibName Editor_d)

target_link_libraries(${PROJECT_NAME} PRIVATE debug ${TKDebugLibName})
target_link_libraries(${PROJECT_NAME} PRIVATE optimized ${TKLibName})
link_directories("${TOOLKIT_DIR}/Bin")

add_library(${PROJECT_NAME} SHARED ${SOURCE} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE debug ${TKDebugEditorLibName})
target_link_libraries(${PROJECT_NAME} PRIVATE optimized ${TKEditorLibName})
target_compile_definitions(${PROJECT_NAME} PRIVATE TK_WITH_EDITOR)
target_compile_definitions(${PROJECT_NAME} PRIVATE TK_DLL_IMPORT)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:TK_DEBUG>)

if (MSVC)
	target_link_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:/INCREMENTAL:NO> $<$<NOT:$<CONFIG:DEBUG>>:/LTCG>)
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE ${PCH})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin$<0:>)
# Plugin includes end defines end

#For hot reload rename the previous output
if (${EDITOR_BUILD})
	set(DLL $<SHELL_PATH:$<TARGET_FILE:${PROJECT_NAME}>>)
	set(PDB $<SHELL_PATH:$<TARGET_PDB_FILE:${PROJECT_NAME}>>)

	add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -Ddll=${DLL} -Dpdb=${PDB} -Dtmp="${OUTDIR}/tmp" -P "${CMAKE_SOURCE_DIR}/CMakeHotReload.cmake")
endif()
