cmake_minimum_required(VERSION 3.1.0)

set(APP vvbench)


if(${QTLESS} STREQUAL 0)
#QT env available
    set (QT_INCLUDE
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
        qwt
        Qt5::Core
        Qt5::Gui
        Qt5::Qml 
        Qt5::Quick 
        Qt5::Multimedia 
        Qt5::MultimediaWidgets
        Qt5::Widgets
        Qt5::Xml
#        qxcb
        )

    set (QT_DISPLAY_FULL_SRC_LIBS
        appshell_dom_ctrl_shared
        appshell_dom_ctrl_qt_shared
        )

    set(QT_DISPLAY_LIBS
        dom_ctrl appshell_dom_ctrl_qt_shared
        )

    set (QT_LINK_DIR
		${CMAKE_PREFIX_PATH}/lib
        ${QWT6_PATH}/lib
        ${CMAKE_PREFIX_PATH}/plugins/platforms
        )
    add_definitions(-DDOM_OUTOUT)
else(${QTLESS} STREQUAL 0)
    #QT env does not available
    set (QT_INCLUDE )
    set (QT_LIBS )
    set (QT_DISPLAY_LIBS )
    set (QT_DISPLAY_FULL_SRC_LIBS )
    set (QT_LINK_DIR )
    

endif(${QTLESS} STREQUAL 0)


if(${FULL_SRC_COMPILE} STREQUAL 0)
    #link libs headers
    set(UNITS_HEADER_FILES 
        ${CMAKE_HOME_DIRECTORY}/shell_libs/include/units_headers/
        )

else(${FULL_SRC_COMPILE} STREQUAL 0)
    #full src compile
    set(UNITS_HEADER_FILES )

endif(${FULL_SRC_COMPILE} STREQUAL 0)



# include directories
include_directories(
    include
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
    ${QT_INCLUDE}
    ../../utils3rd/3rd/jsoncpp/include
    ../../units/cam_device/include
	../dom_ctrl/include
    ${UNITS_HEADER_FILES}
    )


link_directories(
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib
    ${QT_LINK_DIR}
)

file(GLOB VV_SRC src/*.cpp)


#add_definitions(-D_DEFAULT_SOURCE -DCHIP_8000L) # {CHIP_8000L, CHIP_NANO}

add_executable(${APP}
    ${VV_SRC}
)

if(${FULL_SRC_COMPILE} STREQUAL 0)
    set (NATIVE_LIB shell_libs_shared)
else(${FULL_SRC_COMPILE} STREQUAL 0)
    set (NATIVE_LIB cam_device_shared)
endif(${FULL_SRC_COMPILE} STREQUAL 0)

target_link_libraries(
    ${APP}
    ${QT_LIBS}
    ${QT_DISPLAY_FULL_SRC_LIBS}
    ${LIB_PTHREAD_NAME}
    m
    jsoncpp_lib
    ${NATIVE_LIB}
)


if (NOT EXISTS ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin/vvbcfg)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_HOME_DIRECTORY}/vvbench/vvbcfg ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin/vvbcfg)
endif()

install(TARGETS ${APP} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
