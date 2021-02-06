cmake_minimum_required(VERSION 2.6)

project(ISP_V4L_DRM_TEST)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(-Wall)
add_definitions(-fPIC)
add_definitions(-DPRINT_LOG)
add_definitions(-DISP_VIDEO_TEST)

include_directories(
    ../../vvcam/common
    ../../units/cam_device/include
    ../../utils3rd/3rd/jsoncpp/include
    ../../utils3rd/include
    ../../mediacontrol/include_api
)
file(GLOB pub_headers *.h )

add_executable(vvext
    vvext.cpp
)

if( NOT DEFINED APPSHELL_TOP_COMPILE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../../utils3rd/3rd/jsoncpp jsoncpp)
endif( NOT DEFINED APPSHELL_TOP_COMPILE)

target_link_libraries ( vvext ${LIB_PTHREAD_NAME}  jsoncpp_lib ${LIB_LOG_NAME})
