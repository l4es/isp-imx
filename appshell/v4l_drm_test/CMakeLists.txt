cmake_minimum_required(VERSION 2.6)

project(ISP_V4L_DRM_TEST)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set (CMAKE_CXX_FLAGS_DEBUG "-fno-strict-aliasing -O3")
set (CMAKE_C_FLAGS_DEBUG "-fno-strict-aliasing -O3")

add_definitions(-Wall)
add_definitions(-fPIC)
add_definitions(-DPRINT_LOG)
add_definitions(-DISP_VIDEO_TEST)


if (WITH_DRM)
include(drm.cmake)
include_directories(
    ../display/
    ../../vvcam/isp
    ../../vvcam/common
    ../../units/cam_device/include
    ../../utils3rd/3rd/jsoncpp/include
    ../../utils3rd/include
    ../../mediacontrol/include
    ../../mediacontrol/buffer
    ${DRM_HEADER_PATH}
)
add_definitions(-DWITH_DRM)
else(WIDTH_DRM)
include_directories(
    ../display/
    ../../vvcam/isp
    ../../vvcam/common
    ../../units/cam_device/include
    ../../utils3rd/3rd/jsoncpp/include
    ../../utils3rd/include
    ../../mediacontrol/include
    ../../mediacontrol/buffer
)
endif()

file(GLOB pub_headers *.h )


if ( ANDROID )
include_directories(${ANDROID_ROOT}/vendor/nxp-opensource/libdrm-imx)
link_directories(${ANDROID_ROOT}/out/target/product/evk_8mp/vendor/lib64)
link_directories(${ANDROID_ROOT}/out/target/product/evk_8mp/system/lib64)
endif ()

if (WITH_DRM)
add_executable(drm_test
    drm_test.cpp
)
endif()

add_executable(isp_tpg_test
    isp_tpg_test.cpp
)


add_executable(video_test
    video_test.cpp
)



add_definitions(-DAPPMODE_V4L2)
add_definitions(-DSUPPORT_PASS_JSON)

if( NOT DEFINED APPSHELL_TOP_COMPILE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../../utils3rd/3rd/jsoncpp jsoncpp)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../display vvdisplay)
endif( NOT DEFINED APPSHELL_TOP_COMPILE)

if (WITH_DRM)
if ( NOT ANDROID )
    target_link_libraries ( drm_test ${LIB_PTHREAD_NAME} drm ${LIB_LOG_NAME})
    target_link_libraries ( video_test ${LIB_PTHREAD_NAME} drm yuv2rgb_static vvdisplay_shared ${LIB_LOG_NAME})
else ()
    target_link_libraries ( drm_test ${LIB_PTHREAD_NAME} ${LIB_LOG_NAME} drm_android cutils base c++ )
    target_link_libraries ( video_test ${LIB_PTHREAD_NAME} yuv2rgb_static vvdisplay_shared ${LIB_LOG_NAME} drm_android cutils base c++ )
endif ()
else (WITH_DRM)
    target_link_libraries ( video_test ${LIB_PTHREAD_NAME} yuv2rgb_static vvdisplay_shared ${LIB_LOG_NAME})
endif()

if ( NOT ANDROID )
target_link_libraries ( isp_tpg_test ${LIB_LOG_NAME})
else ()
target_link_libraries ( isp_tpg_test ${LIB_LOG_NAME} c++)
endif ()
