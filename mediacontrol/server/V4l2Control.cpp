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

#include "V4l2Control.h"

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include "MediaBufferQueue.h"
#include "BufferManager.h"
#include "viv_video_kevent.h"
#include "V4l2File.h"

#include "log.h"

#define LOGTAG "V4l2Control"

V4l2Control::V4l2Control() {
    // max support output channels;
    mSink.resize(5, MediaPad(PAD_TYPE_IMAGE));
}

V4l2Control::~V4l2Control() {
}

bool V4l2Control::run(uint32_t nSinkIndex) {
    int fd = V4l2File::inst()->open();

    if (!getSinkState(nSinkIndex))
        return false;
    MediaAddrBuffer* pBuffer = (MediaAddrBuffer*)mSink[nSinkIndex].image;
    // ALOGI("get new buffer at pad %d 0x%08x", i, pBuffer->baseAddress);
    if (pBuffer) {
        void* file = V4l2BufferManager::inst()->find(pBuffer->baseAddress);
        if (file) {
            struct v4l2_user_buffer buf = {pBuffer->baseAddress, 0, file};
            ioctl(fd, VIV_VIDIOC_BUFDONE, &buf);
        }
    }

    setSinkState(nSinkIndex, false);
    return true;
}

IMPLEMENT_DYNAMIC_CLASS(V4l2Control)
