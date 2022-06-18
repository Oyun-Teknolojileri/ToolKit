cmake_minimum_required(VERSION 2.4.4)
project(minizip C)
set(VERSION "1.2.12")

if(CMAKE_HOST_APPLE)
  set(PLATFORM __APPLE__)
elseif(CMAKE_HOST_UNIX)
  set(PLATFORM unix)
elseif(CMAKE_HOST_WIN32)
  set(PLATFORM _WIN32)
else(CMAKE_HOST_APPLE)
  message(FATAL_ERROR "Not supported Platform")
endif(CMAKE_HOST_APPLE)

add_definitions(-D${PLATFORM})

include_directories("${CMAKE_SOURCE_DIR}/../../dist/include")
link_directories("${CMAKE_SOURCE_DIR}/../../dist/lib")

set(SOURCE
  ioapi.c
  miniunz.c
  minizip.c
  mztools.c
  unzip.c
  zip.c
)

if(WIN32)
    set(SOURCE ${SOURCE} iowin32.c)
endif(WIN32)

set(HEADERS
  crypt.h
  ioapi.h 
  mztools.h
  unzip.h
  zip.h
)

if(WIN32)
    set(HEADERS ${HEADERS} iowin32.h)
endif(WIN32)

message(STATUS ${HEADERS})
message(STATUS ${SOURCE})

add_library(minizip STATIC ${HEADERS} ${SOURCE})
target_link_libraries (minizip LINK_PUBLIC zlibstatic.lib)
