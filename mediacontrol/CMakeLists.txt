cmake_minimum_required(VERSION 3.1.0)

project(ISP_MEDIA_CONTROL)

if( NOT DEFINED APPSHELL_TOP_COMPILE)
    if(DEFINED PLATFORM)
        if(${PLATFORM} STREQUAL ARM64)
            message("==========SET platform to ARM64==========")
            set (CMAKE_SYSTEM_NAME Linux)
            set (CMAKE_SYSTEM_PROCESSOR AArch64)
            set (ARM64 1)
        elseif(${PLATFORM} STREQUAL ARMV7)
            message("==========SET platform to ARMV7==========")
            set (PLATFORM ARMV7)
            set (ARMV7 1)
        elseif(${PLATFORM} STREQUAL X86)
            message("==========SET platform to X86==========")
            set (PLATFORM X86)
        else(${PLATFORM} STREQUAL ARM64)
            message(FATAL_ERROR "CFG_ERROR PLATFORM=${PLATFORM} error, can not found configuration settings, exit")
        endif (${PLATFORM} STREQUAL ARM64)
    else(DEFINED PLATFORM)
        message("==========SET DEFAILT platform to X86==========")
        set (PLATFORM X86)
    endif(DEFINED PLATFORM)

    set(MC_TOP_DIRECTORY ${PROJECT_SOURCE_DIR})
endif( NOT DEFINED APPSHELL_TOP_COMPILE)

if( DEFINED APPSHELL_TOP_COMPILE)
    set(MC_TOP_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../mediacontrol)
endif( DEFINED APPSHELL_TOP_COMPILE)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -m64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -whole-archive")

if(DEFINED PARTITION_BUILD)
    if(${PARTITION_BUILD} STREQUAL 0)
        set (GENERATE_PARTITION_BUILD 0)
    else(${PARTITION_BUILD} STREQUAL 0)
        set (GENERATE_PARTITION_BUILD 1)
    endif(${PARTITION_BUILD} STREQUAL 0)
else(DEFINED PARTITION_BUILD)
    #default value of GENERATE_PARTITION_BUILD
    set (GENERATE_PARTITION_BUILD 1)
endif(DEFINED PARTITION_BUILD)

if ( ANDROID )
    MESSAGE("system: ANDROID")
    add_definitions(-DANDROID)
    set(LIB_PTHREAD_NAME "")
    set(LIB_RT_NAME "")
else ()
    set(LIB_PTHREAD_NAME "pthread")
    set(LIB_RT_NAME "rt")
endif ( ANDROID )

if (ANDROID)
SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS}  -Wno-unused-private-field -Wno-inconsistent-missing-override -Wno-format-pedantic")
endif ()
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
SET(CMAKE_C_LINKER_FLAGS "$ENV{CXXFLAGS} -${LIB_PTHREAD_NAME}")

include_directories(
    include
    include_api
    buffer
    fpga
    server
    ../utils3rd/3rd/jsoncpp/include
    ../utils3rd/include
    ../units/cam_device/include
    ${MC_TOP_DIRECTORY}/../vvcam/common
    ${MC_TOP_DIRECTORY}/../vvcam/video
    ${MC_TOP_DIRECTORY}/../vvcam/dwe
    ${MC_TOP_DIRECTORY}/../dewarp/hal
    ${MC_TOP_DIRECTORY}/../dewarp/cmodel
    ${MC_TOP_DIRECTORY}/../dewarp/parser
    ${MC_TOP_DIRECTORY}/../dewarp/include/
    ${MC_TOP_DIRECTORY}/../vse/cmodel
    ${MC_TOP_DIRECTORY}/../vse/hal
    ${MC_TOP_DIRECTORY}/../vvcam/vse
    ${MC_TOP_DIRECTORY}/../vvcam/dw200
    ${MC_TOP_DIRECTORY}/../vse/include
    ${MC_TOP_DIRECTORY}/../dw200/hal
    ${MC_TOP_DIRECTORY}/../dw200/parser
)

if ( ANDROID )
set( platform_libs
    media_buffer_static
)
else ()
set( platform_libs  
    jsoncpp_lib
    media_buffer_static
)
endif( ANDROID )

if(ENABLE_IRQ)
    add_definitions(-DENABLE_IRQ)
endif(ENABLE_IRQ)

add_definitions(-Werror)
add_definitions(-O2)

add_subdirectory(buffer)
add_subdirectory(server)
add_subdirectory(fpga)

if( NOT DEFINED APPSHELL_TOP_COMPILE)
    add_subdirectory(${MC_TOP_DIRECTORY}/../utils3rd/3rd/jsoncpp jsoncpp)
endif( NOT DEFINED APPSHELL_TOP_COMPILE)
add_subdirectory(daemon)


