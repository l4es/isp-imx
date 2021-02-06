cmake_minimum_required(VERSION 2.6)

# define module name
set (module scmi)

# set public headers, these get installed
file(GLOB pub_headers include/*.h)

# this installs the libs and header for the target lib_install
add_custom_target(${module}
                  COMMENT "Installing ${module}")

if( DEFINED APPSHELL_TOP_COMPILE)
add_custom_target(copy_shell_libs_${module} ALL
       COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include/${module} ${CMAKE_HOME_DIRECTORY}/shell_libs/include/units_headers/${module}
)
add_dependencies(copy_shell_libs_${module} ${module})
endif( DEFINED APPSHELL_TOP_COMPILE)

# create common targets for this module
include(${UNITS_TOP_DIRECTORY}/targets_api.cmake)
