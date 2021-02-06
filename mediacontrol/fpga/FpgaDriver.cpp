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

#include "FpgaDriver.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>
#include <memory.h>

#include "V4l2File.h"
#include <log.h>

#define LOGTAG "FPGADriver"

FpgaDriver* FpgaDriver::mInst = NULL;
FpgaDriver* FpgaDriver::inst() {
    if (mInst == NULL) {
        mInst = new FpgaDriver();
        mInst->open();
    }
    return mInst;
}

FpgaDriver::FpgaDriver() {
}

FpgaDriver::~FpgaDriver() {
    close();
}

bool FpgaDriver::open() {
    subdev = ::open("/dev/vivisp0", O_RDWR);
    if (subdev < 0) {
        ALOGE("can't open isp dev!!\n");
        subdev = 0;
        //exit(1);
    }

    ALOGI("reservedMemBase: 0x%08X, reservedMemSize: 0x%08X\n", reservedMemBase, reservedMemSize);
    mData = (uint8_t*)mmap(NULL, reservedMemSize, PROT_READ | PROT_WRITE, MAP_SHARED, subdev, reservedMemBase);
    if (!mData || (mData == MAP_FAILED)) {
        ALOGE("can't map reserved memmory !!\n");
        mData = NULL;
        ::close(subdev);
        subdev = 0;
        return false;
    }
    return true;
}

void FpgaDriver::close() {
    if (mData)
        munmap(mData, reservedMemSize);
    if (subdev != -1)
        ::close(subdev);
    subdev = 0;
}

void FpgaDriver::getInfo(uint32_t& addr, uint32_t& size) {
    addr = reservedMemBase;
    size = reservedMemSize;
}
