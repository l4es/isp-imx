###############################################################################
# Determine targets for the module
###############################################################################
get_target_property(HAVE_BARE_TARGET   ${module}        TYPE)
get_target_property(HAVE_STATIC_TARGET ${module}_static TYPE)
get_target_property(HAVE_SHARED_TARGET ${module}_shared TYPE)
list(LENGTH pub_headers HAVE_PUBLIC_HEADERS)

###############################################################################
# Create directory for the modules
###############################################################################
add_custom_target(${module}_create_dir
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include/${module}
                  COMMENT "Create dir for ${module}")
add_dependencies(${module}_create_dir top_create_dir)

get_target_property(TOP_CREATE_ALLDIR top_create_alldir TYPE)
if(NOT TOP_CREATE_ALLDIR)
    add_custom_target(top_create_alldir)
endif()
# the library itself is a dependency for the target lib
add_dependencies(top_create_alldir ${module}_create_dir)


###############################################################################
# Settings for target LIB
###############################################################################
# check if lib target already exists and create it, this is to assure that only one target called lib will be generated
get_target_property(LIB_TARGET lib TYPE)
if(NOT LIB_TARGET)
    add_custom_target(lib)
endif()
# the library itself is a dependency for the target lib
if(HAVE_BARE_TARGET)
  add_dependencies(lib ${module})
endif()
if(HAVE_STATIC_TARGET)
  add_dependencies(lib ${module}_static)
endif()
if(HAVE_SHARED_TARGET)
  add_dependencies(lib ${module}_shared)
endif()


###############################################################################
# Settings for target LIB_INSTALL
###############################################################################
# this installs the libs and header for the target lib_install
add_custom_target(${module}_lib_install_static
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/lib
                    COMMAND install -p -m 644 -t ${CMAKE_INSTALL_PREFIX}/lib ${LIBRARY_OUTPUT_PATH}/lib${module}.a
                    COMMENT "Installing ${module}_static")
add_custom_target(${module}_lib_install_shared
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/lib
                    COMMAND install -p -m 755 -t ${CMAKE_INSTALL_PREFIX}/lib ${LIBRARY_OUTPUT_PATH}/lib${module}.so*
                    COMMENT "Installing ${module}_shared")
add_custom_target(${module}_lib_install_headers
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/include/${module}
                    COMMAND install -p -m 644 -t ${CMAKE_INSTALL_PREFIX}/include/${module} ${pub_headers}
                    COMMENT "Installing ${module} headers")
add_custom_target(${module}_lib_install)
if(HAVE_BARE_TARGET)
  add_dependencies(${module}_lib_install ${module}_lib_install_static)
  add_dependencies(${module}_lib_install_static ${module})
endif()
if(HAVE_STATIC_TARGET)
  add_dependencies(${module}_lib_install ${module}_lib_install_static)
  add_dependencies(${module}_lib_install_static ${module}_static)
endif()
if(HAVE_SHARED_TARGET)
  add_dependencies(${module}_lib_install ${module}_lib_install_shared)
  add_dependencies(${module}_lib_install_shared ${module}_shared)
endif()
if(HAVE_PUBLIC_HEADERS)
  add_dependencies(${module}_lib_install ${module}_lib_install_headers)
  add_dependencies(${module}_lib_install_headers ${module})
endif()

# check if lib_install target already exists and create it
get_target_property(LIB_INSTALL_TARGET lib_install TYPE)
if(NOT LIB_INSTALL_TARGET)
    add_custom_target(lib_install)
endif()

# if lib_install is selected, modulename_lib_install target must be created
add_dependencies(lib_install ${module}_lib_install)

###############################################################################
# Settings for target COPY_HEADER
###############################################################################
# this is copying the header to the project global include folder
foreach(header ${pub_headers})
    set(HEADER_CP_VAR ${HEADER_CP_VAR} COMMAND ${CMAKE_COMMAND} -E copy_if_different ${header}  ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include/${module}/)
endforeach(header ${pub_header})

add_custom_target(${module}_copy_header
                    COMMAND ${HEADER_CP_VAR}
                    COMMENT "Copying Headers of ${module}")

# check if copy_header target already exists and create it
get_target_property(COPY_HEADER_TARGET copy_header TYPE)
if(NOT COPY_HEADER_TARGET)
    add_custom_target(copy_header)
endif()
# if copy_header is selected, modulename_copy_header target must be created
add_dependencies(copy_header ${module}_copy_header)

add_dependencies(${module}_copy_header ${module}_create_dir)
if(FAST_BUILD)
  set (COPY_HEADER ${module}_copy_header)
else(FAST_BUILD)
  set (COPY_HEADER copy_header)
endif(FAST_BUILD)
if(HAVE_BARE_TARGET)
  add_dependencies(${module} ${COPY_HEADER})
endif()
if(HAVE_STATIC_TARGET)
  add_dependencies(${module}_static ${COPY_HEADER})
endif()
if(HAVE_SHARED_TARGET)
  add_dependencies(${module}_shared ${COPY_HEADER})
endif()


###############################################################################
# Settings for target COVERAGE
###############################################################################
set(CODECOV_HTMLOUTPUTDIR "${CMAKE_CURRENT_BINARY_DIR}/coverage")
include(${CMAKE_HOME_DIRECTORY}/CoverageSettings.cmake)
