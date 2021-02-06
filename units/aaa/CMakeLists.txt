cmake_minimum_required( VERSION 2.6 )

set ( module aaa )

# add sub modules
if(GENERATE_3A_LIB)

add_subdirectory( aec )
add_subdirectory( awb )
add_subdirectory( af )
add_subdirectory( adpf )
add_subdirectory( adpcc )
add_subdirectory( a2dnr )
add_subdirectory( a3dnr )
add_subdirectory( awdr3 )
add_subdirectory( ahdr )
add_subdirectory( aee )
add_subdirectory( aflt )
add_subdirectory( avs )

else(GENERATE_3A_LIB)

add_subdirectory( proprietories )

endif(GENERATE_3A_LIB)

