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

#ifndef DEVELOPER_MEDIACONTROL_SERVER_NATIVESENSOR_H_
#define DEVELOPER_MEDIACONTROL_SERVER_NATIVESENSOR_H_

#include "IMediaModule.h"
#include "MediaEvent.h"
#include "thread"

#ifdef USE_SENSOR

#include "cam_device_api.hpp"

class NativeSensor : public IMediaModule {
    DECLARE_DYNAMIC_CLASS()
 public:
    NativeSensor();
    virtual ~NativeSensor();
    bool run(uint32_t nSinkIndex) override;
    const char* getClassName() override { return "NativeSensor"; }
    //void onFrameAvailable(void* data);

    bool start() override;
    bool stop() override;
    bool open() override;
    bool close() override;
    bool load(Json::Value& node) override;
    void setFormat(int nPort, MediaCap& res, int type) override;
    int jsonRequest(int port, int ctrlId, Json::Value& jsonRequest, Json::Value& jsonResponse) override;
    void onGlobalPadBufferChanged(int type, int port) override;
    void onConnectedPadBufferChanged(int port, MediaBufferQueue& queue, const char* remoteClass) override;

 private:
    CAM_DEVICE mDriver;
    void initBuffers();
    std::thread mThread;
    void loop();
    bool bRunning = false;
    std::vector<BufIdentity*> bufList;
    int frameid = 0;
    std::map<uint64_t, MediaBuffer_t *> fullQueue;
    int ispIndex;
    bool bPreloaded = false;
#ifdef ENABLE_IRQ
    int fd;
#endif
};

#endif

#endif  // DEVELOPER_MEDIACONTROL_SERVER_NATIVESENSOR_H_
