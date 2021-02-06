cmake_minimum_required(VERSION 2.6)

# define module name
set (module appshell_oslayer)

# we want to compile certain files only; depending on environment
if ( WIN32 )
    set ( libsources source/oslayer_win32.c )
else ()
    # Assume that linux is the standard platform
    # TODO: implement other platforms
    set ( libsources source/oslayer_linux.c )
    set ( dynamic_system_libs ${LIB_RT_NAME} ${LIB_PTHREAD_NAME} )
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLINUX" )
endif ( WIN32 )
# but want to always compile the generic stuff
set ( libsources ${libsources} source/oslayer_generic.c )

# set public headers, these get installed
file(GLOB pub_headers include/*.h)

# define include paths
include_directories(
  include
  include_priv
  ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
  )

# module specific defines
#add_definitions(-D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H -Wno-error=unused-function)
add_definitions (-Wno-long-long)
add_definitions (-fPIC)

# add lib to build env
add_library(${module}_shared SHARED ${libsources})

SET_TARGET_PROPERTIES(${module}_shared PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES LINK_FLAGS      -shared)
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

# define lib dependencies
target_link_libraries(${module}_shared
                      appshell_ebase_shared
                      ${LIB_PTHREAD_NAME}
                      )

# define stuff to install
install(TARGETS ${module}_shared
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${module}
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${module}
        LIBRARY         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${module}
        )

# create common targets for this module
include(${CMAKE_HOME_DIRECTORY}/targets.cmake)
