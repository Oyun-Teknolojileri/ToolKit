cmake_minimum_required(VERSION 3.0.0)

project (ToolKit)

# Check if the compiler is Clang
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CLANG_COMPILER TRUE)
else ()
    set(CLANG_COMPILER FALSE)
endif ()

# Check if the compiler is MSVC
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set(MSVC_COMPILER TRUE)
else ()
    set(MSVC_COMPILER FALSE)
endif ()

add_subdirectory(ToolKit)
#add_subdirectory(Editor)