cmake_minimum_required(VERSION 2.6)

# define module name
set (module shell_libs)

# but want to always compile the generic stuff
# we want to compile all .c files as default
file(GLOB libsources source/*.c source/*.cpp)

set (camdev_include include)
if(${APPMODE} STREQUAL V4L2)
    message("========== cam_device V4L2 implementation==========")
    set(camdev_libs    V4L2)
elseif(${APPMODE} STREQUAL NATIVE)
    message("========== cam_device native implementation==========")
    set(camdev_libs    NATIVE)
elseif(${APPMODE} STREQUAL MEDIACONTROL)
    message("========== cam_device native media control  implementation==========")
    set(camdev_libs    MEDIACONTROL)
else(${APPMODE} STREQUAL V4L2)
    message(FATAL_ERROR "CFG_ERROR APPMODE=${APPMODE} error, can not found currect vitrtualizaiton, exit")
endif (${APPMODE} STREQUAL V4L2)



# set public headers, these get installed
file(GLOB pub_headers ${camdev_include}/*.h ${camdev_include}/*.hpp)

# define include paths
include_directories(
  ${camdev_include}
  ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
  )

add_definitions (-fPIC)

# add lib to build env
add_library(${module}_shared SHARED ${libsources})

SET_TARGET_PROPERTIES(${module}_shared PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES LINK_FLAGS      -shared)
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

# define lib dependencies
if(${APPMODE} STREQUAL NATIVE)
    target_link_libraries(${module}_shared
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libcam_device.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libadpcc.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libadpf.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libaec.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libaf.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libavs.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libawb.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libbase64.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libcameric_drv.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libisi.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libov2775.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libtinyxml2.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libibd.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libcam_engine.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/liboslayer.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libcam_calibdb.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libcommon.so
                      jsoncpp_lib
                      )
elseif(${APPMODE} STREQUAL V4L2)
    target_link_libraries(${module}_shared
                      jsoncpp_lib
                      )
elseif(${APPMODE} STREQUAL MEDIACONTROL)
    target_link_libraries(${module}_shared
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libcam_device.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libadpcc.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libadpf.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libaec.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libaf.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libavs.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libawb.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libbase64.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libcameric_drv.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libisi.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libov2775.so
                      ${CMAKE_HOME_DIRECTORY}/shell_libs/${camdev_libs}/ispcore/${PLATFORM}/libtinyxml2.so
                      jsoncpp_lib
                      )
endif (${APPMODE} STREQUAL NATIVE)

# define stuff to install
install(TARGETS ${module}_shared
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/${camdev_include}/${module}
        LIBRARY         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        )


add_custom_target(copy_sensor_database ALL
                  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_HOME_DIRECTORY}/shell_libs/sensor ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin
                  COMMENT "Copy sensor database to ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin")
add_dependencies(copy_sensor_database top_create_dir)

if(${APPMODE} STREQUAL V4L2)
    #set up mediacontrol and native stack
    #todo
endif (${APPMODE} STREQUAL V4L2)


# create common targets for this module
include(${CMAKE_HOME_DIRECTORY}/targets.cmake)
