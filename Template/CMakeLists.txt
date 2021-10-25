cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME "__projectname__" CACHE STRING "What is the project name ?")
set(TOOLKIT_DIR "__tkdir__" CACHE FILEPATH "Where is the ToolKit source directory ?")

project (${PROJECT_NAME})

if(NOT EXISTS ${TOOLKIT_DIR}/Source/ToolKit.h)
  message(SEND_ERROR "Can't find ToolKit.h in ${TOOLKIT_DIR}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${TOOLKIT_DIR}/Bin")
set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(TK_EXPORTS)

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

set(SOURCE App.cpp)
set(HEADERS App.h)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Bin")
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Bin")

add_library(${PROJECT_NAME} SHARED  ${SOURCE} ${HEADERS})

target_link_libraries(${PROJECT_NAME} debug ToolKit_d)
target_link_libraries(${PROJECT_NAME} optimized ToolKit)
target_link_libraries(${PROJECT_NAME} glew32 OpenGL32 OpenAL32)