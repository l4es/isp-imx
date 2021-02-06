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

#ifndef DEVELOPER_MEDIACONTROL_BUFFER_MEDIABUFFERQUEUE_H_
#define DEVELOPER_MEDIACONTROL_BUFFER_MEDIABUFFERQUEUE_H_

#include <list>
#include <mutex>

#include "MediaBuffer.h"

enum status {
    BUFFER_STATUS_FREE = 0,
    BUFFER_STATUS_ACQUIRED,
    BUFFER_STATUS_DEQUEUED,
};

class MediaBufferQueue {
 public:
    MediaBufferQueue();
    ~MediaBufferQueue();
    bool create(uint32_t width, uint32_t height, uint32_t format, uint32_t number = 6);
    void  release();

    MediaBuffer* dequeueBuffer();
    MediaBuffer* acquiredBuffer();
    bool queueBuffer(MediaBuffer* pBuffer);
    bool releaseBuffer(MediaBuffer* pBuffer);

    inline int size() { return mBuffers.size(); }
    std::list<MediaBuffer* > mBuffers;

 private:
    std::mutex mAccessLock;
};

#endif  // DEVELOPER_MEDIACONTROL_BUFFER_MEDIABUFFERQUEUE_H_

