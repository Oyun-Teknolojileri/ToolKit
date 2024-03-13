cmake_minimum_required(VERSION 3.21.0)

set(PROJECT_NAME "__projectname__" CACHE STRING "What is the project name ?")
set(TOOLKIT_DIR "__tkdir__" CACHE FILEPATH "Where is the ToolKit source directory ?")

if (EMSCRIPTEN)
	set(WEB_BUILD TRUE)
else()
	set(WEB_BUILD FALSE)
endif()

if (NOT ${WEB_BUILD} AND NOT DEFINED WIN_BUILD)
	set(EDITOR_BUILD TRUE)
else()
	set(EDITOR_BUILD FALSE)
endif()

if (${EDITOR_BUILD})
	set(TKLibName ToolKit)
	set(TKDebugLibName ToolKit_d)
else ()
	set(TKLibName TKLib)
	set(TKDebugLibName TKLib_d)
endif ()

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
	"${TOOLKIT_DIR}/ToolKit"
	"${TOOLKIT_DIR}/Dependency"
	"${TOOLKIT_DIR}/Dependency/glm"
	"${TOOLKIT_DIR}/Dependency/glad"
	"${TOOLKIT_DIR}/Dependency/SDL2/include"
	"${TOOLKIT_DIR}/Dependency/RapidXml"
	"${TOOLKIT_DIR}/Dependency/stb"
	"${TOOLKIT_DIR}/Dependency/zlib-1.2.12/dist/include"
	"${CMAKE_SOURCE_DIR}"
	"${CMAKE_SOURCE_DIR}/../Windows"
	"${CMAKE_SOURCE_DIR}/../Web"
)

set(
	SOURCE 
	Game.cpp
)

if (${WEB_BUILD} OR DEFINED WIN_BUILD)
	set(SOURCE ${SOURCE} ../Codes/Main.cpp)
	add_definitions(-DGAME_BUILD=1)
endif()

set(
	HEADERS 
	Game.h
)

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
 "${TOOLKIT_DIR}/Dependency/glm/glm.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/ext.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtx/scalar_relational.hpp"
 )

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

if (${WEB_BUILD})
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin")

	add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})
	target_link_libraries(${PROJECT_NAME} debug "${TOOLKIT_DIR}/Bin/TKLib_d.a")
	target_link_libraries(${PROJECT_NAME} optimized "${TOOLKIT_DIR}/Bin/TKLib.a")
	target_link_libraries(${PROJECT_NAME} "${TOOLKIT_DIR}/Dependency/zlib-1.2.12/dist/libminizip.a")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TK_CXX_FLAGS} -s USE_SDL=2")

	if (EXISTS "../MinResources.pak")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -o ${PROJECT_NAME}.html --shell-file ../Codes/Bin/shell_minimal.html --preload-file ../MinResources.pak@/../MinResources.pak --preload-file ../Config@/../Config -sALLOW_MEMORY_GROWTH=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3 -s USE_ZLIB=1 -sUSE_SDL=2")
	else()
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -o ${PROJECT_NAME}.html --shell-file ../Codes/Bin/shell_minimal.html --preload-file ../Resources@/../Resources --preload-file ../Config@/../Config -sALLOW_MEMORY_GROWTH=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3 -s USE_ZLIB=1 -s USE_SDL=2")
	endif()
else() # Windows & Editor builds.

	link_directories("${TOOLKIT_DIR}/Bin")
	link_directories("${TOOLKIT_DIR}/Dependency/zlib-1.2.12/dist/lib")
	link_directories("${TOOLKIT_DIR}/Dependency/SDL2/lib")
	
	if (DEFINED WIN_BUILD)
		add_executable(${PROJECT_NAME} WIN32 ${SOURCE})
	else() # Editor or game build
		add_library(${PROJECT_NAME} SHARED ${SOURCE} ${HEADERS})
	endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${PROJECT_NAME} PRIVATE TK_DLL_IMPORT)
endif()
	target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:TK_DEBUG>)

	if (MSVC)
		target_link_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:/INCREMENTAL:NO> $<$<NOT:$<CONFIG:DEBUG>>:/LTCG>)
	endif()

	target_link_libraries(${PROJECT_NAME} PRIVATE debug ${TKDebugLibName})
	target_link_libraries(${PROJECT_NAME} PRIVATE optimized ${TKLibName})
	target_link_libraries(${PROJECT_NAME} PRIVATE zlibstatic minizip)
	target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 SDL2main)
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE ${PCH})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin$<0:>)

#For hot reload rename the previous output
if (${EDITOR_BUILD})
	set(DLL $<SHELL_PATH:$<TARGET_FILE:${PROJECT_NAME}>>)
	set(PDB $<SHELL_PATH:$<TARGET_PDB_FILE:${PROJECT_NAME}>>)

	add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -Ddll=${DLL} -Dpdb=${PDB} -Dtmp="${OUTDIR}/tmp" -P "${CMAKE_SOURCE_DIR}/CMakeHotReload.cmake")
endif()
