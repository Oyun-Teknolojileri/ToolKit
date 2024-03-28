cmake_minimum_required(VERSION 3.10)

project(imgui)

# set output directory for built library
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib$<0:>")

# set source files
set(SOURCES 
    imgui_widgets.cpp 
    imgui_tables.cpp 
    imgui_draw.cpp 
    imgui_demo.cpp 
    imgui.cpp 
    misc/cpp/imgui_stdlib.cpp 
    backends/imgui_impl_opengl3.cpp 
	backends/imgui_impl_sdl2.cpp
)

# add library target
add_library(imgui STATIC ${SOURCES})

# set target properties
set_target_properties(imgui PROPERTIES
    OUTPUT_NAME "imgui"
    DEBUG_POSTFIX "_d"
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

# add root directory to include directories
#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# include subdirectories as additional source files
target_include_directories(imgui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../SDL2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
)
