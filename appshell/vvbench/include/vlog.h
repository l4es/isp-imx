/******************************************************************************\
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef _VLOG_H_
#define _VLOG_H_

#include <stdio.h>
#include <stdlib.h>

enum {
    VVLOG_LEVEL_VERBOSE = 0,
    VVLOG_LEVEL_DEBUG,
    VVLOG_LEVEL_INFO,
    VVLOG_LEVEL_WARNING,
    VVLOG_LEVEL_ERROR,
};

extern int vvbench_log_level;

inline int get_vvbench_log_level() {
    if(vvbench_log_level != -1)
    {
        return vvbench_log_level;
    }else{
        char* szLogLevel = getenv("VVLOG_LEVEL");
        if (szLogLevel)
        {
            vvbench_log_level = atoi(szLogLevel);
        }else{
            vvbench_log_level = VVLOG_LEVEL_DEBUG;
        }
        return vvbench_log_level;
    }
}

#ifndef LOGTAG
#define LOGTAG "VVBENCH"
#endif

// c++11 workaround empty variadic macro
#define __VLOG_INT(format, ...) "[%s] " format "\033[0m%s", LOGTAG, __VA_ARGS__

#define LOGD(...)\
    if (get_vvbench_log_level() <= VVLOG_LEVEL_DEBUG) printf("\033[1;30;37mDEBUG  : " __VLOG_INT(__VA_ARGS__, "\n"))  // white

#define LOGI(...)\
    if (get_vvbench_log_level() <= VVLOG_LEVEL_INFO) printf("\033[1;30;32mINFO  : " __VLOG_INT(__VA_ARGS__, "\n"))  // green

#define LOGW(...)\
    if (get_vvbench_log_level() <= VVLOG_LEVEL_WARNING) printf("\033[1;30;33mWARN  : " __VLOG_INT(__VA_ARGS__, "\n"))  // yellow

#define LOGE(...)\
    if (get_vvbench_log_level() <= VVLOG_LEVEL_ERROR) printf("\033[1;30;31mERROR  : " __VLOG_INT(__VA_ARGS__, "\n"))  // red

#endif  // _VLOG_H_
