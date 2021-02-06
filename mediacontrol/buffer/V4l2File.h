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

#ifndef DEVELOPER_MEDIACONTROL_SERVER_V4L2FILE_H_
#define DEVELOPER_MEDIACONTROL_SERVER_V4L2FILE_H_
#include <stdint.h>

class V4l2File {
 public:
    V4l2File() {}
    ~V4l2File();

    int open();
    uint64_t alloc(uint64_t size);
    void free(uint64_t addr);
    void* mmap(uint64_t addr, uint64_t size);
    void munmap(unsigned char* addr, uint64_t size);
    static V4l2File* inst();

 private:
    int fd = -1;
    static V4l2File* mInst;
};

#endif  // DEVELOPER_MEDIACONTROL_SERVER_V4L2FILE_H_

