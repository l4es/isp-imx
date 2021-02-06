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

#ifndef _VSI_3RDLIBS_INCLUDE_LOG_H_
#define _VSI_3RDLIBS_INCLUDE_LOG_H_

#include <stdio.h>
#include <stdlib.h>

enum {
    VSI_LOG_LEVEL_VERBOSE = 0,
    VSI_LOG_LEVEL_DEBUG,
    VSI_LOG_LEVEL_INFO,
    VSI_LOG_LEVEL_WARNING,
    VSI_LOG_LEVEL_ERROR,
};

inline int vsi_log_level() {
    char* szLogLevel = getenv("VSI_LOG_LEVEL");
    if (szLogLevel)
        return atoi(szLogLevel);
    else
        return VSI_LOG_LEVEL_WARNING;
}

// c++11 workaround empty variadic macro
#define __ALOG_INT(format, ...) "[%s] " format "\033[0m%s", LOGTAG, __VA_ARGS__

#ifdef ANDROID

#include <android/log.h>
#define ISP_INFO "VSI_ISP"

#define ALOGV(...)\
    if (vsi_log_level() <= VSI_LOG_LEVEL_VERBOSE) __android_log_print(ANDROID_LOG_INFO, ISP_INFO, "\033[1;30;37mVERBOSE : " __ALOG_INT(__VA_ARGS__, "\n"))  // white
#define ALOGD(...)\
    if (vsi_log_level() <= VSI_LOG_LEVEL_DEBUG) __android_log_print(ANDROID_LOG_INFO, ISP_INFO, "\033[1;30;37mDEBUG  : " __ALOG_INT(__VA_ARGS__, "\n"))  // white

#define ALOGI(...)\
    if (vsi_log_level() <= VSI_LOG_LEVEL_INFO) __android_log_print(ANDROID_LOG_INFO, ISP_INFO, "\033[1;30;32mINFO   : " __ALOG_INT(__VA_ARGS__, "\n"))  // green

#define ALOGW(...)\
    if (vsi_log_level() <= VSI_LOG_LEVEL_WARNING) __android_log_print(ANDROID_LOG_INFO, ISP_INFO, "\033[1;30;33mWARN   : " __ALOG_INT(__VA_ARGS__, "\n"))  // yellow

#define ALOGE(...)\
    if (vsi_log_level() <= VSI_LOG_LEVEL_ERROR) __android_log_print(ANDROID_LOG_INFO, ISP_INFO, "\033[1;30;31mERROR  : " __ALOG_INT(__VA_ARGS__, "\n"))  // red

#else

#define ALOGV(...)\
    if (vsi_log_level() <= VSI_LOG_LEVEL_VERBOSE) printf("\033[1;30;37mVERBOSE : " __ALOG_INT(__VA_ARGS__, "\n"))  // white

#define ALOGD(...)\
    if (vsi_log_level() <= VSI_LOG_LEVEL_DEBUG) printf("\033[1;30;37mDEBUG  : " __ALOG_INT(__VA_ARGS__, "\n"))  // white

#define ALOGI(...)\
    if (vsi_log_level() <= VSI_LOG_LEVEL_INFO) printf("\033[1;30;32mINFO   : " __ALOG_INT(__VA_ARGS__, "\n"))  // green

#define ALOGW(...)\
    if (vsi_log_level() <= VSI_LOG_LEVEL_WARNING) printf("\033[1;30;33mWARN   : " __ALOG_INT(__VA_ARGS__, "\n"))  // yellow

#define ALOGE(...)\
    if (vsi_log_level() <= VSI_LOG_LEVEL_ERROR) printf("\033[1;30;31mERROR  : " __ALOG_INT(__VA_ARGS__, "\n"))  // red

#endif

//#define TRACE_IN  ALOGI("enter %s", __PRETTY_FUNCTION__)
//#define TRACE_OUT ALOGI("leave %s", __PRETTY_FUNCTION__)

#endif  // _VSI_3RDLIBS_INCLUDE_LOG_H_
