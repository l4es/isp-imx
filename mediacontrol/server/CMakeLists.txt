cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
add_definitions(-Wall)

if(CMODEL)
    add_definitions(-DHAL_CMODEL)
    add_definitions(-DLINUX)
endif()

if(V4L2)
    add_definitions(-DUSE_V4L2)
    set (V4L2 ON)
endif()

set (module media_server)

file(GLOB libsources *.cpp)
file(GLOB pub_headers *.h )

add_library(${module}_shared SHARED ${libsources} )

add_definitions(-DUSE_SENSOR)

if( NOT DEFINED APPSHELL_TOP_COMPILE)
    if(ARM64)
        set (cam_device_local ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libcam_device.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libadpcc.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libadpf.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libaec.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libaf.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libavs.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libawb.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libbase64.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libcameric_drv.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libcam_engine.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libhal.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libov2775.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libisi.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libtinyxml2.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libcameric_reg_drv.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libmipi_drv.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libcam_calibdb.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libfpga.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libebase.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/liboslayer.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libcommon.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libbufferpool.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/liba2dnr.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/liba3dnr.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libawdr3.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libahdr.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libaee.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libaflt.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libmim_ctrl.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libmom_ctrl.so
            ${MC_TOP_DIRECTORY}/lib/arm-64/fpga/libbufsync_ctrl.so
        )
    else(ARM64)
        set (cam_device_local ${MC_TOP_DIRECTORY}/lib/x86-64/fpga/libcam_device.so)
    endif(ARM64)

endif( NOT DEFINED APPSHELL_TOP_COMPILE)

if (WITH_DWE)
add_definitions(-DWITH_DWE)
if (GENERATE_PARTITION_BUILD)
add_subdirectory(${MC_TOP_DIRECTORY}/../dewarp/hal dewarp.hal.out)
add_subdirectory(${MC_TOP_DIRECTORY}/../dewarp/parser dewarp.parser.out)
else(GENERATE_PARTITION_BUILD)
add_subdirectory(${MC_TOP_DIRECTORY}/../dewarp/proprietories/hal dewarp.hal.out)
add_subdirectory(${MC_TOP_DIRECTORY}/../dewarp/proprietories/parser dewarp.parser.out)
endif()
target_link_libraries ( ${module}_shared dewarp_hal_shared dewarp_parser_static)
endif()



if (WITH_VSE)
add_definitions(-DWITH_VSE)
target_link_libraries ( ${module}_shared  ${LIB_PTHREAD_NAME} vse_hal_shared )
add_subdirectory(${MC_TOP_DIRECTORY}/../vse/hal vse.hal.out)
endif()
#install(TARGETS ${module}_shared DESTINATION bin)

if (WITH_DW200)
add_definitions(-DWITH_DW200)
target_link_libraries ( ${module}_shared dw200_hal_shared dw200_config_parser_static)
add_subdirectory(${MC_TOP_DIRECTORY}/../dw200/hal dw200.hal.out)
add_subdirectory(${MC_TOP_DIRECTORY}/../dw200/parser dw200.parser.out)
endif()

target_link_libraries ( ${module}_shared ${platform_libs} ${cam_device_local} ${LIB_PTHREAD_NAME})

SET_TARGET_PROPERTIES(${module}_shared PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES LINK_FLAGS      -shared)
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")


if( DEFINED APPSHELL_TOP_COMPILE)
# define stuff to install
install(TARGETS ${module}_shared
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${module}
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        LIBRARY         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        )

# create common targets for this module
include(${CMAKE_HOME_DIRECTORY}/targets.cmake)
endif( DEFINED APPSHELL_TOP_COMPILE)
