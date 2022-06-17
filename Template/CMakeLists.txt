cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME "__projectname__" CACHE STRING "What is the project name ?")
set(TOOLKIT_DIR "__tkdir__" CACHE FILEPATH "Where is the ToolKit source directory ?")

#if not a path is provided, try fetching it from the path file.
if("${TOOLKIT_DIR}" STREQUAL "__tkdir__")
	file (STRINGS "Path.txt" TOOLKIT_DIR)
endif()

#Store original output name
set(DLL_NAME ${PROJECT_NAME})

string(REPLACE " " "-" PROJECT_NAME ${PROJECT_NAME})
project (${PROJECT_NAME})

if(NOT EXISTS ${TOOLKIT_DIR}/ToolKit/ToolKit.h)
  message(SEND_ERROR "Can't find ToolKit.h in ${TOOLKIT_DIR}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${TOOLKIT_DIR}/Bin")
set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(TK_DLL_IMPORT)

link_directories("${TOOLKIT_DIR}/Bin")
link_directories("${TOOLKIT_DIR}/Dependency/OpenAL/libs/Win64")
link_directories("${TOOLKIT_DIR}/Dependency/Glew/lib/Release/x64")
link_directories("${TOOLKIT_DIR}/Dependency/zlib/lib")
link_directories("${TOOLKIT_DIR}/Dependency/Minizip/lib")

include_directories(
	"${TOOLKIT_DIR}/ToolKit"
	"${TOOLKIT_DIR}/Dependency"
	"${TOOLKIT_DIR}/Dependency/Glew/include"
	"${TOOLKIT_DIR}/Dependency/SDL2/include"
	"${TOOLKIT_DIR}/Dependency/OpenAL/include"
	"${TOOLKIT_DIR}/Dependency/RapidXml"
	"${TOOLKIT_DIR}/Dependency/stb"
  "${TOOLKIT_DIR}/Dependency/zlib/include"
  "${TOOLKIT_DIR}/Dependency/Minizip/include"
	"${CMAKE_SOURCE_DIR}"
)

set(SOURCE Game.cpp)
set(HEADERS Game.h)

set(OUTDIR "${CMAKE_SOURCE_DIR}/Bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTDIR})

add_library(${PROJECT_NAME} SHARED  ${SOURCE} ${HEADERS})

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

target_precompile_headers(${PROJECT_NAME} PRIVATE ${PCH})

target_link_libraries(${PROJECT_NAME} debug ToolKit_d)
target_link_libraries(${PROJECT_NAME} optimized ToolKit)
target_link_libraries(${PROJECT_NAME} glew32 OpenGL32 OpenAL32 zlib Minizip)

#Set original output name
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${DLL_NAME})

#For hotreload rename the previous output
set(DLL $<SHELL_PATH:$<TARGET_FILE:${PROJECT_NAME}>>)
set(PDB $<SHELL_PATH:$<TARGET_PDB_FILE:${PROJECT_NAME}>>)
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -Ddll=${DLL} -Dpdb=${PDB} -Dtmp="${OUTDIR}/tmp" -P "${CMAKE_SOURCE_DIR}/CMakeHotReload.cmake")