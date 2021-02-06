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

#ifndef DEVELOPER_MEDIACONTROL_SERVER_NATIVEDEWARP_H_
#define DEVELOPER_MEDIACONTROL_SERVER_NATIVEDEWARP_H_

#include "IMediaModule.h"

#ifdef WITH_DWE
#include "DewarpDriver.h"

class NativeDewarp : public IMediaModule {
    DECLARE_DYNAMIC_CLASS()
 public:
    NativeDewarp();
    virtual ~NativeDewarp();
    bool run(uint32_t nSinkIndex) override;
    const char* getClassName() override { return "NativeDewarp"; }
    void trigerNext(uint32_t nSrcIndex) override;

    bool start() override;
    bool stop() override;
    bool open() override;
    bool close() override;
    bool load(Json::Value& node) override;
    void setFormat(int nPort, MediaCap& res, int type) override;
    void onFrameAvailable(uint64_t addrSrc, uint64_t addrDst, int port);
    int jsonRequest(int port, int ctrlId, Json::Value& jsonRequest, Json::Value& jsonResponse) override;
    void onGlobalPadBufferChanged(int type, int port) override;
    void onConnectedPadBufferChanged(int port, MediaBufferQueue& queue, const char* remoteClass) override;

 private:
    int dewarpFd = -1;
    bool bRunning = false;
    DewarpDriver mDriver;
    struct dewarp_parameters params;
    struct dewarp_distortion_map dmap[2];
    int mFrameNumber = 0;
#ifdef ENABLE_IRQ
    int fd;
#endif
};

#endif
#endif  // DEVELOPER_MEDIACONTROL_SERVER_NATIVEDEWARP_H_
