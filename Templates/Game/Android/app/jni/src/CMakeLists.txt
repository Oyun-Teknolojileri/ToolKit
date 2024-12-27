# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.6)

project("ToolKit")

# Use file(GLOB ...) to collect all .cpp files in the ToolKit directory.
file(GLOB GAME_MAIN_SOURCES ${CMAKE_SOURCE_DIR}/../../../Codes/*.cpp)

set(GAME_SOURCES ${GAME_MAIN_SOURCES})

include_directories(
        ${TOOLKIT_DIR}/ToolKit
        ${TOOLKIT_DIR}/Dependency
        ${TOOLKIT_DIR}/Dependency/RapidXml
        ${TOOLKIT_DIR}/Dependency/glm
        ${TOOLKIT_DIR}/Dependency/minizip-ng
        ${TOOLKIT_DIR}/Dependency/minizip-ng/build
        ${CMAKE_SOURCE_DIR}/SDL2/include
        ${CMAKE_SOURCE_DIR}/../../../Codes # user headers for game
        ${CMAKE_SOURCE_DIR}/src)

find_library(SDL2 SDL2)
find_library(SDL2main SDL2main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-parentheses -Wno-switch")

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_library(main SHARED ${GAME_SOURCES}) # Include the list of Toolkit sources here

# if(ANDROID_ABI STREQUAL armeabi-v7a)
#   set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS -mfpu=neon)
# endif()

# Configure libraries CMake uses to link your target library.
target_link_libraries(main
        ToolKitStatic
        jnigraphics
        android
        log
        SDL2main
        SDL2)