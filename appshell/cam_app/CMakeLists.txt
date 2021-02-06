cmake_minimum_required(VERSION 2.6)

# define application name
set (application cam_app)

if (CMAKE_BUILD_TYPE STREQUAL RELEASE)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_NO_DEBUG_OUTPUT -DQT_NO_WARNING_OUTPUT")
endif (CMAKE_BUILD_TYPE STREQUAL RELEASE)


#include( ${QT_USE_FILE} )

# defines files to compile
set( MOC_FILES_COMMON
#include_priv/level1/calibdatawidget.h
include_priv/level1/isiregmodel.h
include_priv/level1/propertiessensorwidget.h
include_priv/level1/registerwidget.h
#include_priv/calib_xml/calibtreewidget.h
include_priv/calib_xml/calibtags.h
include_priv/captureimagedialog.h
include_priv/level2tab.h
include_priv/mainwindow.h
include_priv/Scene/scene.h
include_priv/level3tab.h
include_priv/imageviewer.h
include_priv/level2/imgeffectswidget.h
include_priv/level2/wdrplot.h
include_priv/level2/wdrwidget.h
include_priv/level2/cprocwidget.h
include_priv/level2/canvaspicker.h
#include_priv/Matrix/matrix.h
#include_priv/cac/caccanvaspicker.h
include_priv/cac/cac-plot.h
include_priv/level1tab.h
#include_priv/level4tab.h
include_priv/Histogram/HistPlot.h
include_priv/Histogram/Histogram.h
#include_priv/Oscilloscope/plot.h
#include_priv/Oscilloscope/mainwindow.h
#include_priv/Oscilloscope/samplingthread.h
#include_priv/Oscilloscope/curvedata.h
#include_priv/Oscilloscope/signaldata.h
include_priv/envtab.h
include_priv/level3/exposureplot.h
include_priv/level3/aecwidget.h
include_priv/level3/awbwidget.h
    )


set( UI_FILES_COMMON
        form/propertiessensorwidget.ui
    )

set( SOURCES_COMMON
    source/imageviewer.cpp
    source/captureimagedialog.cpp
    source/mainwindow.cpp
    source/envtab.cpp
    source/level1tab.cpp
    source/level2tab.cpp
    source/level3tab.cpp
#    source/level4tab.cpp
    source/main.cpp
    source/level1/isiregmodel.cpp
    source/level1/propertiessensorwidget.cpp
    source/level1/registerwidget.cpp
    #source/level1/calibdatawidget.cpp
    source/level2/cprocwidget.cpp
    source/level2/imgeffectswidget.cpp
    source/level2/wdrwidget.cpp
    source/level2/wdrplot.cpp
    source/level2/canvaspicker.cpp
    source/level3/awbwidget.cpp
    source/level3/aecwidget.cpp
    source/level3/exposureplot.cpp
    #source/calib_xml/calibtreewidget.cpp
    source/cac/cac-plot.cpp
#    #source/cac/caccanvaspicker.cpp
    source/Histogram/Histogram.cpp
    source/Histogram/HistPlot.cpp
	source/Scene/scene.cpp
#    source/Matrix/matrix.cpp
#    source/Oscilloscope/curvedata.cpp
#    #source/Oscilloscope/mainwindow.cpp
#    source/Oscilloscope/plot.cpp
#    source/Oscilloscope/samplingthread.cpp
#    source/Oscilloscope/signaldata.cpp
    )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set( RESSOURCE_FILES
    ressource/application.qrc
    )

# define include paths
include_directories(
    include
    include_priv
    #${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
    ${CMAKE_CURRENT_BINARY_DIR} ### for the generated ui_*.h
    utils/base64
    #${LIBPFID_INCLUDE_DIRS}
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
    ${CMAKE_PREFIX_PATH}/include
    ${CMAKE_PREFIX_PATH}/include/QtCore
    ${CMAKE_PREFIX_PATH}/include/QtGui
    ${CMAKE_PREFIX_PATH}/include/QtMultimedia
    ${CMAKE_PREFIX_PATH}/include/QtWidgets
    ${CMAKE_PREFIX_PATH}/include/QtMultimediaWidgets
    ${QWT6_PATH}/include
    )

link_directories(
    ${LIBPFID_LIBRARY_DIRS}
    ${QWT6_PATH}/lib
    )
# module specific defines
add_definitions(-Wno-error=unused-function -DQT_NO_DEBUG_OUTPUT -DQT_NO_PRINTER)

# add exec to build env
add_executable(${application}
               ${QM_FILES}
               ${GUI_QT_RESOURCES}
               ${SOURCES_COMMON}
               ${MOC_FILES_COMMON}
               ${MOC_FILES}
               ${UI_FILES}
               )

set(cam_app_depend_libs 
    m
    ${units_libs}
    ${QT_LIBRARIES}
    qwt
    cam_app_base64_shared
    appshell_hal_shared
    appshell_dom_ctrl_shared
    appshell_dom_ctrl_qt_shared
    Qt5::Core
    Qt5::Gui
    Qt5::Qml 
    Qt5::Quick 
    Qt5::Multimedia 
    Qt5::MultimediaWidgets
    Qt5::Widgets
    Qt5::Xml
    )

# define lib dependencies
if(${FULL_SRC_COMPILE} STREQUAL 0)
    target_link_libraries(${application}
                    shell_libs_shared
                    ${cam_app_depend_libs}
    )
else(${FULL_SRC_COMPILE} STREQUAL 0)
    target_link_libraries(${application}
                    ${cam_app_depend_libs}
    )
endif(${FULL_SRC_COMPILE} STREQUAL 0)

# define stuff to install
install(TARGETS ${application} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_subdirectory(utils)
# create common targets for this module
set(appsources ${SOURCES_COMMON}) # 'targets_app.cmake' requires this variable being set as input
include(${CMAKE_HOME_DIRECTORY}/targets_app.cmake)
