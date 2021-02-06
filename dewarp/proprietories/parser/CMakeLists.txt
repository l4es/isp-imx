cmake_minimum_required(VERSION 2.6)

# define module name
set (module dewarp_parser)

file(GLOB dewarp_parser_lib lib/libdewarp_parser.a)
message("###dewarp_parser_lib is ${dewarp_parser_lib}")
add_library(dewarp_parser_static STATIC IMPORTED GLOBAL)
set_property(TARGET dewarp_parser_static PROPERTY IMPORTED_LOCATION ${dewarp_parser_lib})

if(NOT DEFINED APPSHELL_TOP_COMPILE)
    add_custom_target(copy_libs_${module} ALL
        COMMENT "##Copy parser library to lib"
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/libdewarp_parser.a  ${CMAKE_INSTALL_LIBDIR}/libdewarp_parser.a
    )
else(NOT DEFINED APPSHELL_TOP_COMPILE)
    add_custom_target(copy_libs_${module} ALL
        COMMENT "##Copy parser library to lib"
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/libdewarp_parser.a  ${PROJECT_BINARY_DIR}/../../generated/${CMAKE_BUILD_TYPE}/lib/libdewarp_parser.a
    )
endif(NOT DEFINED APPSHELL_TOP_COMPILE)

