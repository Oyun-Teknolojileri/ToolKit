cmake_minimum_required(VERSION 3.0.0)

include_directories(
	"${CMAKE_SOURCE_DIR}/Samples/SpaceShooter"
	"${CMAKE_SOURCE_DIR}/Source"
	"${CMAKE_SOURCE_DIR}/Dependency"
	"${CMAKE_SOURCE_DIR}/Dependency/OpenAL/include"
	"${CMAKE_SOURCE_DIR}/Dependency/RapidXml"
	"${CMAKE_SOURCE_DIR}/Dependency/SDL2/include"
	"${CMAKE_SOURCE_DIR}/Dependency/stb"
)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/Samples/SpaceShooter/*.cpp")

set(CMAKE_EXECUTABLE_SUFFIX ".html")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin")
add_executable(SpaceShooter ${SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O0 -g")
set_target_properties(SpaceShooter PROPERTIES LINK_FLAGS "-o SpaceShooter.html --preload-file ../Resources -s ALLOW_MEMORY_GROWTH=1 -s USE_WEBGL2=1 -s USE_GLFW=3 -s USE_SDL=2 -s USE_SDL_TTF=2")
target_link_libraries(SpaceShooter ToolKit)