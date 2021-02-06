cmake_minimum_required(VERSION 2.6)

# define module name
set (module xom_ctrl)

# we want to compile all .c files as default
file(GLOB libsources source/*.c)

# set public headers, these get installed
file(GLOB pub_headers include/*.h)

# define include paths
include_directories(
    include
    include_priv
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
    )

# module specific defines
###add_definitions(-Wno-error=unused-function)

# add lib to build env
add_library(${module}_shared SHARED ${libsources})

SET_TARGET_PROPERTIES(${module}_shared PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES LINK_FLAGS      -shared)
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

# define lib dependencies
#target_link_libraries(${module}_shared
#                      ${platform_libs}
#                      ${base_libs}
#                      )

# define stuff to install
install(TARGETS ${module}_shared
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${module}
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${module}
        LIBRARY         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${module}
        )

if( DEFINED APPSHELL_TOP_COMPILE)
add_custom_target(copy_shell_libs_${module} ALL
       COMMENT "##Copy libs to shell libs"
       COMMAND ${CMAKE_COMMAND} -E copy ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/lib${module}.so ${CMAKE_HOME_DIRECTORY}/shell_libs/${APPMODE}/ispcore/${PLATFORM}/lib${module}.so
       #COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include/${module} ${CMAKE_HOME_DIRECTORY}/shell_libs/include/units_headers/${module}
)
add_dependencies(copy_shell_libs_${module} ${module}_shared)
endif( DEFINED APPSHELL_TOP_COMPILE)

# create common targets for this module
include(${UNITS_TOP_DIRECTORY}/targets.cmake)
