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

#ifndef DEVELOPER_MEDIACONTROL_SERVER_FAKESOURCE_H_
#define DEVELOPER_MEDIACONTROL_SERVER_FAKESOURCE_H_

#include "IMediaModule.h"
#include <thread>

class FakeSource : public IMediaModule {
    DECLARE_DYNAMIC_CLASS()
 public:
    FakeSource();
    virtual ~FakeSource();
    bool run(uint32_t nSinkIndex) override;
    virtual const char* getClassName() override { return "FakeSource"; }
    bool start() override;
    bool stop() override;
    void setFormat(int nPort, MediaCap& res, int type) override;
    int jsonRequest(int port, int ctrlId, Json::Value& jsonRequest, Json::Value& jsonResponse) override;

 private:
    std::thread mThread;
    void loop();
    bool bRunning = false;
    int mFps = 30;
};

#endif  // DEVELOPER_MEDIACONTROL_SERVER_FAKESOURCE_H_
