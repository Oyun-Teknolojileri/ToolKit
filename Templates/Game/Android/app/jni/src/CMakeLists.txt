# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.6)

project("toolkit")

# Use file(GLOB ...) to collect all .cpp files in the ToolKit directory.
file(GLOB TOOLKIT_SOURCES ${TOOLKIT_DIR}/ToolKit/*.cpp)
file(GLOB GAME_SOURCES ${CMAKE_SOURCE_DIR}/../../../Codes/*.cpp)
file(GLOB UNZIP_SOURCES ${TOOLKIT_DIR}/Dependency/Android/minizip/*.c)

# Define TK_GL_ES_3_0 globally
add_definitions(-DTK_GL_ES_3_0)
# Define TK_DLL_EXPORT globally
add_definitions(-DTK_DLL_EXPORT)

include_directories(${TOOLKIT_DIR}/ToolKit
        ${TOOLKIT_DIR}/Dependency
        ${TOOLKIT_DIR}/Dependency/glm
        ${CMAKE_SOURCE_DIR}/SDL2/include
        ${CMAKE_SOURCE_DIR}/../../../Codes # user headers for game
        ${CMAKE_SOURCE_DIR}/src)

find_library(SDL2 SDL2)
find_library(SDL2main SDL2main)

set(CMAKE_CXX_STANDARD 17)

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_library(main SHARED
        ${GAME_SOURCES}
        ${UNZIP_SOURCES}
        ${TOOLKIT_SOURCES}) # Include the list of Toolkit sources here


# if(ANDROID_ABI STREQUAL armeabi-v7a)
#   set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS -mfpu=neon)
# endif()
find_package(ZLIB REQUIRED)

# Configure libraries CMake uses to link your target library.
target_link_libraries(main
        GLESv3
        jnigraphics
        android
        log
        SDL2main
        SDL2
        ZLIB::ZLIB)