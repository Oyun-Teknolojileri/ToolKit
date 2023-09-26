cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME "__projectname__" CACHE STRING "What is the project name ?")
set(TOOLKIT_DIR "__tkdir__" CACHE FILEPATH "Where is the ToolKit source directory ?")

if (EMSCRIPTEN)
	set(WEB_BUILD TRUE)
else()
	set(WEB_BUILD FALSE)
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

project (${PROJECT_NAME})

# Check if the compiler is Clang
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CLANG_COMPILER TRUE)
else ()
    set(CLANG_COMPILER FALSE)
endif ()

# Check if the compiler is MSVC
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  set(MSVC_COMPILER TRUE)
else ()
  set(MSVC_COMPILER FALSE)
endif ()

if(NOT EXISTS ${TOOLKIT_DIR}/ToolKit/ToolKit.h)
	message(SEND_ERROR "Can't find ToolKit.h in ${TOOLKIT_DIR}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${TOOLKIT_DIR}/Bin")
set(CMAKE_DEBUG_POSTFIX _d)

include_directories(
	"${TOOLKIT_DIR}/ToolKit"
	"${TOOLKIT_DIR}/Dependency"
	"${TOOLKIT_DIR}/Dependency/glad"
	"${TOOLKIT_DIR}/Dependency/SDL2/include"
	"${TOOLKIT_DIR}/Dependency/RapidXml"
	"${TOOLKIT_DIR}/Dependency/stb"
	"${TOOLKIT_DIR}/Dependency/zlib-1.2.12/dist/include"
	"${CMAKE_SOURCE_DIR}"
)

set(
	SOURCE 
	Game.cpp
)

if (${WEB_BUILD})
  set(SOURCE ${SOURCE} ../Web/web_main.cpp)
	add_definitions(-DGAME_BUILD=1)
elseif (${WIN_BUILD})
	set(SOURCE ${SOURCE} ../Windows/win_main.cpp)
	add_definitions(-DGAME_BUILD=1)
endif()

set(
	HEADERS 
	Game.h
)

set(
  PCH
 "${TOOLKIT_DIR}/Dependency/glm/glm.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtc/epsilon.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtc/matrix_inverse.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtc/matrix_transform.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtx/matrix_operation.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtx/matrix_query.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtc/quaternion.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtx/closest_point.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtx/vector_query.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtx/string_cast.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtx/component_wise.hpp"
 "${TOOLKIT_DIR}/Dependency/glm/gtx/euler_angles.hpp"
)

set(OUTDIR "${CMAKE_SOURCE_DIR}/Bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTDIR})

if (CLANG_COMPILER)
	SET(TK_CXX_FLAGS " -std=c++17")
endif ()

if (MSVC_COMPILER)
	set(CMAKE_CXX_STANDARD 17)
endif ()

if (NOT TK_CXX_EXTRA STREQUAL "")
	SET(TK_CXX_FLAGS "${TK_CXX_FLAGS} ${TK_CXX_EXTRA}")
endif()

if (${WEB_BUILD})
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin")

  add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})
  target_link_libraries(${PROJECT_NAME} "${TOOLKIT_DIR}/Bin/ToolKit.a")
  target_link_libraries(${PROJECT_NAME} "${TOOLKIT_DIR}/Dependency/zlib-1.2.12/dist/libminizip.a")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TK_CXX_FLAGS} -s USE_SDL=2")

	if (EXISTS "../../MinResources.pak")
	  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -o ${PROJECT_NAME}.html --shell-file ../Bin/shell_minimal.html --preload-file ../../MinResources.pak@/../MinResources.pak --preload-file ../../Config@/../Config -sALLOW_MEMORY_GROWTH=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3 -s USE_ZLIB=1 -sUSE_SDL=2")
	else()
	  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -o ${PROJECT_NAME}.html --shell-file ../Bin/shell_minimal.html --preload-file ../../Resources@/../Resources --preload-file ../../Config@/../Config -sALLOW_MEMORY_GROWTH=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3 -s USE_ZLIB=1 -s USE_SDL=2")
	endif()
else()

  link_directories("${TOOLKIT_DIR}/Bin")
  link_directories("${TOOLKIT_DIR}/Dependency/zlib-1.2.12/dist/lib")
  link_directories("${TOOLKIT_DIR}/Dependency/SDL2/lib/x64")
	
if (${WIN_BUILD})
	add_executable(${PROJECT_NAME} WIN32 ${SOURCE})
else()
	add_library(${PROJECT_NAME} SHARED ${SOURCE} ${HEADERS})
endif()

  target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:TK_DLL_IMPORT>)

  if (MSVC)
		target_link_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:/INCREMENTAL:NO> $<$<NOT:$<CONFIG:DEBUG>>:/LTCG>)
  endif()

  target_link_libraries(${PROJECT_NAME} debug ToolKit_d)
  target_link_libraries(${PROJECT_NAME} optimized ToolKit)
  target_link_libraries(${PROJECT_NAME} zlibstatic minizip)
  target_link_libraries(${PROJECT_NAME} SDL2 SDL2main)
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE ${PCH})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin$<0:>)

#For hotreload rename the previous output
if (NOT ${WEB_BUILD} OR NOT ${WIN_BUILD})
  set(DLL $<SHELL_PATH:$<TARGET_FILE:${PROJECT_NAME}>>)
  set(PDB $<SHELL_PATH:$<TARGET_PDB_FILE:${PROJECT_NAME}>>)

  add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -Ddll=${DLL} -Dpdb=${PDB} -Dtmp="${OUTDIR}/tmp" -P "${CMAKE_SOURCE_DIR}/CMakeHotReload.cmake")
endif()
