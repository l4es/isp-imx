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

#ifndef _CAM_SHELL_V4L2CAMERA_H_
#define _CAM_SHELL_V4L2CAMERA_H_

#include "VirtualCamera.h"

#ifdef SUPPORT_PASS_JSON
#include "viv_video_kevent.h"
#include "EAutoLock.h"
#endif

#ifdef APPMODE_V4L2

class V4l2Camera : public VirtualCamera {
public:
    ~V4l2Camera();
    int open(int id);
    void close(int id);
    int connectCamera();
    int disconnectCamera();
    int start();
    int stop();
    int ioctl(int cmdID, Json::Value& jsonRequest, Json::Value& jsonResponse);

private:
    void loop();
    std::thread mThread;
    bool bRunning = false;
    int fd;
    std::vector<void*> mappedBuffers;

#ifdef SUPPORT_PASS_JSON
    /*
       init physical address to pass json,  mediacontrol will map once at first time.
       user can replace to socket or share memory.
       video0 also provide alloc/free/mmap function. 
       this "alloc" means alloc memory from reserved memory by CMA.
       Then we can manage all physical address in kernel.
       fixed size : 16K
    */
    uint64_t request_pa;
    uint64_t response_pa;
    char* request_va;
    char* response_va;
    EMutex mMutex;
#endif
};

#endif  // _CAM_SHELL_V4L2CAMERA_H_

#endif 
