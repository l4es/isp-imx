cmake_minimum_required( VERSION 2.6 )

set ( module_qtapp ${module_qt}_test_appl )

###add_definitions( -DQT_NO_DEBUG_OUTPUT )   ### add to remove qDebug   output
###add_definitions( -DQT_NO_WARNING_OUTPUT ) ### add to remove qWarning output

if (NOT LOCAL_QT_FIND_INCLUDE)
    ### can't use this, as support for QtMultimedia is missing in our cmake's Qt support files:
    find_package( Qt4 COMPONENTS QtCore QtGui QtMultimedia REQUIRED )
else (NOT LOCAL_QT_FIND_INCLUDE)
    ### use this instead, ${CMAKE_SOURCE_DIR}/FindQt4.cmake is a suitably modified replacement,
    ### but we have to set some control variables by hand first:
    set ( Qt4_FIND_COMPONENTS QtCore QtGui QtMultimedia )
    set ( Qt4_FIND_REQUIRED 1 )
    include ( ${CMAKE_SOURCE_DIR}/FindQt4.cmake )
endif (NOT LOCAL_QT_FIND_INCLUDE)

if ( NOT QT4_FOUND )
    message( "-----------------------------" )
    message( FATAL_ERROR "QT4 not found" )
endif ( NOT QT4_FOUND )

include( ${QT_USE_FILE} )

include_directories( ../include_priv include_priv include ${LIB_ROOT}/include )

file(GLOB _MOC_FILES_COMMON include/*.h include_priv/*.h)
file(GLOB _SOURCES_COMMON source/*.cpp)
file(GLOB _RESSOURCE_FILES ressource/*.qrc)

qt4_wrap_cpp( _MOC_FILES ${_MOC_FILES_COMMON} )
QT4_ADD_TRANSLATION( QM_FILES )
QT4_ADD_RESOURCES( GUI_QT_RESOURCES )

# Add build target for the binary
add_executable( ${module_qtapp}
                ${QM_FILES}
                ${GUI_QT_RESOURCES}
                ${_SOURCES_COMMON}
                ${_MOC_FILES}
                )

target_link_libraries( ${module_qtapp}
                       ${module_qt}_shared
                       ${module}_shared
                       ${base_libs}
                       m
                       ${QT_LIBRARIES}
                       )
