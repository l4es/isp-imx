#
# s.stuerke 2010
# t.metzler 2010
#

set (CMAKE_INSTALL_PREFIX ${INSTALL_ROOT})

if (ANDROID)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIC -Wno-parentheses-equality -Wno-newline-eof -Wno-sometimes-uninitialized -Wno-enum-conversion -Wno-deprecated-declarations -Wno-unused-private-field -Wno-format-pedantic")
else ()
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIC -O2")
endif ()
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O2 -DDEBUG -g2 -ggdb")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-strict-aliasing -O3")

if (ANDROID)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -std=c++11 -Wno-parentheses-equality -Wno-newline-eof -Wno-sometimes-uninitialized -Wno-enum-conversion -Wno-deprecated-declarations -Wno-unused-private-field")
else ()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -std=c++11")
endif ()
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2 -DDEBUG -g2 -ggdb -fno-strict-aliasing")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-strict-aliasing -O2")

add_definitions(-pedantic) # to allow for removal via remove_definitions(-pedantic) by modules this isn't suitable for

ENABLE_TESTING()

OPTION( ENABLE_CODECOVERAGE "Enable code coverage testing support" )

if (CMAKE_BUILD_TYPE STREQUAL COV)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set (CMAKE_LDFLAGS "${CMAKE_C_FLAGS} -fprofile-arcs")
    set (ENABLE_CODECOVERAGE 1)
endif (CMAKE_BUILD_TYPE STREQUAL COV)

if (CMAKE_BUILD_TYPE STREQUAL PROF)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -DDEBUG -pg")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG -pg")
    set (CMAKE_LDFLAGS "${CMAKE_LDFLAGS} -pg")
endif (CMAKE_BUILD_TYPE STREQUAL PROF)

include_directories(${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include)
link_directories(${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib)
set (LIBRARY_OUTPUT_PATH ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/lib)
set (EXECUTABLE_OUTPUT_PATH ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/bin)
