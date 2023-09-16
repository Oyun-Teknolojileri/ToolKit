# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.0.0)

project("toolkit")

set(TOOLKIT_DIR "__tkdir__" CACHE FILEPATH "Where is the ToolKit source directory ?")

#if not a path is provided, try fetching it from the path file.
if("${TOOLKIT_DIR}" STREQUAL "__tkdir__")
    if(DEFINED ENV{APPDATA})
        set(CFG_FILE_PATH "$ENV{APPDATA}/ToolKit/Config/Path.txt")

        if(NOT EXISTS ${CFG_FILE_PATH})
            message(SEND_ERROR "Set TOOLKIT_DIR or provide ${CFG_FILE_PATH} containing path to ToolKit.")
        else()
            file(STRINGS ${CFG_FILE_PATH} TOOLKIT_DIR)
        endif()
    endif()

    if(NOT EXISTS ${TOOLKIT_DIR}/ToolKit/ToolKit.h)
        message(SEND_ERROR "Please use Forward slashes instead of backslashes. Can't find ToolKit.h in ${TOOLKIT_DIR}")
    endif()
endif()

# Use file(GLOB ...) to collect all .cpp files in the ToolKit directory.
file(GLOB TOOLKIT_SOURCES ${TOOLKIT_DIR}/ToolKit/*.cpp)
file(GLOB GAME_SOURCES ${CMAKE_SOURCE_DIR}/../../../../../Codes/*.cpp)
file(GLOB UNZIP_SOURCES ${TOOLKIT_DIR}/Dependency/Android/minizip/*.c)

# Define TK_GL_ES_3_0 globally
add_definitions(-DTK_GL_ES_3_0)
# Define TK_DLL_EXPORT globally
add_definitions(-DTK_DLL_EXPORT)

include_directories(${TOOLKIT_DIR}/ToolKit
                    ${TOOLKIT_DIR}/Dependency
                    ${CMAKE_SOURCE_DIR}/../../../../../Codes) # user headers for game

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_library(toolkit SHARED
        main.cpp
        AndroidMain.cpp
        ${GAME_SOURCES}
        ${UNZIP_SOURCES}
        ${TOOLKIT_SOURCES}) # Include the list of Toolkit sources here

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

# if(ANDROID_ABI STREQUAL armeabi-v7a)
#   set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS -mfpu=neon)
# endif()
find_package(ZLIB REQUIRED)

# Configure libraries CMake uses to link your target library.
target_link_libraries(toolkit
        # The game activity
        game-activity::game-activity

        # EGL and other dependent libraries required for drawing
        # and interacting with Android system
        EGL
        GLESv3
        jnigraphics
        android
        log
        ZLIB::ZLIB)