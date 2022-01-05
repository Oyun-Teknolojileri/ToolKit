cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME "ThiefGo" CACHE STRING "What is the project name ?")
set(TOOLKIT_DIR "C:/Users/afrai/source/repos/ToolKit" CACHE FILEPATH "Where is the ToolKit source directory ?")

project (${PROJECT_NAME})

if(NOT EXISTS ${TOOLKIT_DIR}/Source/ToolKit.h)
  message(SEND_ERROR "Can't find ToolKit.h in ${TOOLKIT_DIR}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${TOOLKIT_DIR}/Bin")
set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(TK_DLL_IMPORT)

link_directories("${TOOLKIT_DIR}/Bin")
link_directories("${TOOLKIT_DIR}/Dependency/OpenAL/libs/Win64")
link_directories("${TOOLKIT_DIR}/Dependency/Glew/lib/Release/x64")

include_directories(
	"${TOOLKIT_DIR}/Source"
	"${TOOLKIT_DIR}/Dependency"
	"${TOOLKIT_DIR}/Dependency/Glew/include"
	"${TOOLKIT_DIR}/Dependency/SDL2/include"
	"${TOOLKIT_DIR}/Dependency/OpenAL/include"
	"${TOOLKIT_DIR}/Dependency/RapidXml"
	"${TOOLKIT_DIR}/Dependency/stb"
	"${CMAKE_SOURCE_DIR}"
)

set(SOURCE App.cpp PlayerController.cpp EnemyController.cpp GameUtil.cpp MainUI.h)
set(HEADERS App.h PlayerController.h EnemyController.h GameUtil.h MainUI.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Bin")

add_library(${PROJECT_NAME} SHARED  ${SOURCE} ${HEADERS})

set (
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
target_link_libraries(${PROJECT_NAME} glew32 OpenGL32 OpenAL32)