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

#include "DewarpHardware.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <unistd.h>

#include <log.h>
#include "dewarpdev.h"
#include <MediaBuffer.h>
#include <dwe_regs.h>

#define LOGTAG "DEWARP_HW"

#ifdef HAL_CMODEL
#include "register_interface.h"
#include "dewarp_module.h"

static DewarpHardware* dweDriverInst;

static bool dweWriteReg(uint32_t offset, uint32_t data) {
    writeRegisterToCmodel(&dweDriverInst->mRegisterVariable, offset, data);
    ALOGI("%s 0x%08x   0x%08x", __func__, offset, data);
    return true;
}

static bool dweReadReg(uint32_t offset, uint32_t* data) {
    readRegisterFromCmodel(&dweDriverInst->mRegisterVariable, offset, data);
    return true;
}

bool DewarpHardware::create() {
    mRegisterVariable.pMGRV = new mGlobalRegisterVariable;
    dewarp_register_initional(&mRegisterVariable);
    mRegisterVariable.dump_data_flag = 0;
    dweDriverInst = this;
    dwe_set_func(dweReadReg, dweWriteReg);
    return true;
}

void DewarpHardware::destroy() {
    stop();
    if (mRegisterVariable.pMGRV) {
        delete (mRegisterVariable.pMGRV);
        mRegisterVariable.pMGRV = NULL;
    }
}

void DewarpHardware::process() {
    ALOGI("enter %s", __func__);
    while (mRunning) {
        ALOGD("DewarpHardware start waiting frame\n");
        std::unique_lock<std::mutex> lock(mLock);
        mUpdatedSignal.wait(lock);
        ALOGI("DewarpHardware get new  frame %p %p %p %p %p", srcAddr, map0, map1, dstAddr0, dstAddr1);
        DewarpModuleProcess(srcAddr, 0, mCurrentChannel == 0 ? map0 : map1, map1,
                            dstAddr0, dstAddr1, &mRegisterVariable);
        mRegisterVariable.frame_num++;
    }
}

bool DewarpHardware::start() {
    ALOGI("enter %s", __func__);
    mRunning = true;
    mProcessThread = std::thread([this]() { process(); });
    return true;
}

bool DewarpHardware::stop() {
    ALOGI("enter %s", __func__);
    mRunning = false;
    if (mProcessThread.joinable()) {
        mProcessThread.detach();
    }
    return true;
}

void DewarpHardware::startDmaRead(uint32_t stride, uint32_t height, uint64_t addr) {
    ALOGI("enter %s", __func__);
    srcAddr = (unsigned char*)addr;
    // ALOGI("DewarpHardware trigerStart\n");
    std::unique_lock<std::mutex> lock(mLock);
    mUpdatedSignal.notify_one();
}

void DewarpHardware::reset() {
    dwe_ioctl(DWEIOC_RESET, 0);
}

void DewarpHardware::setDstBufferAddress(uint32_t stride, uint32_t height, uint64_t addr) {
    ALOGI("enter %s", __func__);
    dstAddr0 = (unsigned char*)addr;
    dstAddr1 = (unsigned char*)addr;
}

void DewarpHardware::setMapLutAddr(uint32_t index, uint64_t addr) {
    ALOGI("enter %s", __func__);
    if (index == 0) map0 = (unsigned char*)addr;
    else map1 = (unsigned char*)addr;
}


void DewarpHardware::clearIrq() {
    ALOGI("enter %s", __func__);
    // writeReg(INTERRUPT_STATUS, INT_CLR);
}


int DewarpHardware::dwe_ioctl(int cmd, void* args) {
    return dwe_priv_ioctl(&dev, cmd, args);
}

#else
#ifdef USE_V4L2
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <assert.h>
#include <memory.h>
#include <linux/videodev2.h>

bool DewarpHardware::create() {
    ALOGD("enter %s", __func__);
    char szDeviceName[64] = { 0 };
    struct v4l2_capability cap;
    for (int index = 0; index < 20; index++) {
        sprintf(szDeviceName, "/dev/v4l-subdev%d", index);
        fd = open(szDeviceName, O_RDWR | O_NONBLOCK);
        if (fd < 0) {
            continue;
        }
        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) != 0) {
            continue;
        }

        ALOGW("dwe type %s %d ", szDeviceName,  index);
        if (strcmp((char*)cap.driver, "viv_dewarp100") == 0) {
            // is dwe
            break;
        }
        close(fd);
        fd = -1;
    }

    if (fd < 0) {
        ALOGE("can't find dwe subdev!");
        exit(1);
    }

    reset();
    return true;
}

void DewarpHardware::destroy() {
    ALOGD("enter %s", __func__);
    close(fd);
    fd = -1;
}

int DewarpHardware::dwe_ioctl(int cmd, void* args) {
    return ioctl(fd, cmd, args);
}

#else
#include <FpgaDriver.h>

bool DewarpHardware::create() {
    ALOGD("enter %s", __func__);
    if (!FpgaDriver::inst()->isOpened()) {
        return false;
    }
    dwe_set_func(FpgaDriver::readBar, FpgaDriver::writeBar);
    reset();
    return true;
}

void DewarpHardware::destroy() {
    ALOGD("enter %s", __func__);
}

int DewarpHardware::dwe_ioctl(int cmd, void* args) {
    return dwe_priv_ioctl(&dev, cmd, args);
}

#endif

bool DewarpHardware::start() {
    dwe_ioctl(DWEIOC_START, 0);
    return true;
}

bool DewarpHardware::stop() {
    dwe_ioctl(DWEIOC_STOP, 0);
    return true;
}

void DewarpHardware::clearIrq() {
    dwe_ioctl(DWEIOC_CLEAR_IRQ, 0);
}

void DewarpHardware::startDmaRead(uint32_t stride, uint32_t height, uint64_t addr) {
    dwe_ioctl(DWEIOC_START_DMA_READ, &addr);
}

void DewarpHardware::reset() {
    dwe_ioctl(DWEIOC_RESET, 0);
}

void DewarpHardware::setDstBufferAddress(uint32_t stride, uint32_t height, uint64_t addr) {
    dwe_ioctl(DWEIOC_SET_BUFFER, &addr);
}

void DewarpHardware::setMapLutAddr(uint32_t index, uint64_t addr) {
    struct lut_info info = {index, addr};
    dwe_ioctl(DWEIOC_SET_LUT, &info);
}

#endif


int DewarpHardware::setParams(struct dwe_hw_info* info) {
    ALOGI("enter %s", __func__);
    dwe_ioctl(DWEIOC_S_PARAMS, info);
    return 0;
}

uint32_t DewarpHardware::readIrq() {
    uint32_t irqStatus;
    dwe_ioctl(DWEIOC_READ_IRQ, &irqStatus);
    return irqStatus;
}

void DewarpHardware::enableBus() {
    dwe_ioctl(DWEIOC_ENABLE_BUS, 0);
}

void DewarpHardware::disableBus() {
    dwe_ioctl(DWEIOC_DISABLE_BUS, 0);
}

void DewarpHardware::disableIrq() {
    dwe_ioctl(DWEIOC_DISABLE_IRQ, 0);
}
