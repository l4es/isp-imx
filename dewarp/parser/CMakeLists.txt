cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

add_definitions(-Wall)

set (module dewarp_parser)

file(GLOB libsources *.cpp)

file(GLOB pub_headers *.h)

add_library(${module}_static STATIC ${libsources})

target_link_libraries ( ${module}_static ${platform_libs} )

SET_TARGET_PROPERTIES(${module}_static PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_static PROPERTIES LINK_FLAGS      "-static -fPIC")
SET_TARGET_PROPERTIES(${module}_static PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

if(GENERATE_PARTITION_BUILD)
    if (DEFINED APPSHELL_TOP_COMPILE)
        add_custom_target(copy_libs_${module} ALL
            COMMENT "##Copy share lib to propritories"
            COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/../../generated/${CMAKE_BUILD_TYPE}/lib/lib${module}.a ${CMAKE_CURRENT_SOURCE_DIR}/../proprietories/parser/lib/lib${module}.a
        )
    	add_dependencies(copy_libs_${module} ${module}_static)
    endif(NOT DEFINED APPSHELL_TOP_COMPILE)
endif(GENERATE_PARTITION_BUILD)
