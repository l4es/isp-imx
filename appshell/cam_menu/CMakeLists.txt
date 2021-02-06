cmake_minimum_required(VERSION 3.1.0)

set(APP cam_menu)

# include directories
find_package(PythonLibs 2.7 REQUIRED)
include_directories(
    include
    ${PYTHON_INCLUDE_DIRS}
    $ENV{SDKTARGETSYSROOT}/usr/include/drm
    )

file(GLOB FILE_SRC source/*.c source/*.cpp)

add_executable(${APP}
    ${FILE_SRC}
)

target_link_libraries(
    ${APP}
    jsoncpp_lib
    ${PYTHON_LIBRARIES}
    ${LIB_PTHREAD_NAME} drm
)

add_custom_target(copy_script_files ALL
                  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_HOME_DIRECTORY}/cam_menu/script ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin
                  COMMENT "Copy scripts to ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin")
add_dependencies(copy_script_files top_create_dir)

install(TARGETS ${APP} DESTINATION bin)
