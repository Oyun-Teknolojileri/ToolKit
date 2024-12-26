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
add_subdirectory(${SDL_DIRECTORY_PATH} ${SDL_DIRECTORY_PATH}/TkAndriodBuild)

# Set the options for minizip
set(MZ_COMPAT ON)
set(ZSTD_BUILD_STATIC ON)
set(ZSTD_BUILD_SHARED OFF)
set(MZ_ZLIB OFF)
set(MZ_BZIP2 OFF)
set(MZ_LZMA OFF)
set(MZ_FETCH_LIBS ON)
set(MZ_FORCE_FETCH_LIBS ON)
set(MZ_PKCRYPT OFF)
set(MZ_WZAES OFF)
set(MZ_ICONV OFF)
set(MINIZIP_DIRECTORY_PATH ${TOOLKIT_DIR}/Dependency/minizip-ng)
add_subdirectory(${MINIZIP_DIRECTORY_PATH} ${MINIZIP_DIRECTORY_PATH}/TkAndriodBuild)

# Your game and its CMakeLists.txt are in a subfolder named "src"
add_subdirectory(src)
