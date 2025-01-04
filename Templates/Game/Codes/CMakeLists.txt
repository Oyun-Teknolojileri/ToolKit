cmake_minimum_required(VERSION 3.21.0)

set(GAME_PROJECT_NAME "__projectname__" CACHE STRING "What is the project name ?")
set(TOOLKIT_DIR "__tkdir__" CACHE FILEPATH "Where is the ToolKit source directory ?")

if (NOT WEB_BUILD AND NOT DEFINED WIN_BUILD AND NOT DEFINED ANDROID_BUILD)
	set(EDITOR_BUILD TRUE)
else()
	set(EDITOR_BUILD FALSE)
endif()

if (NOT DEFINED GAME_CODE_DIR)
    set(GAME_CODE_DIR ${CMAKE_SOURCE_DIR})
endif()

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

project (${GAME_PROJECT_NAME})

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
	"${TOOLKIT_DIR}/Dependency/SDL2/include"
	"${TOOLKIT_DIR}/Dependency/RapidXml"
	"${TOOLKIT_DIR}/Dependency/stb"
	"${TOOLKIT_DIR}/Dependency/minizip-ng/dist/include"
	"${GAME_CODE_DIR}"
	"${GAME_CODE_DIR}/../Windows"
	"${GAME_CODE_DIR}/../Web"
	"${GAME_CODE_DIR}/../Android/app/jni/src"
)

set(
	SOURCE 
	Game.cpp 
)

if (WEB_BUILD OR DEFINED WIN_BUILD OR DEFINED ANDROID_BUILD)
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
 "${TOOLKIT_DIR}/ToolKit/stdafx.h"
 "${TOOLKIT_DIR}/ToolKit/Types.h"
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

if (WEB_BUILD)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin")

	add_executable(${GAME_PROJECT_NAME} ${SOURCE} ${HEADERS})
	target_link_libraries(${GAME_PROJECT_NAME} debug "${TOOLKIT_DIR}/Bin/ToolKitStatic_d.a")
	target_link_libraries(${GAME_PROJECT_NAME} optimized "${TOOLKIT_DIR}/Bin/ToolKitStatic.a")
	target_link_libraries(${GAME_PROJECT_NAME} "${TOOLKIT_DIR}/Dependency/minizip-ng/dist/lib/libminizip.a")
	target_link_libraries(${GAME_PROJECT_NAME} "${TOOLKIT_DIR}/Dependency/minizip-ng/dist/lib/libzstd.a")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TK_CXX_FLAGS} -sUSE_SDL=2")

	if (EXISTS "../MinResources.pak")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -o ${PROJECT_NAME}.html --shell-file ../Codes/Bin/shell_minimal.html --preload-file ../MinResources.pak@/../MinResources.pak 	--preload-file ../Config@/../Config -sALLOW_MEMORY_GROWTH=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3 -sSTACK_SIZE=131072")
	else()
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -o ${PROJECT_NAME}.html --shell-file ../Codes/Bin/shell_minimal.html --preload-file ../Resources@/../Resources 				--preload-file ../Config@/../Config -sALLOW_MEMORY_GROWTH=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3 -sSTACK_SIZE=131072")
	endif()
else() # Windows & Editor builds.

	link_directories("${TOOLKIT_DIR}/Bin")
	link_directories("${TOOLKIT_DIR}/Dependency/minizip-ng/dist/lib")
	link_directories("${TOOLKIT_DIR}/Dependency/SDL2/lib")
	
	find_library(minizip minizip)
	find_library(libzstd_static libzstd_static)
	find_library(ToolKitStatic ToolKitStatic)

	if (${EDITOR_BUILD})
		add_library(${GAME_PROJECT_NAME} SHARED ${SOURCE} ${HEADERS})
		target_link_libraries(${GAME_PROJECT_NAME} PRIVATE debug Editor_d)
		target_link_libraries(${GAME_PROJECT_NAME} PRIVATE optimized Editor)
		target_link_libraries(${GAME_PROJECT_NAME} PRIVATE debug ToolKit_d)
		target_link_libraries(${GAME_PROJECT_NAME} PRIVATE optimized ToolKit)
		target_compile_definitions(${GAME_PROJECT_NAME} PRIVATE TK_WITH_EDITOR TK_DLL_IMPORT)
	elseif(DEFINED WIN_BUILD)
		add_executable(${GAME_PROJECT_NAME} WIN32 ${SOURCE})

		find_library(OpenGL32 OpenGL32)
		target_link_libraries(${GAME_PROJECT_NAME} PRIVATE SDL2 SDL2main minizip zstd_static OpenGL32)
		target_link_libraries(${GAME_PROJECT_NAME} PRIVATE debug ToolKitStatic_d)
		target_link_libraries(${GAME_PROJECT_NAME} PRIVATE optimized ToolKitStatic)
	elseif (DEFINED ANDROID_BUILD)
		# Reset game project name to main if it is android build, sdl expect a libmain.so
		set(GAME_PROJECT_NAME "main")
		add_library(${GAME_PROJECT_NAME} SHARED ${SOURCE} ${HEADERS})
		
		find_library(GLESv3 GLESv3)
		target_link_libraries(${GAME_PROJECT_NAME} PRIVATE GLESv3 jnigraphics android log SDL2 SDL2main libzstd_static minizip ToolKitStatic)
	endif()

	target_compile_definitions(${GAME_PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:TK_DEBUG>)

	if (MSVC)
		target_link_options(${GAME_PROJECT_NAME} PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:/INCREMENTAL:NO> $<$<NOT:$<CONFIG:DEBUG>>:/LTCG>)
	endif()
endif()

target_precompile_headers(${GAME_PROJECT_NAME} PRIVATE ${PCH})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin$<0:>)

#For hot reload rename the previous output
if (${EDITOR_BUILD})
	set(DLL $<SHELL_PATH:$<TARGET_FILE:${GAME_PROJECT_NAME}>>)
	set(PDB $<SHELL_PATH:$<TARGET_PDB_FILE:${GAME_PROJECT_NAME}>>)

	add_custom_command(TARGET ${GAME_PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -Ddll=${DLL} -Dpdb=${PDB} -Dtmp="${OUTDIR}/tmp" -P "${CMAKE_SOURCE_DIR}/CMakeHotReload.cmake")
endif()
