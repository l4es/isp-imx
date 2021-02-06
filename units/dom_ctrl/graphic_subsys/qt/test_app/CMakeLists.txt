cmake_minimum_required( VERSION 2.6 )

set ( module_qtapp ${module_qt}_test_appl )

###add_definitions( -DQT_NO_DEBUG_OUTPUT )   ### add to remove qDebug   output
###add_definitions( -DQT_NO_WARNING_OUTPUT ) ### add to remove qWarning output

#include( ${QT_USE_FILE} )

include_directories( ../include_priv include_priv include ${LIB_ROOT}/include )

file(GLOB MOC_FILES_COMMON include/*.h include_priv/*.h)
file(GLOB SOURCES_COMMON source/*.cpp)
file(GLOB RESSOURCE_FILES ressource/*.qrc)

set(qt_depend_libs

    ${units_libs}
    ${QT_LIBRARIES}
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Multimedia
    Qt5::MultimediaWidgets
    Qt5::Widgets
    Qt5::Xml
    )

# Add build target for the binary
add_executable( ${module_qtapp}
                ${QM_FILES}
                ${GUI_QT_RESOURCES}
                ${SOURCES_COMMON}
                ${MOC_FILES_COMMON}
                ${MOC_FILES}
                )

target_link_libraries( ${module_qtapp}
                       ${module_qt}_shared
                       ${module}_shared
                       ${base_libs}
                       m
                       ${qt_depend_libs}
                       )
