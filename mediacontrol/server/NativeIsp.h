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

#ifndef DEVELOPER_MEDIACONTROL_SERVER_NATIVEISP_H_
#define DEVELOPER_MEDIACONTROL_SERVER_NATIVEISP_H_

#include "IMediaModule.h"
#include "MediaEvent.h"
#include "thread"

#ifdef CAM_INTERFACE

extern "C" {
#include "CameraInterface.h"
}

class NativeIsp : public IMediaModule {
    DECLARE_DYNAMIC_CLASS()
 public:
    NativeIsp();
    virtual ~NativeIsp();
    bool run() override;
    const char* getClassName() override { return "NativeIsp"; }
    void onFrameAvailable(void* data);

    bool start() override;
    bool stop() override;
    bool open() override;
    bool close() override;
    bool load(const char* szName) override;
    void setFormat(int nPort, MediaCap& res, int type);
    bool setMode(int msgType, void* param);

 private:
    CameraInterface* mDriver;
    void updateBuffer();
};

#endif
#endif  // DEVELOPER_MEDIACONTROL_SERVER_NATIVEISP_H_
