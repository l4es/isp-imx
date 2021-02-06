cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(V4L2)
    add_definitions(-DUSE_V4L2)
    set (V4L2 ON)
endif()

add_definitions(-Wall)
add_definitions(-fPIC)

set (APP isp_media_server)

file(GLOB pub_headers *.h )

if(DAEMON_NATIVE)
    set( SOURCES_COMMON
    main.cpp
    )
else()
    set( SOURCES_COMMON
        main_v4l2.cpp
        V4l2Event.cpp
    )
endif()

if (GENERATE_PARTITION_BUILD)

else (GENERATE_PARTITION_BUILD)
    link_directories(${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib) 
endif (GENERATE_PARTITION_BUILD)

add_executable(${APP}
${SOURCES_COMMON}
)

if ( ANDROID )
add_library(jsoncpp STATIC IMPORTED)
set_property(TARGET jsoncpp PROPERTY IMPORTED_LOCATION ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib//libjsoncpp.a)
target_link_libraries(isp_media_server jsoncpp )
endif ( ANDROID )

if( NOT DEFINED APPSHELL_TOP_COMPILE )
    target_link_libraries ( ${APP} ${platform_libs} media_server_shared ${LIB_LOG_NAME} )
    add_custom_target(copy_lists ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory  ${MC_TOP_DIRECTORY}/case/ ${MC_TOP_DIRECTORY}/build/daemon/case
    )
if ( ANDROID )
    MESSAGE("system: ANDROID")
    add_definitions(-DANDROID)
    set(LIB_PTHREAD_NAME "")
    set(LIB_RT_NAME "")
else ()
    set(LIB_PTHREAD_NAME "pthread")
    set(LIB_RT_NAME "rt")
endif ( ANDROID )

endif( NOT DEFINED APPSHELL_TOP_COMPILE )

if( DEFINED APPSHELL_TOP_COMPILE )
    # define include paths
    include_directories(
        include
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
        ../../utils3rd/3rd/jsoncpp/include
        )
    set(MC_TOP_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../mediacontrol)
    message("MCTOP DIR: ##:" ${MC_TOP_DIRECTORY})

if (GENERATE_PARTITION_BUILD)
    target_link_libraries ( ${APP} ${platform_libs} media_server_shared ${units_libs} ${LIB_LOG_NAME})
else (GENERATE_PARTITION_BUILD)
    target_link_libraries ( 
        ${APP}
        ${platform_libs}
        media_server_shared
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libcam_device.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libcam_engine.so 
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libcameric_drv.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libhal.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libbase64.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libtinyxml2.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libbufferpool.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libcameric_reg_drv.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libisi.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libmipi_drv.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libcam_calibdb.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libfpga.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libebase.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/liboslayer.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libcommon.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libaec.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libawb.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libaf.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libadpf.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libadpcc.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/liba2dnr.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/liba3dnr.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libawdr3.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libahdr.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libaee.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libaflt.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libavs.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libmim_ctrl.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libmom_ctrl.so
        ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/libbufsync_ctrl.so
        )
endif (GENERATE_PARTITION_BUILD)

endif( DEFINED APPSHELL_TOP_COMPILE )

#install(TARGETS ${APP} DESTINATION bin)
