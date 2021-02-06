cmake_minimum_required(VERSION 2.6)

option(JUST_VERIFY_CASE_FILES "Just verify case files base on JSON config"
    OFF
)

set(IP IP_NANO_D) # IP_DEWARP, IP_DOLPHIN, IP_NANO_D

message("IP: ${IP}")

set(INC
    include
)

file(GLOB SRC src/*.c)
file(GLOB SRC_BM src/bm/*.c)
file(GLOB SRC_BM_IP src/bm/ip/*.c)
file(GLOB SRC_FILE src/file/*.c)
file(GLOB SRC_JSON src/json/*.c)

list(APPEND SRC
    ${SRC_BM}
    ${SRC_BM_IP}
    ${SRC_CMODEL}
    ${SRC_FILE}
    ${SRC_JSON}
)

if(IP STREQUAL IP_8000L)
set(APP test.bm.8000l)
file(GLOB SRC_8000L src/bm/ip/8000l/*.c)

list(APPEND SRC
    ${SRC_8000L}
)

add_definitions(
    -DBM_LIST_FILE="/home/yy/case/8000l/list.txt"
    -DIP_DEFINE_HEADER="bm/ip/8000l/ip.h"
)
elseif(IP STREQUAL IP_NANO_D)
set(APP test.bm.nano-d)

file(GLOB SRC_NANO_D src/bm/ip/nano-d/*.c)

list(APPEND SRC
    ${SRC_NANO_D}
)

add_definitions(
    -DBM_LIST_FILE="/home/yy/case/nano-d/list.txt"
    -DIP_DEFINE_HEADER="bm/ip/nano-d/ip.h"
    # -DCMODEL_COMPARE
    -DCMODEL_SO_PATH="../lib/libnano-d.201905091503.so"
    -DDUMP_DATA_WIDTH_8
)

# set(CHIP vsISP_NANO_12MP_STEREO)
# add_subdirectory(CModel.Nxp)
elseif(IP STREQUAL IP_DEWARP)
set(APP test.bm.dewarp)

file(GLOB SRC_DEWARP src/bm/ip/dewarp/*.c)

list(APPEND SRC
    ${SRC_DEWARP}
)

add_definitions(
    -DBM_LIST_FILE="/home/yy/case/dewarp/list.txt"
    -DIP_DEFINE_HEADER="bm/ip/dewarp/ip.h"
    # -DCMODEL_COMPARE
    -DCMODEL_SO_PATH="../lib/libdewarp.so"
)

# set(CHIP module_dewarp)
# add_subdirectory(CModel.Nxp.Dewarp)
elseif(IP STREQUAL IP_DOLPHIN)
set(APP test.bm.dolphin)

file(GLOB SRC_DOLPHIN src/bm/ip/dolphin/*c)

list(APPEND SRC
    ${SRC_DOLPHIN}
)

add_definitions(
    -DBM_LIST_FILE="/home/yy/case/dolphin/list.txt"
    -DIP_DEFINE_HEADER="bm/ip/dolphin/ip.h"
    -DCMODEL_COMPARE
    -DCMODEL_SO_PATH="../lib/libdolphin.so"
)
endif(IP STREQUAL IP_8000L)

# include directories
include_directories(
    ${INC}
)

list(SORT SRC)

add_definitions(
    -D_DEFAULT_SOURCE
)

if(${JUST_VERIFY_CASE_FILES})
message("WARNING: Just verify case files.")
add_definitions(
    -DJUST_VERIFY_CASE_FILES
)
endif(${JUST_VERIFY_CASE_FILES})

add_executable(${APP}
    ${SRC}
)

target_link_libraries(
    ${APP}
    cam_engine_shared
    dl
)

install(TARGETS ${APP} DESTINATION bin)
