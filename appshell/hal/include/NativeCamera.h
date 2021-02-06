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

#ifndef _CAM_SHELL_NATIVECAMERA_H_
#define _CAM_SHELL_NATIVECAMERA_H_

#include "VirtualCamera.h"

#ifdef APPMODE_NATIVE

class NativeCamera : public VirtualCamera {
public:
    ~NativeCamera();
    int open(int id);
    void close(int id);
    int connectCamera();
    int disconnectCamera();
    int start();
    int stop();
    int ioctl(int cmdID, Json::Value& jsonRequest, Json::Value& jsonResponse);

private:
    CAM_DEVICE mDevice;
    void loop();
    std::thread mThread;
    bool bRunning = false;
    int fd;
};

#endif  // _CAM_SHELL_NATIVECAMERA_H_

#endif
