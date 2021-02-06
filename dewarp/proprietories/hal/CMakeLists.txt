cmake_minimum_required(VERSION 2.6)

# define module name
set (module dewarp_hal)

file(GLOB dewarp_hal_lib lib/libdewarp_hal.so)
message("###dewarp_hal_lib is ${dewarp_hal_lib}")
add_library(dewarp_hal_shared SHARED IMPORTED GLOBAL)
set_property(TARGET dewarp_hal_shared PROPERTY IMPORTED_LOCATION ${dewarp_hal_lib})

if(NOT DEFINED APPSHELL_TOP_COMPILE)
    add_custom_target(copy_libs_${module} ALL
        COMMENT "##Copy hal library to lib"
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/libdewarp_hal.so ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/ 
)
else(NOT DEFINED APPSHELL_TOP_COMPILE)
    add_custom_target(copy_libs_${module} ALL
        COMMENT "##Copy hal library to lib"
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/libdewarp_hal.so  ${PROJECT_BINARY_DIR}/../../generated/${CMAKE_BUILD_TYPE}/lib/libdewarp_hal.so
    )
endif(NOT DEFINED APPSHELL_TOP_COMPILE)
