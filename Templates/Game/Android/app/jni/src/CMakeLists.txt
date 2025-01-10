# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.6)

project(Game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-parentheses -Wno-switch")

set(GAME_CODE_DIR "${CMAKE_SOURCE_DIR}/../../../Codes")

# Add the game cmake file.
add_subdirectory(${GAME_CODE_DIR} ${CMAKE_BINARY_DIR}/Codes)
