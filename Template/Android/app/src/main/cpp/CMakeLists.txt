# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.0.0)

project("myapplication")

set(TOOLKIT_DIR "__tkdir__" CACHE FILEPATH "Where is the ToolKit source directory ?")

#if not a path is provided, try fetching it from the path file.
if("${TOOLKIT_DIR}" STREQUAL "__tkdir__")
    if(DEFINED ENV{APPDATA})
        cmake_path(SET CFG_FILE_PATH "$ENV{APPDATA}/ToolKit/Config/Path.txt")
        if(NOT EXISTS ${CFG_FILE_PATH})
            message(SEND_ERROR "Set TOOLKIT_DIR or provide ${CFG_FILE_PATH} containing path to ToolKit.")
        else()
            file(STRINGS ${CFG_FILE_PATH} TOOLKIT_DIR)
        endif()
    endif()

    if(NOT EXISTS ${TOOLKIT_DIR}/ToolKit/ToolKit.h)
        message(SEND_ERROR "Can't find ToolKit.h in ${TOOLKIT_DIR}")
    endif()
endif()

# copy engine resources to Android/assets folder
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/../assets/Resources")
    message("copying engine resources to android assets folder")

    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/../assets/Resources")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/../assets/Resources/Engine")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/../assets/Resources/Engine")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/../assets/Resources/Engine/Fonts")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/../assets/Resources/Engine/Materials")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/../assets/Resources/Engine/Meshes")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/../assets/Resources/Engine/Shaders")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/../assets/Resources/Engine/Textures")

    file(GLOB TOOLKIT_TEXTURES
            "${TOOLKIT_DIR}/Resources/Engine/Textures/*.png"
            "${TOOLKIT_DIR}/Resources/Engine/Textures/*.jpg"
            "${TOOLKIT_DIR}/Resources/Engine/Textures/*.hdr")

    file(COPY ${TOOLKIT_TEXTURES} DESTINATION "${CMAKE_SOURCE_DIR}/../assets/Resources/Engine/Textures")

    file(GLOB TOOLKIT_SHADERS "${TOOLKIT_DIR}/Resources/Engine/Shaders/*.png")

    file(COPY ${TOOLKIT_SHADERS} DESTINATION "${CMAKE_SOURCE_DIR}/../assets/Resources/Engine/Shaders")

    file(GLOB TOOLKIT_MESHES "${TOOLKIT_DIR}/Resources/Engine/Meshes/*.mesh")
    file(COPY ${TOOLKIT_MESHES} DESTINATION "${CMAKE_SOURCE_DIR}/../assets/Resources/Engine/Meshes")
endif()

# Use file(GLOB ...) to collect all .cpp files in the ToolKit directory.
file(GLOB TOOLKIT_SOURCES ${TOOLKIT_DIR}/ToolKit/*.cpp)
file(GLOB UNZIP_SOURCES unzip/*.c)

include_directories(${TOOLKIT_DIR}/ToolKit
                    ${TOOLKIT_DIR}/Dependency
                    ${CMAKE_SOURCE_DIR}/unzip)

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_library(myapplication SHARED
        main.cpp
        ToolKitMain.cpp
        Game.cpp
        ${UNZIP_SOURCES}
        ${TOOLKIT_SOURCES}) # Include the list of Toolkit sources here

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

# if(ANDROID_ABI STREQUAL armeabi-v7a)
#   set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS -mfpu=neon)
# endif()
find_package(ZLIB REQUIRED)

# Configure libraries CMake uses to link your target library.
target_link_libraries(myapplication
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