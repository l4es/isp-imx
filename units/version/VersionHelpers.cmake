# Functions that are used for automatic version contol

# update revision version number
function( update_revision_version VERSION_FILE SOURCE_DIR )
  # get revision number
  FIND_PACKAGE(Subversion)
  IF(Subversion_FOUND)
    Subversion_WC_INFO(${SOURCE_DIR} project)
  ELSE( Subversion_FOUND )
    MESSAGE( ERROR "Subversion was not found" )
  ENDIF(Subversion_FOUND)
  file( STRINGS ${VERSION_FILE} VERSION_NUMBER_STRING
	    REGEX "^([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)$" )
  string( REGEX MATCH "^([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)$" ${VERSION_NUMBER_STRING} ${VERSION_NUMBER_STRING} )
  if( ${CMAKE_MATCH_3} EQUAL ${project_WC_REVISION} )
	#do nothing
  else( ${CMAKE_MATCH_3} EQUAL ${project_WC_REVISION} )
    file( WRITE ${VERSION_FILE} "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${project_WC_REVISION}.0.0" )
  endif( ${CMAKE_MATCH_3} EQUAL ${project_WC_REVISION} )
  message( INFO " Revision number: ${project_WC_REVISION}" )
endfunction( update_revision_version )

# update configuration version number
function( update_configuration_version VERSION_FILE )
  file( STRINGS ${VERSION_FILE} VERSION_NUMBER_STRING
	    REGEX "^([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)$" )
  string( REGEX MATCH "^([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)$" ${VERSION_NUMBER_STRING} ${VERSION_NUMBER_STRING} )
  math(EXPR CONFIGURATION_NUMBER "${CMAKE_MATCH_4} + 1" )
  file( WRITE ${VERSION_FILE} "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}.${CONFIGURATION_NUMBER}.0" )
  message( INFO " Configuration number: " ${CONFIGURATION_NUMBER} )
endfunction( update_configuration_version )

# update build version number
function( update_build_version VERSION_FILE )
  file( STRINGS ${VERSION_FILE} VERSION_NUMBER_STRING
	    REGEX "^([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)$" )
  string( REGEX MATCH "^([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)$" ${VERSION_NUMBER_STRING} ${VERSION_NUMBER_STRING} )
  math(EXPR BUILD_NUMBER "${CMAKE_MATCH_5} + 1" )
  file( WRITE ${VERSION_FILE} "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}.${CMAKE_MATCH_4}.${BUILD_NUMBER}" )
  message( INFO " Build number: " ${BUILD_NUMBER} )
endfunction( update_build_version )

#register special target which uses CMake script to update build version number by update_build_version.
# Without such registration update_build_version won't be executed at every build
function( register_update_build_version VERSION_FILE HEADER_FILE )
  #execute script at the build time
  add_custom_target( UPDATE_VERSION ALL
					COMMAND ${CMAKE_COMMAND}
							-DVERSION_FILE:FILEPATH=${VERSION_FILE}
							-DSOURCE_DIR:FILEPATH=${PROJECT_SOURCE_DIR}
							-DBINARY_DIR:FILEPATH=${CMAKE_BINARY_DIR}
							-DHEADER_FILE:FILEPATH=${HEADER_FILE}
							-P version/UpdateBuildVersion.cmake
					WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} )
endfunction( register_update_build_version )

#creates C file which contains constant with version number
function( create_version_header VERSION_FILE HEADER_FILE )
  file( STRINGS ${VERSION_FILE} VERSION_NUMBER_STRING
	    REGEX "^([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)\\.([1234567890]+)$" )
  file( WRITE ${HEADER_FILE} "#ifndef SW_VERSION\n"
							"#define SW_VERSION \"${VERSION_NUMBER_STRING}\"\n"
							"#endif" )
endfunction( create_version_header )
