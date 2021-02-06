cmake_minimum_required(VERSION 2.6)

# define module name
set (module cam_engine)

# set public headers, these get installed
file(GLOB_RECURSE pub_headers include/*.h)

# define include paths
include_directories(
    include
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
    )

add_library(${module}_shared STATIC IMPORTED GLOBAL)
set_property(TARGET ${module}_shared PROPERTY IMPORTED_LOCATION ${module}_shared)

add_custom_target(copy_libs_${module} ALL
       COMMENT "##Copy ${module} headers to include" 
       COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/include ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
       COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/lib${module}.so ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/lib${module}.so
)
# create common targets for this module
include(${UNITS_TOP_DIRECTORY}/targets.cmake)

