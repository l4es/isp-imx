cmake_minimum_required(VERSION 2.6)

# define module name
set (module dom_ctrl)


# we want to compile all .c files as default
file(GLOB libsources source/*.c)

# set public headers, these get installed
file(GLOB pub_headers include/*.h)

# define include paths
include_directories(
    include
    include_priv
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
    ${LIB_ROOT}/include
    ${CMAKE_PREFIX_PATH}/include
    ${CMAKE_PREFIX_PATH}/include/QtCore
    ${CMAKE_PREFIX_PATH}/include/QtGui
    ${CMAKE_PREFIX_PATH}/include/QtMultimedia
    ${CMAKE_PREFIX_PATH}/include/QtWidgets
    ${CMAKE_PREFIX_PATH}/include/QtMultimediaWidgets
    ${QWT6_PATH}/include
    )

# module specific defines
###add_definitions(-Wno-error=unused-function)
add_definitions("-O3") ### force optimization for all targets to speed up color space conversion stuff

set ( graphic_subsystem qt ) #### select graphics subsystem to use: for now only Qt is supported (use all-lower-case 'qt' in set command)

# add lib to build env
add_library(${module}_shared SHARED ${libsources})

SET_TARGET_PROPERTIES(${module}_shared PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES LINK_FLAGS      -shared)
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

set (QT_LIBS
     ${QT_LIBRARIES}
     #qwt
     Qt5::Core
     Qt5::Gui
     Qt5::Qml
     Qt5::Quick
     Qt5::Multimedia
     Qt5::MultimediaWidgets
     Qt5::Widgets
     Qt5::Xml
     #qxcb
     )

set (QT_LINK_DIR
${QWT6_PATH}/lib
${CMAKE_PREFIX_PATH}/plugins/platforms
)

#link_directories(
#  ${QT_LINK_DIR}
#)

# define lib dependencies
target_link_libraries(${module}_shared
#                      ${module}_${graphic_subsystem}_shared
#                      ${platform_libs}
#                      ${base_libs}
                      ibd_shared
                      ${QT_LIBS}
                      )

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
       COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include/${module} ${CMAKE_HOME_DIRECTORY}/shell_libs/include/units_headers/${module}
)
add_dependencies(copy_shell_libs_${module} ${module}_shared)
endif( DEFINED APPSHELL_TOP_COMPILE)

# create common targets for this module
include(${CMAKE_HOME_DIRECTORY}/targets.cmake)

# add graphic subsystems
add_subdirectory(graphic_subsys/${graphic_subsystem})

# add unit tests and example appl subdirs
# add_subdirectory(unit_test)
###add_subdirectory(example_app)
