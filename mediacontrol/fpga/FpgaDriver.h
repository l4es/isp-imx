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

#ifndef DEVELOPER_DEWARP_HAL_FPGADRIVER_H_
#define DEVELOPER_DEWARP_HAL_FPGADRIVER_H_

#include <stdint.h>
#include <unistd.h>

class FpgaDriver {
 public:
    FpgaDriver();
    virtual ~FpgaDriver();

    void getInfo(uint32_t& addr, uint32_t& size);

    static FpgaDriver* inst();
    inline bool isOpened() { return subdev > 0; }
    inline unsigned char* getBuffer() { return mData; }

    int subdev = -1;
 protected:
    virtual bool open();
    virtual void close();

    unsigned char* mData = NULL;
    uint32_t reservedMemBase = 0x10000000;
    uint32_t reservedMemSize = 0x10000000;
    static FpgaDriver* mInst;
};

#endif  // DEVELOPER_DEWARP_HAL_FPGADRIVER_H_

