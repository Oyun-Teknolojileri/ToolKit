cmake_minimum_required(VERSION 3.10)

project(imgui)

# Define IM_EXPORT
add_definitions(-DIM_EXPORT)

# Add a preprocessor definition for IMGUI_USER_CONFIG
add_definitions(-DIMGUI_USER_CONFIG="tk_imconfig.h")

# set output directory for built library
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib$<0:>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib$<0:>")

# set source files
set(SOURCES 
    imgui/imgui_widgets.cpp 
    imgui/imgui_tables.cpp 
    imgui/imgui_draw.cpp 
    imgui/imgui_demo.cpp 
    imgui/imgui.cpp 
    imgui/misc/cpp/imgui_stdlib.cpp 
    imgui/backends/imgui_impl_opengl3.cpp 
	imgui/backends/imgui_impl_sdl2.cpp
)

# add library target
add_library(imgui SHARED ${SOURCES})

# set target properties
set_target_properties(imgui PROPERTIES
    OUTPUT_NAME "imgui"
    DEBUG_POSTFIX "_d"
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

#link sdl
set(SDL2_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2/lib)
target_link_libraries(imgui PRIVATE ${SDL2_LIB_DIR}/SDL2.lib)
target_link_libraries(imgui PRIVATE ${SDL2_LIB_DIR}/SDL2main.lib)

# include subdirectories as additional source files
target_include_directories(imgui PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../SDL2/include
	${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)
