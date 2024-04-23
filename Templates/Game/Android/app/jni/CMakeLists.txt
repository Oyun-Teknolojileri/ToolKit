cmake_minimum_required(VERSION 3.6)

project(GAME)

# armeabi-v7a requires cpufeatures library
# include(AndroidNdkModules)
# android_ndk_import_module_cpufeatures()

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

# SDL sources are in a subfolder named "SDL"
set(SDL_DIRECTORY_PATH ${TOOLKIT_DIR}/Dependency/SDL2)
add_subdirectory(${SDL_DIRECTORY_PATH} ${SDL_DIRECTORY_PATH}/../build)

# Compilation of companion libraries
#add_subdirectory(SDL_image)
#add_subdirectory(SDL_mixer)
#add_subdirectory(SDL_ttf)

# Your game and its CMakeLists.txt are in a subfolder named "src"
add_subdirectory(src)
