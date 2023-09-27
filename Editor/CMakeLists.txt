cmake_minimum_required(VERSION 3.0.0)

include_directories(
	"${CMAKE_SOURCE_DIR}/Editor"
	"${CMAKE_SOURCE_DIR}/ToolKit"
	"${CMAKE_SOURCE_DIR}/Dependency"
	"${CMAKE_SOURCE_DIR}/Dependency/RapidXml"
	"${CMAKE_SOURCE_DIR}/Dependency/stb"
)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/Editor/*.cpp" "${CMAKE_SOURCE_DIR}/Editor/ImGui/*.cpp")

set(CMAKE_EXECUTABLE_SUFFIX ".html")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin")
add_executable(Editor ${SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O0 -g -s USE_SDL=2")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -o Editor.html --shell-file ../Bin/shell_minimal.html --preload-file ../Resources@/../Resources -s ALLOW_MEMORY_GROWTH=1 -s LEGACY_GL_EMULATION=1 -s USE_GLFW=3 -s USE_WEBGL2=1")
target_link_libraries(Editor ToolKit)