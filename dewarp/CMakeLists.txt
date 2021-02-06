cmake_minimum_required(VERSION 2.6)

if(DEFINED PLATFORM)
    if(${PLATFORM} STREQUAL ARM64)
        message("==========SET platform to ARM64==========")
        set (CMAKE_SYSTEM_NAME Linux)
        set (CMAKE_SYSTEM_PROCESSOR AArch64)
        set (ARM64 1)
    elseif(${PLATFORM} STREQUAL ARMV7)
        message("==========SET platform to ARMV7==========")
        set (PLATFORM ARMV7)
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

if (GDB)
#SET(CMAKE_BUILD_TYPE "Debug")
if (ANDROID)
SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS}  -Wno-unused-private-field -Wno-inconsistent-missing-override -Wno-format-pedantic")
endif ()
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
SET(CMAKE_C_LINKER_FLAGS "$ENV{CXXFLAGS} -pthread")
endif()

add_definitions(-Werror)
add_definitions(-DPRINT_LOG)

if(CMODEL)
    add_definitions(-DHAL_CMODEL)
    add_definitions(-DLINUX)
endif()

if(V4L2)
    add_definitions(-DUSE_V4L2)
endif()

include_directories(
    ../vvcam/dwe/
    ../vvcam/common/
    ../vvcam/isp
    ../vvcam/video/
    hal
    parser
    cmodel
    ../mediacontrol/buffer
    ../mediacontrol/fpga
    ../mediacontrol/include
    ../utils3rd/3rd/jsoncpp/include/
    ../utils3rd/include/
)

set( platform_libs 
    dewarp_parser_static
    media_buffer_static
    jsoncpp_lib)

if (GENERATE_PARTITION_BUILD)
    add_subdirectory(hal)
    add_subdirectory(parser)
else (GENERATE_PARTITION_BUILD)
    add_subdirectory(proprietories/hal)
    add_subdirectory(proprietories/parser)
endif (GENERATE_PARTITION_BUILD)
add_subdirectory(test)
add_subdirectory(../utils3rd/3rd/jsoncpp jsoncpp.out)
add_subdirectory(../mediacontrol/buffer buffer.out)
add_subdirectory(../mediacontrol/fpga fpga.out)

