cmake_minimum_required(VERSION 2.6)

# define module name
set (module cam_app_base64)

# we want to compile all .c files as default
file(GLOB libsources *.cpp)

# set public headers, these get installed
file(GLOB pub_headers *.hpp)

# define include paths
include_directories(
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
    )

# module specific defines
###add_definitions(-Wno-error=unused-function)

# add lib to build env
add_library(${module}_shared SHARED ${libsources})

SET_TARGET_PROPERTIES(${module}_shared PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES LINK_FLAGS      -shared)
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

target_link_libraries(${module}_shared
                      )

install(TARGETS ${module}_shared
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${module}
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${module}
        LIBRARY         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${module}
       )

# create common targets for this module
include(${CMAKE_HOME_DIRECTORY}/targets.cmake)
