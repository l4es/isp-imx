include( version/VersionHelpers.cmake NO_POLICY_SCOPE )

update_configuration_version( version/VERSION.txt )
create_version_header( version/VERSION.txt version/include/version_number.h )
register_update_build_version( version/VERSION.txt version/include/version_number.h )
