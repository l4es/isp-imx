cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")

if(V4L2)
    add_definitions(-DUSE_V4L2)
    set (V4L2 ON)
endif()
add_definitions(-Wall)
add_definitions(-fPIC)

if(CMODEL)
    add_definitions(-DHAL_CMODEL)
    add_definitions(-DLINUX)
endif()

set (APP dewarp_test)

file(GLOB pub_headers *.h )

add_executable(${APP}
    test_main.cpp
    highwayhash.c
)

add_executable(case_generator
    case_generator.cpp
)

target_link_libraries ( ${APP} ${platform_libs} viv_fpga_driver_static dewarp_hal_shared ${LIB_PTHREAD_NAME} )
target_link_libraries ( case_generator jsoncpp_lib )

add_custom_target(copy_lists ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory  ${PROJECT_SOURCE_DIR}/test/case/ ${PROJECT_SOURCE_DIR}/build/test/case
)

#install(TARGETS ${APP} DESTINATION bin)
