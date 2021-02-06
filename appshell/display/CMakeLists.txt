cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

add_definitions(-Wall)

set (module vvdisplay)

if ( ANDROID )
include_directories(${ANDROID_ROOT}/vendor/nxp-opensource/libdrm-imx)
link_directories(${ANDROID_ROOT}/out/target/product/evk_8mp/vendor/lib64)
link_directories(${ANDROID_ROOT}/out/target/product/evk_8mp/system/lib64)
endif ()

if (WITH_DRM)
include(drm.cmake)
include_directories(
    ../../mediacontrol/buffer
    ../../mediacontrol/include
    ${DRM_HEADER_PATH}
)
add_definitions(-DWITH_DRM)
else(WITH_DRM)
include_directories(
    ../../mediacontrol/buffer
    ../../mediacontrol/include
)
endif()


file(GLOB libsources *.cpp)

file(GLOB pub_headers *.h)

add_library(${module}_shared SHARED ${libsources})

if( NOT DEFINED APPSHELL_TOP_COMPILE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../yuv yuv)
endif( NOT DEFINED APPSHELL_TOP_COMPILE)

if (WITH_DRM)
if (NOT ANDROID)
target_link_libraries ( ${module}_shared yuv2rgb_static drm ${LIB_PTHREAD_NAME})
else ()
target_link_libraries ( ${module}_shared yuv2rgb_static ${LIB_PTHREAD_NAME} drm_android cutils base c++)
endif ()
else(WITH_DRM)
target_link_libraries ( ${module}_shared yuv2rgb_static ${LIB_PTHREAD_NAME})
endif()

