cmake_minimum_required(VERSION 2.6)

# define module name
set (module propriteories)
set (aaa_prop_list a2dnr a3dnr adpcc adpf aec af aflt ahdr avs awb awdr3 aee)

# set public headers, these get installed
file(GLOB_RECURSE pub_headers include/*.h)

file(GLOB aaa_libs lib/*.so)

# define include paths
include_directories(
    include
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
    )

foreach(_var ${aaa_prop_list})
    file(GLOB ${_var}_lib lib/lib${_var}.so)
    message("###${_var}_libs is ${${_var}_lib}")
    add_library(${_var}_shared SHARED IMPORTED GLOBAL)
    set_property(TARGET ${_var}_shared PROPERTY IMPORTED_LOCATION ${${_var}_lib})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/lib${_var}.so ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/lib${_var}.so)
endforeach()


add_custom_target(copy_libs_${module} ALL
       COMMENT "##Copy 3A headers to include" 
       COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/include ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
)

# create common targets for this module
include(${UNITS_TOP_DIRECTORY}/targets.cmake)

