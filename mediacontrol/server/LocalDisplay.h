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

#ifndef DEVELOPER_MEDIACONTROL_SERVER_LOCALDISPLAY_H_
#define DEVELOPER_MEDIACONTROL_SERVER_LOCALDISPLAY_H_

#ifdef CV_DISPLAY
#include <opencv2/opencv.hpp>
#include <mutex>
#include <vector>
#endif

#include "IMediaModule.h"
#include <thread>

class LocalDisplay : public IMediaModule {
    DECLARE_DYNAMIC_CLASS()
 public:
    LocalDisplay();
    virtual ~LocalDisplay();
    bool run(uint32_t nSinkIndex) override;
    const char* getClassName() override { return "LocalDisplay"; }
    void setInputFormat(int id, const MediaCap& res);
};

#endif  // DEVELOPER_MEDIACONTROL_SERVER_LOCALDISPLAY_H_
