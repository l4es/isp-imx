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
    DOM_LEVEL_VERBOSE = 0,
    DOM_LEVEL_DEBUG,
    DOM_LEVEL_INFO,
    DOM_LEVEL_WARNING,
    DOM_LEVEL_ERROR,
};

extern int dom_log_level;

inline int get_dom_log_level() {
    if(dom_log_level != -1)
    {
        return dom_log_level;
    }else{
        char* szLogLevel = getenv("DOM_LEVEL");
        if (szLogLevel)
        {
            dom_log_level = atoi(szLogLevel);
        }else{
            dom_log_level = DOM_LEVEL_DEBUG;
        }
        return dom_log_level;
    }
}

#ifndef LOGTAG
#define LOGTAG "APPDOM"
#endif

// c++11 workaround empty variadic macro
#define __VLOG_INT(format, ...) "[%s] " format "\033[0m%s", LOGTAG, __VA_ARGS__

#define LOGD(...)\
    if (get_dom_log_level() <= DOM_LEVEL_DEBUG) printf("\033[1;30;37mDEBUG  : " __VLOG_INT(__VA_ARGS__, "\n"))  // white

#define LOGI(...)\
    if (get_dom_log_level() <= DOM_LEVEL_INFO) printf("\033[1;30;32mINFO  : " __VLOG_INT(__VA_ARGS__, "\n"))  // green

#define LOGW(...)\
    if (get_dom_log_level() <= DOM_LEVEL_WARNING) printf("\033[1;30;33mWARN  : " __VLOG_INT(__VA_ARGS__, "\n"))  // yellow

#define LOGE(...)\
    if (get_dom_log_level() <= DOM_LEVEL_ERROR) printf("\033[1;30;31mERROR  : " __VLOG_INT(__VA_ARGS__, "\n"))  // red

#endif  // _VLOG_H_
