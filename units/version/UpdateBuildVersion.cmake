include ( version/VersionHelpers.cmake NO_POLICY_SCOPE )

update_revision_version( ${VERSION_FILE} ${SOURCE_DIR} )
update_build_version( ${VERSION_FILE} )
create_version_header( ${VERSION_FILE} ${HEADER_FILE} )
