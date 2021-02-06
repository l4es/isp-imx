cmake_minimum_required( VERSION 2.6 )

set ( module_qt ${module}_qt )

###add_definitions( -DQT_NO_DEBUG_OUTPUT )   ### add to remove qDebug   output
###add_definitions( -DQT_NO_WARNING_OUTPUT ) ### add to remove qWarning output
add_definitions(-DFB_ENABLE)
add_definitions(-DFB_DEV="/dev/fb0")

#include( ${QT_USE_FILE} )

include_directories(
    ../include_priv
    include_priv
    include
    ${LIB_ROOT}/include
    ${CMAKE_PREFIX_PATH}/include
    ${CMAKE_PREFIX_PATH}/include/QtCore
    ${CMAKE_PREFIX_PATH}/include/QtGui
    ${CMAKE_PREFIX_PATH}/include/QtMultimedia
    ${CMAKE_PREFIX_PATH}/include/QtWidgets
    ${CMAKE_PREFIX_PATH}/include/QtMultimediaWidgets
    ${QWT6_PATH}/include
     )

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

file(GLOB MOC_FILES_COMMON include/*.h include_priv/*.h)
file(GLOB SOURCES_COMMON source/*.cpp)
file(GLOB RESSOURCE_FILES ressource/*.qrc)


# add build target
add_library( ${module_qt}_shared SHARED
             ${QM_FILES}
             ${GUI_QT_RESOURCES}
             ${SOURCES_COMMON}
             ${MOC_FILES}
             ${MOC_FILES_COMMON}
             )

set_target_properties( ${module_qt}_shared PROPERTIES OUTPUT_NAME ${module}_qt )
set_target_properties( ${module_qt}_shared PROPERTIES LINK_FLAGS -shared)
set_target_properties( ${module_qt}_shared PROPERTIES FRAMEWORK TRUE )


set (QT_LINK_DIR
${QWT6_PATH}/lib
${CMAKE_PREFIX_PATH}/plugins/platforms
)

link_directories(
  ${QT_LINK_DIR}
)

    target_link_libraries( ${module_qt}_shared
#                       ${base_libs}
                       ${QT_LIBS}
                       )

if( DEFINED APPSHELL_TOP_COMPILE)
add_custom_target(copy_shell_libs_${module_qt} ALL
       COMMENT "##Copy libs to shell libs"
       COMMAND ${CMAKE_COMMAND} -E copy ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib/lib${module_qt}.so ${CMAKE_HOME_DIRECTORY}/shell_libs/${APPMODE}/ispcore/${PLATFORM}/lib${module_qt}.so
)
add_dependencies(copy_shell_libs_${module_qt} ${module_qt}_shared)
endif( DEFINED APPSHELL_TOP_COMPILE)

add_dependencies(${module_qt}_shared ${base_libs})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# add graphic subsystem test appl
#add_subdirectory( test_app )
