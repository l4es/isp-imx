cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

if(V4L2)
    add_definitions(-DUSE_V4L2)
endif()
if(SUBDEV_V4L2)
    add_definitions(-DUSE_V4L2)
endif()

if(CMODEL)
    add_definitions(-DHAL_CMODEL)
    add_definitions(-DLINUX)
    set (dewarp_cmodel_lib ${PROJECT_SOURCE_DIR}/../dewarp/cmodel/libdewarp_cmodel.so)
endif()

add_definitions(-Wall)

set (module dewarp_hal)

file(GLOB libsources *.cpp ../../vvcam/dwe/*.c)

file(GLOB pub_headers *.h)

add_library(${module}_shared SHARED ${libsources})

message(${PROJECT_SOURCE_DIR})
target_link_libraries ( ${module}_shared  ${LIB_PTHREAD_NAME} )
target_link_libraries ( ${module}_shared ${dewarp_cmodel_lib} ) 
target_link_libraries ( ${module}_shared  viv_fpga_driver_static)


SET_TARGET_PROPERTIES(${module}_shared PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES LINK_FLAGS      "-shared -fPIC")
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

if(GENERATE_PARTITION_BUILD)
    if(DEFINED APPSHELL_TOP_COMPILE)
        add_custom_target(copy_libs_${module} ALL
            COMMENT "##Copy share lib to propritories"
            COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/../../generated/${CMAKE_BUILD_TYPE}/lib/lib${module}.so ${CMAKE_CURRENT_SOURCE_DIR}/../proprietories/hal/lib/lib${module}.so
        )
        add_dependencies(copy_libs_${module} ${module}_shared)
    endif(DEFINED APPSHELL_TOP_COMPILE)
endif(GENERATE_PARTITION_BUILD)
