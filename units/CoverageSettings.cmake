
if ( ENABLE_CODECOVERAGE )

    if ( NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG" )
        message( WARNING "Code coverage results with an optimised (non-Debug) build may be misleading" )
    endif ( NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG" )

    if ( NOT DEFINED CODECOV_OUTPUTFILE )
        set( CODECOV_OUTPUTFILE cmake_coverage.output )
    endif ( NOT DEFINED CODECOV_OUTPUTFILE )

    if ( NOT DEFINED CODECOV_HTMLOUTPUTDIR )
        set( CODECOV_HTMLOUTPUTDIR coverage_results )
    endif ( NOT DEFINED CODECOV_HTMLOUTPUTDIR )

    if ( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCXX )
       find_program( CODECOV_GCOV gcov )
       find_program( CODECOV_LCOV lcov )
       find_program( CODECOV_GENHTML genhtml )
       add_definitions( -fprofile-arcs -ftest-coverage )
       link_libraries( gcov )
       set( CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} --coverage )

        add_custom_target( ${module}_coverage_init ${CODECOV_LCOV} --base-directory .
                --directory ${CMAKE_BINARY_DIR} --output-file ${CODECOV_OUTPUTFILE} --capture
                --initial )
        get_target_property(COV_TARGET_INIT coverage_init TYPE)
        if(NOT COV_TARGET_INIT)
            add_custom_target(coverage_init)
        endif()
        add_dependencies(coverage_init ${module}_coverage_init)

        add_custom_target( ${module}_coverage ${CODECOV_LCOV} --base-directory .
                --directory ${CMAKE_BINARY_DIR} --output-file ${CODECOV_OUTPUTFILE} --capture
                COMMAND genhtml -o ${CODECOV_HTMLOUTPUTDIR} ${CODECOV_OUTPUTFILE} )
        get_target_property(COV_TARGET coverage TYPE)
        if(NOT COV_TARGET)
            add_custom_target(coverage)
        endif()
        add_dependencies(coverage ${module}_coverage)


       endif ( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCXX )

endif (ENABLE_CODECOVERAGE )

