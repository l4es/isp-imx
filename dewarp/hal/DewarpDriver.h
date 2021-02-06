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

#ifndef DEVELOPER_DEWARP_INCLUDE_DEWARPDRIVER_H_
#define DEVELOPER_DEWARP_INCLUDE_DEWARPDRIVER_H_

#include <atomic>
#include <thread>
#include <functional>
#include <condition_variable>
#include <list>
#include <vector>
#include <mutex>

#include "dewarpdev.h"
#include "dwe_dev.h"
#include "DewarpHardware.h"
#include "MediaBuffer.h"
#include "EAutoLock.h"

/******************************************************************************
* manage buffer ring/interrupt/reg operation
* so that cmodel and fpga have the same interface.
******************************************************************************/
class DewarpDriver {
 public:
    DewarpDriver() {}
    ~DewarpDriver() {}

    int open();
    void close(int fd);

    bool start();
    bool stop();
    bool setParams(dewarp_parameters* params);
    bool queryCaps(dewarp_capability* caps);
    bool setDistortionMap(dewarp_distortion_map* pMap);  // must have two maps.
    void registerBufferCallback(std::function<void(uint64_t addrSrc, uint64_t addrDst, int port)> callback = nullptr) {
        mBufferCallback = callback;
    }
    void removeBufferCallback() { mBufferCallback = nullptr; }
    inline void getDstSize(int& dw, int& dh) { dw = info.dst_w; dh = info.dst_h; }
    void setFlip(int port, bool hflip, bool vflip);

 private:
    void setMap(dewarp_distortion_map& dmap, int index);
    void mainStream();
    void irqEvent(int irqStatus);
    void processRequest();
    void clear();

 private:
    DewarpHardware* mHardware = NULL;
    uint64_t dweMapAddr[2] = {0};

    std::thread mStreamThread;
    std::thread mRequestThread;
    std::atomic<int> mRunMode;
    EMutex frameDoneMutex;
    ESignal frameDoneSignal;

    int mInputBufferCount  = 0;
    int mOutputBufferCount = 0;
//    int mStatus = INPUT_FREE; // error: private field 'mStatus' is not used [-Werror,-Wunused-private-field]
    enum RunModes {
        STOPPED = 0,
        RUN = 1,
        STOPPING = 2,
    };

    enum HardwareInputStatus {
        INPUT_FREE = 0,
        INPUT_BUSY = 1,
    };

 private:
    struct dewarp_parameters mParams;
    struct dwe_hw_info info;
    uint32_t dewarp_type = 0;

    unsigned int* mDewarpMap[2] = { 0 };  // two maps for dual ISP.
    std::function<void(uint64_t addrSrc, uint64_t addrDst, int port)> mBufferCallback = nullptr;
};

#endif  // DEVELOPER_DEWARP_INCLUDE_DEWARPDRIVER_H_
