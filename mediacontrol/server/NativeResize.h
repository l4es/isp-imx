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

#ifndef DEVELOPER_MEDIACONTROL_SERVER_NATIVERESIZE_H_
#define DEVELOPER_MEDIACONTROL_SERVER_NATIVERESIZE_H_

#include <thread>

#ifdef WITH_VSE
#include "IMediaModule.h"
#include <VseDriver.h>
#include <EAutoLock.h>

class NativeResize : public IMediaModule {
    DECLARE_DYNAMIC_CLASS()
 public:
    NativeResize();
    virtual ~NativeResize();
    bool run(uint32_t nSinkIndex) override;
    const char* getClassName() override { return "NativeResize"; }
    void trigerNext(uint32_t nSrcIndex) override;
    bool start() override;
    bool stop() override;
    bool open() override;
    bool close() override;
    bool load(Json::Value& node) override;
    bool needBackwardFormat(int nPort);
    void setFormat(int nPort, MediaCap& res, int type);
    void onAllChannelsBufferDone(uint64_t addrSrc);
    void onGlobalPadBufferChanged(int type, int port) override;
    void onConnectedPadBufferChanged(int port, MediaBufferQueue& queue, const char* remoteClass) override;

 private:
    int vseFd = -1;
    bool bRunning = false;
    VseDriver mDriver;
    struct vse_params params;
    std::thread mThread;
    void dequeueBufferThread();
    ESignal sig;
};

#endif
#endif  // DEVELOPER_MEDIACONTROL_SERVER_NATIVERESIZE_H_
