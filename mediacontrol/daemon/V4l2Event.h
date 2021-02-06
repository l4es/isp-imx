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

#ifndef DEVELOPER_MEDIACONTROL_DAEMON_V4L2EVENT_H_
#define DEVELOPER_MEDIACONTROL_DAEMON_V4L2EVENT_H_

#include <thread>
#include <string>
#include <map>

#include "MediaEvent.h"

class V4l2Event : public EventSource {
 public:
    V4l2Event();
    ~V4l2Event();
    void load(const char* szFileName);
 private:
    bool bRunning;
    std::thread mThread, qbufThread;
    void eventLoop();
    void qbufLoop();
};

#endif  // DEVELOPER_MEDIACONTROL_DAEMON_V4L2EVENT_H_
