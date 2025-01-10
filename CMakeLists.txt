cmake_minimum_required(VERSION 3.0.0)

project (ToolKit)

# Check if the compiler is Clang
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CLANG_COMPILER TRUE)
else()
    set(CLANG_COMPILER FALSE)
endif()

# Check if the compiler is MSVC
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set(MSVC_COMPILER TRUE)
else ()
    set(MSVC_COMPILER FALSE)
endif ()

if (NOT DEFINED TOOLKIT_DIR)
    set(TOOLKIT_DIR ${CMAKE_SOURCE_DIR})
endif()

# Define TK_GL_ES_3_0 globally
add_definitions(-DTK_GL_ES_3_0)

# Define TK_DLL_EXPORT globally
add_definitions(-DTK_DLL_EXPORT)

add_subdirectory(ToolKit)
#add_subdirectory(Editor)