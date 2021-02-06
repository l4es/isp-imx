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

#ifndef DEVELOPER_MEDIACONTROL_SERVER_NATIVEDW200_H_
#define DEVELOPER_MEDIACONTROL_SERVER_NATIVEDW200_H_

#include "IMediaModule.h"

#ifdef WITH_DW200
#include "DW200Driver.h"

class NativeDW200 : public IMediaModule {
    DECLARE_DYNAMIC_CLASS()
 public:
    NativeDW200();
    virtual ~NativeDW200();
    bool run(uint32_t nSinkeIndex) override;
    const char* getClassName() override { return "NativeDW200"; }
    void trigerNext(uint32_t nSrcIndex) override;

    bool start() override;
    bool stop() override;
    bool open() override;
    bool close() override;
    bool load(Json::Value& node) override;
    void setFormat(int nPort, MediaCap& res, int type) override;
    bool needBackwardFormat(int nPort);
    void onAllFramesDone(uint64_t src0, uint64_t src1);
    int jsonRequest(int port, int ctrlId, Json::Value& jsonRequest, Json::Value& jsonResponse) override;
    void onGlobalPadBufferChanged(int type, int port) override;
    void onConnectedPadBufferChanged(int port, MediaBufferQueue& queue, const char* remoteClass) override;

 private:
    int dewarpFd = -1;
    bool bRunning = false;
    DW200Driver mDriver;
    struct dw200_parameters params;
    struct dewarp_distortion_map dmap[2];
    int mFrameNumber = 0;
};

#endif
#endif  // DEVELOPER_MEDIACONTROL_SERVER_NATIVEDW200_H_
