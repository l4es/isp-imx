###############################################################################
# Settings for target RUN
###############################################################################
if( UNIX )
  set( SYNC COMMAND sync )
  set( COMMENT "sync + ${application}" )
else( UNIX )
  set( SYNC )
  set( COMMENT "${application}")
endif( UNIX )
add_custom_target(${application}_run
                  ${SYNC}
                  COMMAND ${CMAKE_COMMAND} -E chdir ${EXECUTABLE_OUTPUT_PATH} ./${application}
                  COMMENT "Executing ${COMMENT}"
                  )


###############################################################################
# Settings for target DBG
###############################################################################
if( UNIX )
  set( SYNC COMMAND sync )
  set( DBGCMD COMMAND ${CMAKE_COMMAND} -E chdir ${EXECUTABLE_OUTPUT_PATH} ddd ./${application} )
  set( COMMENT "sync + ddd ${application}" )
else( UNIX )
  set( SYNC )
  set( DBGCMD COMMAND ${CMAKE_COMMAND} -E chdir ${EXECUTABLE_OUTPUT_PATH} <fixme_debugger> ./${application} )
  set( COMMENT "<fixme_debugger> ${application}")
endif( UNIX )
add_custom_target(${application}_dbg
                  ${SYNC}
                  ${DBGCMD}
                  COMMENT "Executing ${COMMENT}"
                  )


###############################################################################
# Settings for target LINT
###############################################################################
include( ${UNITS_TOP_DIRECTORY}/lint.cmake)
if(COMMAND add_pc_lint)
  set(PC_LINT_USER_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/pc_lint private.lnt)
  add_pc_lint(${application} STATIC ${appsources})
endif(COMMAND add_pc_lint)


###############################################################################
# Settings for target DOXYGEN
###############################################################################
set(DOXYGEN_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
set(DOXYGEN_COMP_SHORT "simg") #lower case!!!
set(DOXYGEN_HTML_OUTPUTDIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen/html")
set(DOXYGEN_IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/images")
set(DOXYGEN_LATEX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/latex")
set(DOXYGEN_LATEX_OUTPUTDIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen/latex")
set(DOXYGEN_LATEX_MODULE_NAME "${application}")
set(DOXYGEN_LATEX_MODULE_TYPE_DESC "Application")
set(DOXYGEN_LATEX_MODULE_STATE_DESC "Preliminary")
string(TOUPPER ${DOXYGEN_LATEX_MODULE_NAME} DOXYGEN_LATEX_MODULE_NAME)
string(REGEX REPLACE "_" "\\\\_" DOXYGEN_LATEX_MODULE_NAME ${DOXYGEN_LATEX_MODULE_NAME})

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/doxyfile.cmake")
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    # create doxyfile using cmake variables
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/doxyfile.cmake ${DOXYGEN_CONFIG_DIR}/doxyfile)
    # create ${DOXYGEN_COMP_SHORT}_header.tex using cmake variables
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/latex/${DOXYGEN_COMP_SHORT}_header.tex.in")
    	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/latex/${DOXYGEN_COMP_SHORT}_header.tex.in ${DOXYGEN_CONFIG_DIR}/${DOXYGEN_COMP_SHORT}_header.tex)
    	set (HAVE_LATEX_CONFIG 1)
    endif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/latex/${DOXYGEN_COMP_SHORT}_header.tex.in")

    # create targets
    if (HAVE_LATEX_CONFIG)
      # create both HTML & TeX/PDF documentation
      add_custom_target(${application}_doc
                        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DOXYGEN_HTML_OUTPUTDIR}
                        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DOXYGEN_LATEX_OUTPUTDIR}
                        COMMAND ${CMAKE_COMMAND} -E remove -f ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/doc/${application}_ref.pdf
                        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_DIR}/doxyfile > ${DOXYGEN_CONFIG_DIR}/doxygen.log
                        COMMAND install -p -m 644 ${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/images/* ${DOXYGEN_HTML_OUTPUTDIR}
                        COMMAND install -p -m 644 ${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/latex/*.png ${DOXYGEN_LATEX_OUTPUTDIR}
                        COMMAND install -p -m 644 ${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/latex/*.sty ${DOXYGEN_LATEX_OUTPUTDIR}
                        COMMAND make -C ${DOXYGEN_LATEX_OUTPUTDIR} > ${DOXYGEN_CONFIG_DIR}/doxygen_latex.log
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/doc
                        COMMAND install -p -m 644 ${DOXYGEN_LATEX_OUTPUTDIR}/refman.pdf ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/doc/${application}_ref.pdf
                        COMMENT "Generating ${application} documentation"
                        )
    else(HAVE_LATEX_CONFIG)
      # create HTML documentation only
      add_custom_target(${application}_doc
                        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DOXYGEN_HTML_OUTPUTDIR}
                        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_DIR}/doxyfile
                        COMMAND install -p -m 644 ${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/images/* ${DOXYGEN_HTML_OUTPUTDIR}
                        COMMENT "Generating ${application} documentation"
                        )
    endif(HAVE_LATEX_CONFIG)

    # check if doc target already exists and create it
    get_target_property(DOC_TARGET doc TYPE)
    if(NOT DOC_TARGET)
        add_custom_target(doc)
    endif()

    # finally create dependencies
    add_dependencies(doc ${application}_doc)
  endif (DOXYGEN_FOUND)
endif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/codeanalysis/doxygen/doxyfile.cmake")

###############################################################################
# Settings for target COVERAGE
###############################################################################
set(CODECOV_HTMLOUTPUTDIR "${CMAKE_CURRENT_BINARY_DIR}/coverage")
include(${UNITS_TOP_DIRECTORY}/CoverageSettings_app.cmake)
