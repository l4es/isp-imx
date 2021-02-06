cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

add_definitions(-Wall)

set (module viv_fpga_driver)

file(GLOB libsources *.cpp)

file(GLOB pub_headers *.h)

add_library(${module}_static STATIC ${libsources})


SET_TARGET_PROPERTIES(${module}_static PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_static PROPERTIES LINK_FLAGS      "-static -fPIC")
SET_TARGET_PROPERTIES(${module}_static PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

if( DEFINED APPSHELL_TOP_COMPILE)
# define stuff to install
install(TARGETS ${module}_static
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${module}
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        )

# create common targets for this module
include(${CMAKE_HOME_DIRECTORY}/targets.cmake)
endif( DEFINED APPSHELL_TOP_COMPILE)