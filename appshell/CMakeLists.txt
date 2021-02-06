#/******************************************************************************\
#|* Copyright (c) 2011-2015 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")  *|
#|* All Rights Reserved.                                                       *|
#|*                                                                            *|
#|* The material in this file is confidential and contains trade secrets       *|
#|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
#|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
#|* transmitted, or used in any way for any purpose, without the express       *|
#|* written permission of VeriSilicon.                                         *|
#|*                                                                            *|
#\******************************************************************************/


cmake_minimum_required(VERSION 3.1.0)
project(ISP_APP_SHELL)

if(DEFINED QTLESS)
    if(${QTLESS} STREQUAL 0)
        set (QTLESS 0)
    else(${QTLESS} STREQUAL 0)
        set (QTLESS 1)
    endif(${QTLESS} STREQUAL 0)
else(DEFINED QTLESS)
    #default value of QTLESS
set (QTLESS 0)
endif(DEFINED QTLESS)

if(DEFINED SERVER_LESS)
    if(${SERVER_LESS} STREQUAL 0)
        set (SERVER_LESS 0)
    else(${SERVER_LESS} STREQUAL 0)
        set (SERVER_LESS 1)
    endif(${SERVER_LESS} STREQUAL 0)
else(DEFINED SERVER_LESS)
    #default value of SERVER_LESS
set (SERVER_LESS 0)
endif(DEFINED SERVER_LESS)

if(DEFINED FULL_SRC_COMPILE)
    if(${FULL_SRC_COMPILE} STREQUAL 0)
        set (FULL_SRC_COMPILE 0)
    else(${FULL_SRC_COMPILE} STREQUAL 0)
    set (FULL_SRC_COMPILE 1)
    endif(${FULL_SRC_COMPILE} STREQUAL 0)
else(DEFINED FULL_SRC_COMPILE)
    #default value of FULL_SRC_COMPILE
    set (FULL_SRC_COMPILE 0)
endif(DEFINED FULL_SRC_COMPILE)

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

if(DEFINED 3A_SRC_BUILD)
    if(${3A_SRC_BUILD} STREQUAL 0)
        set (GENERATE_3A_LIB 0)
    else(${3A_SRC_BUILD} STREQUAL 0)
    set (GENERATE_3A_LIB 1)
    endif(${3A_SRC_BUILD} STREQUAL 0)
else(DEFINED 3A_SRC_BUILD)
    #default value of GENERATE_PARTITION_BUILD
    set (GENERATE_3A_LIB 1)
endif(DEFINED 3A_SRC_BUILD)

if(DEFINED WITH_DRM)
    if(${WITH_DRM} STREQUAL 0)
        set (WITH_DRM 0)
    else(${WITH_DRM} STREQUAL 0)
        set (WITH_DRM 1)
    endif(${WITH_DR} STREQUAL 0)
else(DEFINED WITH_DRM)
    #default value of WITH_DRM
    set (WITH_DRM 0)
endif(DEFINED WITH_DRM)

##default path of drm header path
if (DEFINED WITH_DRM)
    set (DRM_HEADER_PATH $ENV{SDKTARGETSYSROOT}/usr/include/drm/)
endif(DEFINED WITH_DRM)

set (APPSHELL_TOP_COMPILE 1)
set (SENSOR ON)

set (PLATFORM $ENV{COMPILE_PLATFORM})

if(DEFINED PLATFORM)
    if(${PLATFORM} STREQUAL ARM64)
        message("==========SET platform to ARM64==========")
        set (CMAKE_SYSTEM_NAME Linux)
        set (CMAKE_SYSTEM_PROCESSOR AArch64)

        set(CMAKE_PREFIX_PATH $ENV{Qt_DIR5_ARM64})
        set(QWT6_PATH $ENV{QWT_DIR_ARM64})
        message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
        message("QWT6_PATH: ${QWT6_PATH}")
        set (ARM64 1)
    elseif(${PLATFORM} STREQUAL ARMV7)
        message("==========SET platform to ARMV7==========")
        set (PLATFORM ARMV7)
        set(CMAKE_PREFIX_PATH $ENV{Qt_DIR5_ARMV7})
        set(QWT6_PATH $ENV{QWT_DIR_ARMV7})
        message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
        message("QWT6_PATH: ${QWT6_PATH}")
        set (ARMV7 1)
    elseif(${PLATFORM} STREQUAL X86)
        message("==========SET platform to X86==========")
        set (PLATFORM X86)

        set(CMAKE_PREFIX_PATH $ENV{Qt_DIR5_X86})
        set(QWT6_PATH $ENV{QWT_DIR_X86})
        message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
        message("QWT6_PATH: ${QWT6_PATH}")
    elseif(${PLATFORM} STREQUAL CSKY)
        message("==========SET platform to CSKY==========")
        set (PLATFORM CSKY)
    else(${PLATFORM} STREQUAL ARM64)
        message(FATAL_ERROR "CFG_ERROR PLATFORM=${PLATFORM} error, can not found configuration settings, exit")
    endif (${PLATFORM} STREQUAL ARM64)
else(DEFINED PLATFORM)
    message("==========SET DEFAILT platform to X86==========")
    set (PLATFORM X86)
endif(DEFINED PLATFORM)


if(DEFINED APPMODE)
    if(${APPMODE} STREQUAL V4L2)
    message("========== APPMODE set to V4L2 ==========")
        set (APPMODE_V4L2 1)
        set (V4L2 1)
        add_definitions(-DAPPMODE_V4L2)
        add_definitions(-DSUPPORT_PASS_JSON)
    elseif(${APPMODE} STREQUAL NATIVE)
        message("==========APPMODE set to  NATIVE==========")
        set (APPMODE_NATIVE 1)
        add_definitions(-DAPPMODE_NATIVE)
        add_definitions(-DSUPPORT_PASS_JSON)
    elseif(${APPMODE} STREQUAL MEDIACONTROL)
        message("==========APPMODE set to  NATIVE_MEDIACONTROL==========")
        set (APPMODE_MEDIACONTROL 1)
        add_definitions(-DAPPMODE_MEDIACONTROL)
        add_definitions(-DSUPPORT_PASS_JSON)
    else(${APPMODE} STREQUAL V4L2)
        message(FATAL_ERROR "CFG_ERROR APPMODE=${APPMODE} error, can not found configuration settings, exit")
    endif (${APPMODE} STREQUAL V4L2)
else(DEFINED APPMODE)
    message("==========APPMODE DEFAILT to  NATIVE==========")
    set( APPMODE NATIVE)
    set (APPMODE_NATIVE 1)
    add_definitions(-DAPPMODE_NATIVE)
endif(DEFINED APPMODE)

if(DEFINED WITH_VVBENCH)
    if(${WITH_VVBENCH} STREQUAL 0)
        set (WITH_VVBENCH 0)
    else(${WITH_VVBENCH} STREQUAL 0)
        set (WITH_VVBENCH 1)
    endif(${WITH_VVBENCH} STREQUAL 0)
else(DEFINED WITH_VVBENCH)
    #default value of WITH_DRM
    set (WITH_VVBENCH 0)
endif(DEFINED WITH_VVBENCH)
#for Tuning server compile
#By configuration option choice for selections
#########################################################
#HW configuration begin
#ISP_VERSION selection choices:
# ISP8000NANO
# ISP8000NANO_V1801
# ISP8000NANO_V1802
#
# ISP8000L
# ISP8000L_V1801 *default
#
# ISP8000
#
#
#########################################################

message("## Full source compile, ISP_VERSION input: ${ISP_VERSION}")
#set (PROJECT_CFG_FILE product_cfg_${ISP_VERSION}.cmake)
if(${ISP_VERSION} STREQUAL ISP8000NANO)
message("##NANO CFG")
add_definitions(-DISP8000NANO)
set (PROJECT_CFG_FILE product_cfg_ISP8000NANO.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000NANO_V1801)
message("##NANO_V1801 CFG")
add_definitions(-DISP8000NANO_V1801)
set (PROJECT_CFG_FILE product_cfg_ISP8000NANO_V1801.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000NANO_V1802)
message("##NANO_V1802 CFG")
add_definitions(-DISP8000NANO_V1802)
set (PROJECT_CFG_FILE product_cfg_ISP8000NANO_V1802.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000L)
message("##8000L CFG")
add_definitions(-DISP8000L)
set (PROJECT_CFG_FILE product_cfg_ISP8000L.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000L_V1801)
message("##8000L_V1801 CFG")
add_definitions(-DISP8000L_V1801)
set (PROJECT_CFG_FILE product_cfg_ISP8000L_V1801.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000L_V1901)
message("##8000L_V1901 CFG")
add_definitions(-DISP8000L_V1901)
set (PROJECT_CFG_FILE product_cfg_ISP8000L_V1901.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000L_V1902)
message("##8000L_V1902 CFG")
add_definitions(-DISP8000L_V1902)
set (PROJECT_CFG_FILE product_cfg_ISP8000L_V1902.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000)
message("##8000 CFG")
add_definitions(-DISP8000BASE)
add_definitions(-DISP8000_V01)
set (PROJECT_CFG_FILE product_cfg_ISP8000.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000_V1901)
message("##8000_V1901 CFG")
add_definitions(-DISP8000BASE)
add_definitions(-DISP8000_V1901)
set (PROJECT_CFG_FILE product_cfg_ISP8000_V1901.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000_V2003)
message("##8000_V2003 CFG")
add_definitions(-DISP8000BASE)
add_definitions(-DISP8000_V2003)
set (PROJECT_CFG_FILE product_cfg_ISP8000_V2003.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000L_V2001)
message("##8000L_V2001 CFG")
add_definitions(-DISP8000BASE)
add_definitions(-DISP8000L_V2001)
set (PROJECT_CFG_FILE product_cfg_ISP8000L_V2001.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000L_V2002)
message("##8000L_V2002 CFG")
add_definitions(-DISP8000BASE)
add_definitions(-DISP8000L_V2002)
set (PROJECT_CFG_FILE product_cfg_ISP8000L_V2002.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000L_V2006)
message("##8000L_V2006 CFG")
add_definitions(-DISP8000BASE)
add_definitions(-DISP8000L_V2006)
set (PROJECT_CFG_FILE product_cfg_ISP8000L_V2006.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000L_V1903)
message("##8000L_V1903 CFG")
add_definitions(-DISP8000BASE)
add_definitions(-DISP8000L_V1903)
set (PROJECT_CFG_FILE product_cfg_ISP8000L_V1903.cmake)

elseif(${ISP_VERSION} STREQUAL ISP8000L_V1905)
message("##8000L_V1905 CFG")
add_definitions(-DISP8000BASE)
add_definitions(-DISP8000L_V1905)
set (PROJECT_CFG_FILE product_cfg_ISP8000L_V1905.cmake)
else(${ISP_VERSION} STREQUAL ISP8000NANO)

message(FATAL_ERROR "CFG_ERROR ISP_VERSION=${ISP_VERSION} error, can not found version settings, exit")

endif(${ISP_VERSION} STREQUAL ISP8000NANO)

message("## ISP_VERSION configure setting: ${ISP_VERSION}")

set (PROJECT_CFG_DIR ${CMAKE_HOME_DIRECTORY}/../units/mkrel)
include(${PROJECT_CFG_DIR}/${ISP_VERSION}/${PROJECT_CFG_FILE})

#HW configuration end
#########################################################


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -fPIC")
SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

message(STATUS "=============================================================================")
set(PROJ_DESC "ISP app shell for test app and UIs")

message(STATUS "Project description: " ${PROJ_DESC})

set(PROJECT_SUB_DIR "${PROJECT_BINARY_DIR}/obj")
message(STATUS "project bin:" ${PROJECT_BINARY_DIR})
message(STATUS "project obj dir:" ${PROJECT_SUB_DIR})
message(STATUS "project src:" ${PROJECT_SOURCE_DIR})
message(STATUS "Cmake root dor:" ${CMAKE_HOME_DIRECTORY})

if (NOT LIB_ROOT)
    get_filename_component(LIB_ROOT ${PROJECT_BINARY_DIR}/generated ABSOLUTE)
endif (NOT LIB_ROOT)
message ("LIB_ROOT is at: ${LIB_ROOT}")


if (NOT INSTALL_ROOT)
    get_filename_component(INSTALL_ROOT ${PROJECT_BINARY_DIR}/dist ABSOLUTE)
endif (NOT INSTALL_ROOT)
message ("INSTALL_ROOT is at: ${INSTALL_ROOT}")

# global verbosity enable
###set ( CMAKE_VERBOSE_MAKEFILE 1 )

###set ( FAST_BUILD 1 ) ###NOTE: ONLY FOR EXPERTS

# add some compiler switches for the whole project
if ( WIN32 )
    MESSAGE("system: WIN32")
    add_definitions(-DWIN32)
endif( WIN32 )

if ( UNIX )
    MESSAGE("system: UNIX")
    add_definitions(-DLINUX)
endif ( UNIX )

if ( ANDROID )
    MESSAGE("system: ANDROID")
    add_definitions(-DANDROID)
    set(LIB_PTHREAD_NAME "")
    set(LIB_RT_NAME "")
    set(LIB_LOG_NAME "log")
else ()
    set(LIB_PTHREAD_NAME "pthread")
    set(LIB_RT_NAME "rt")
    set(LIB_LOG_NAME "")
endif ( ANDROID )

#include global rules valid for the whole project:
include (globals.cmake)

if (APPMODE_V4L2)
include_directories(
    ../utils3rd/3rd/jsoncpp/include/ 
    ../utils3rd/include/
    hal/include
    ../units/cam_device/include
    ../vvcam/common

)
endif()

if (APPMODE_NATIVE)
include_directories(
    ../utils3rd/3rd/jsoncpp/include/
    ../utils3rd/include/
    hal/include
    ../units/cam_device/include
    ../vvcam/common

)
endif()

if (APPMODE_MEDIACONTROL)
include_directories(
    ../utils3rd/3rd/jsoncpp/include/
    ../utils3rd/include/
    hal/include
    ../units/cam_device/include
    ../vvcam/common
    cam_menu/include/
)
endif()

add_custom_target(top_clobber
                  COMMAND ${CMAKE_COMMAND} -E remove_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}
                  COMMENT "Delete ${LIB_ROOT}/${CMAKE_BUILD_TYPE}")

add_custom_target(top_create_dir
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_ROOT}
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin/${ISP_VERSION}
                  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/kill_app.sh  ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin/kill_app.sh
                  COMMENT "Create directory ${LIB_ROOT}")

add_custom_target(top_create_alldir
                   COMMENT "Create all modules' include directories")

if ( UNIX )
    add_custom_target(top_strip
                      COMMAND strip --strip-all "${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin/*"
                      COMMENT "Stripping all files in ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin")
    add_custom_target(top_ctags
                      COMMAND ctags -R -f../tags --tag-relative=no  --totals=no  --exclude=.svn --exclude=../debug --exclude=../release --exclude=../build --exclude=${CMAKE_BUILD_TYPE} ..
                      COMMAND ctags -R -f./tags  --tag-relative=yes --totals=yes --exclude=.svn --exclude=../debug --exclude=../release --exclude=../build --exclude=${CMAKE_BUILD_TYPE} ..
                      COMMENT "Creating ctags file for all modules")
endif ( UNIX )

if(${QTLESS} STREQUAL 0)
    set(Qt5_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5")
    set(Qt5Core_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5Core")
    set(Qt5Gui_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5Gui")
    set(Qt5Qml_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5Qml")
    set(Qt5Quick_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5Quick")
    set(Qt5Network_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5Network")
    set(Qt5Multimedia_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5Multimedia")
    set(Qt5MultimediaWidgets_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5MultimediaWidgets")
    set(Qt5Widgets_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5Widgets")
    set(Qt5Xml_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5Xml")
    #set(Qt5Widgets_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5Widgets")
    
    #set(Qt5LinguistTools_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5LinguistTools")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    find_package(Qt5 COMPONENTS Core Gui Qml Quick Multimedia MultimediaWidgets Widgets Xml REQUIRED)

endif(${QTLESS} STREQUAL 0)

# some usefull groups of libraries
set ( base_libs appshell_oslayer_shared appshell_ebase_shared )

# add all units of the project here:
# - base modules first
add_subdirectory(oslayer ${PROJECT_SUB_DIR}/appshell_oslayer )
add_subdirectory(ibd ${PROJECT_SUB_DIR}/appshell_ibd )
add_subdirectory(${PROJECT_SOURCE_DIR}/../utils3rd/3rd/jsoncpp ${PROJECT_SUB_DIR}/jsoncpp)
add_subdirectory(ebase ${PROJECT_SUB_DIR}/appshell_ebase)
add_subdirectory(yuv ${PROJECT_SUB_DIR}/yuv)

add_subdirectory(display ${PROJECT_SUB_DIR}/display)

if(${FULL_SRC_COMPILE} STREQUAL 0)
    message("Using library compile")
    add_subdirectory( shell_libs ${PROJECT_SUB_DIR}/shell_libs )
 else(${FULL_SRC_COMPILE} STREQUAL 0)
    message("Using full source compile")
    set ( units_libs jsoncpp_lib cam_device_shared)

    add_subdirectory( ${PROJECT_SOURCE_DIR}/../units ${PROJECT_SUB_DIR}/units )
if (SUBDEV_V4L2)
    add_subdirectory( ${PROJECT_SOURCE_DIR}/../mediacontrol ${PROJECT_SUB_DIR}/mediacontrol )
endif()
endif(${FULL_SRC_COMPILE} STREQUAL 0)


add_subdirectory( hal ${PROJECT_SUB_DIR}/appshell_hal)

if(${WITH_VVBENCH} STREQUAL 1)
    add_subdirectory( vvbench ${PROJECT_SUB_DIR}/vvbench )
endif(${WITH_VVBENCH} STREQUAL 1)

if (SUBDEV_CHAR)
add_definitions(-DSUBDEV_CHAR)
endif()

if (SUBDEV_V4L2)
add_definitions(-DSUBDEV_V4L2)
endif()

if(${QTLESS} STREQUAL 0)
#    add_subdirectory( cam_app ${PROJECT_SUB_DIR}/cam_app )
    add_subdirectory( dom_ctrl ${PROJECT_SUB_DIR}/appshell_dom_ctrl )

endif(${QTLESS} STREQUAL 0)

if(${SERVER_LESS} STREQUAL 0)
    #tunning server
    if(NOT ANDROID)
        add_subdirectory( ../tuning-server ${PROJECT_SUB_DIR}/tuning-server )
    endif()
endif(${SERVER_LESS} STREQUAL 0)

if (APPMODE_V4L2)
    if(NOT ANDROID)
        add_subdirectory(v4l_drm_test ${PROJECT_SUB_DIR}/v4l_drm_test )
    endif(NOT ANDROID)
add_subdirectory( vvext ${PROJECT_SUB_DIR}/vvext )
endif(APPMODE_V4L2)
