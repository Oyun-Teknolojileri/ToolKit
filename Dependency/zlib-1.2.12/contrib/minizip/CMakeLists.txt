cmake_minimum_required(VERSION 2.4.4)
project(minizip C)
set(VERSION "1.2.12")

include_directories("${CMAKE_SOURCE_DIR}/../../dist/include")
link_directories("${CMAKE_SOURCE_DIR}/../../dist/lib")

set(SOURCE
  ioapi.c
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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_ZLIB=1")

add_library(minizip STATIC ${HEADERS} ${SOURCE})
target_link_libraries (minizip LINK_PUBLIC zlibstatic.lib)