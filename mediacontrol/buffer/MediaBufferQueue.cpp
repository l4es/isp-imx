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

#include "MediaBufferQueue.h"
#include <memory.h>

#include "log.h"

#define LOGTAG "MediaBufferQueue"

MediaBufferQueue::MediaBufferQueue() {
}

MediaBufferQueue::~MediaBufferQueue() {
    release();
}

bool MediaBufferQueue::create(uint32_t width, uint32_t height, uint32_t format, uint32_t number) {
    if (width <= 0 || height <= 0 || format < 0 || number <= 0)
        return false;
    ALOGI("create buffer queue %d %d %d %d", width, height, format, number);

    for (uint32_t i = 0; i < number; i++) {
        MediaBuffer* pBuffer = new MediaAddrBuffer();
        pBuffer->create(width, height, format);
        pBuffer->mStatus = BUFFER_STATUS_FREE;
        mBuffers.push_back(pBuffer);
    }
    return true;
}

void MediaBufferQueue::release() {
    std::lock_guard<std::mutex> lock(mAccessLock);
    for (auto& item : mBuffers) {
        delete item;
    }
    mBuffers.clear();
}

MediaBuffer* MediaBufferQueue::dequeueBuffer() {
    std::lock_guard<std::mutex> lock(mAccessLock);
    for (auto& item : mBuffers) {
        if (item->mStatus == BUFFER_STATUS_FREE) {
            item->mStatus = BUFFER_STATUS_DEQUEUED;
            return item;
        }
    }
    return NULL;
}

MediaBuffer* MediaBufferQueue::acquiredBuffer() {
    std::lock_guard<std::mutex> lock(mAccessLock);
    for (auto& item : mBuffers) {
        if (item->mStatus == BUFFER_STATUS_ACQUIRED) {
            item->mStatus = BUFFER_STATUS_DEQUEUED;
            return item;
        }
    }
    return NULL;
}

bool MediaBufferQueue::queueBuffer(MediaBuffer* pBuffer) {
    std::lock_guard<std::mutex> lock(mAccessLock);
    if (!pBuffer)
        return false;
    if (pBuffer->mStatus != BUFFER_STATUS_DEQUEUED)
        return false;
    for (auto& item : mBuffers) {
        if (item == pBuffer) {
            item->mStatus = BUFFER_STATUS_ACQUIRED;
            mBuffers.remove(item);
            mBuffers.push_back(item);
            return true;
        }
    }
    return false;
}

bool MediaBufferQueue::releaseBuffer(MediaBuffer* pBuffer) {
    std::lock_guard<std::mutex> lock(mAccessLock);
    if (!pBuffer)
        return false;
    if (pBuffer->mStatus != BUFFER_STATUS_DEQUEUED)
        return false;

    for (auto& item : mBuffers) {
        if (item == pBuffer) {
            item->mStatus = BUFFER_STATUS_FREE;
            mBuffers.remove(item);
            mBuffers.push_back(item);
            return true;
        }
    }
    return false;
}
