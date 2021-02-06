cmake_minimum_required(VERSION 2.6)
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_DEBUG "")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

add_definitions(-Wall)

set (module yuv2rgb)

file(GLOB libsources *.cpp)

file(GLOB pub_headers *.h)

add_library(${module}_static STATIC ${libsources})

target_link_libraries ( ${module}_static ${platform_libs} )


