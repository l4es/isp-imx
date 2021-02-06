cmake_minimum_required(VERSION 2.6)
if (GENERATE_PARTITION_BUILD)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

add_definitions(-Wall)

set (module appshell_hal)

file(GLOB libsources src/*.cpp)

file(GLOB pub_headers include/*.h)
include_directories(
    include
)

add_library(${module}_shared SHARED ${libsources})

message(${PROJECT_SOURCE_DIR})

if(${FULL_SRC_COMPILE} STREQUAL 0)
    target_link_libraries ( ${module}_shared  ${LIB_PTHREAD_NAME} shell_libs_shared)
else(${FULL_SRC_COMPILE} STREQUAL 0)
    target_link_libraries ( ${module}_shared  ${LIB_PTHREAD_NAME} cam_device_shared)
endif(${FULL_SRC_COMPILE} STREQUAL 0)

SET_TARGET_PROPERTIES(${module}_shared PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES LINK_FLAGS      "-shared -fPIC")
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")


# define stuff to install
install(TARGETS ${module}_shared
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${module}
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        LIBRARY         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${module}
        )

if(GENERATE_PARTITION_BUILD)
add_custom_target(copy_libs_${module} ALL
       COMMENT "##Copy cam_engine static libs to propritories"
       COMMAND ${CMAKE_COMMAND} -E copy ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/lib${module}.so ${CMAKE_CURRENT_SOURCE_DIR}/proprietories/lib/lib${module}.so
       COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include/${module} ${CMAKE_CURRENT_SOURCE_DIR}/proprietories/include
)
add_dependencies(copy_libs_${module} ${module}_shared)
endif(GENERATE_PARTITION_BUILD)

if( DEFINED APPSHELL_TOP_COMPILE)
    # create common targets for this module
    include(${CMAKE_HOME_DIRECTORY}/targets.cmake)
endif( DEFINED APPSHELL_TOP_COMPILE)

else (GENERATE_PARTITION_BUILD)
add_subdirectory( proprietories )
endif (GENERATE_PARTITION_BUILD)
