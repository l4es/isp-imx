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

#ifndef DEVELOPER_DEWARP_HAL_DEWARP_HARDWARE_H_
#define DEVELOPER_DEWARP_HAL_DEWARP_HARDWARE_H_

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
extern "C" {
#include <dwe_dev.h>
#include <dwe_ioctl.h>
}

#ifdef HAL_CMODEL
#include "dewarp_module.h"
#endif

class DewarpHardware {
 public:
    ~DewarpHardware() { destroy(); }
    bool create();
    void destroy();

    void setDstBufferAddress(uint32_t stride, uint32_t height, uint64_t addr);
    void setMapLutAddr(uint32_t index, uint64_t addr);
    bool start();
    bool stop();
    void startDmaRead(uint32_t stride, uint32_t height, uint64_t addr);
    void reset();

    // reg operation
    void enableBus();
    void disableBus();
    void disableIrq();

    int setParams(struct dwe_hw_info* info);
    uint32_t readIrq();
    void clearIrq();

#ifdef HAL_CMODEL
public:
    regVarStruct mRegisterVariable;
private:
    std::thread mProcessThread;
    std::atomic<bool> mRunning;
    void process();
    std::mutex                  mLock;
    std::condition_variable     mUpdatedSignal;

    int mCurrentChannel = 0;
    unsigned char* map0 = 0;
    unsigned char* map1 = 0;
    unsigned char* srcAddr = 0;
    unsigned char* dstAddr0 = 0;
    unsigned char* dstAddr1 = 0;
#endif
private:
    int fd = -1;
    int dwe_ioctl(int cmd, void* args);
   // struct dwe_ic_dev dev;
};

#endif  // DEVELOPER_DEWARP_HAL_DEWARP_HARDWARE_H_
