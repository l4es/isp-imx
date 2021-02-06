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

#include "NativeResize.h"

#include <assert.h>
#include <memory.h>

#include <fstream>
#include <map>
#include <string>

#ifdef WITH_VSE
#include "vse_dev.h"
#include "VseDriver.h"
#include "log.h"

#define LOGTAG "NATIVE_RESIZE"

NativeResize::NativeResize() {
    mSink.resize(2, MediaPad(PAD_TYPE_IMAGE));
    mSrc.resize(4, MediaPad(PAD_TYPE_IMAGE));
}

NativeResize::~NativeResize() {
    stop();
}

bool NativeResize::load(Json::Value& node) {
    memset(&params, 0, sizeof(params));

    auto nameNode = node.get("format", "YUV420SP");
    const char* szNode = nameNode.asCString();
    auto iter = mediaFormatStringTable.find(szNode);
    if (iter == mediaFormatStringTable.end()) {
        ALOGE("unsupport input format %s", szNode);
        return false;
    }
    params.in_format = iter->second;
    params.src_w = node["width"].asUInt();
    params.src_h = node["height"].asUInt();
    params.input_select = node["channel"].asUInt();
    return true;
}

bool NativeResize::start() {
    mRefCount++;
    if (bRunning) return false;
    mDriver.setParams(&params);
    if (!mDriver.start()) {
        ALOGE("failed to start vse driver");
        return false;
    }
    bRunning = true;
    mThread = std::thread([this]() { dequeueBufferThread(); });
    return true;
}

bool NativeResize::stop() {
    if (--mRefCount == 0) {
        if (!bRunning) return false;
        bRunning = false;
        mDriver.stop();
        sig.post();
        mThread.join();
        GCLEAR(this);
        GCLEAR(&mDriver);
    }
    return true;
}

bool NativeResize::open() {
    vseFd = mDriver.open();
    mDriver.registerBufferCallback(std::bind(&NativeResize::onAllChannelsBufferDone,
                                    this,
                                    std::placeholders::_1));
    return vseFd == 0;
}

bool NativeResize::close() {
    mDriver.close(vseFd);
    vseFd = -1;
    mDriver.removeBufferCallback();
    return true;
}

void NativeResize::onAllChannelsBufferDone(uint64_t addrSrc) {
    ALOGI("enter %s %d", __func__, bRunning);
    for (auto& item : mPadQueue[0].mBuffers) {
        releasePadBuffer(0, addrSrc);
    }
    sig.post();
}

// async mode,  never block at interrupt or trigerNext function.
void NativeResize::dequeueBufferThread() {
    while (bRunning) {
        sig.wait();
        if (!bRunning) break;
        int count = 0;
        for (int i = 0; i < 3; i++) {
            uint64_t addr = GPOP(&mDriver, 2, i);
            if (addr == 0) continue;
            auto& res = mPadCaps[1][i];
            ALOGD("deque one dst buffer: w:%d h:%d addr: %p,  channel %d", res.width, res.height, (unsigned char*)addr,  i);
            MediaAddrBuffer buffer;
            buffer.vcreate(res.width, res.height, res.format);
            buffer.baseAddress = addr;
            mSrc[i].image = &buffer;
            IMediaModule::trigerNext(i);
            mSrc[i].image = NULL;
            count++;
        }
    }
}

void NativeResize::onGlobalPadBufferChanged(int type, int port) {
    uint64_t dstAddress = 0;
    while ( (dstAddress = GPOP(this, type, port)) != 0) {
        ALOGI("%s: push external sink buffer %p into vse", __func__, (unsigned char*) dstAddress);
        GPUSH(&mDriver, type, port, dstAddress);
    }
}

void NativeResize::onConnectedPadBufferChanged(int port, MediaBufferQueue& queue, const char* remoteClass) {
    MediaAddrBuffer* pBuffer = NULL;
    if ((pBuffer = (MediaAddrBuffer*)queue.dequeueBuffer()) != NULL) {
        ALOGI("%s: push %s sink buffer %p into vse", __func__, remoteClass, (unsigned char*) pBuffer->baseAddress);
        GPUSH(&mDriver, 1, port, BASEADDR(pBuffer));
    }
}

bool NativeResize::run(uint32_t nSinkIndex) {
    if (!getSinkState(nSinkIndex) || !mSink[nSinkIndex].image)
        return false;;
    MediaBuffer* pBuffer = mSink[nSinkIndex].image;
    for (auto& item : mPadQueue[nSinkIndex].mBuffers) {
        if (BASEADDR(item) == BASEADDR(pBuffer)) {
            mPadQueue[nSinkIndex].queueBuffer(item);
            mPadQueue[nSinkIndex].acquiredBuffer();
            break;
        } else {
            // ALOGE("can't match %p %p", (void*)BASEADDR(item) , (void*)BASEADDR(pBuffer));
        }
        GPUSH(&mDriver, 0, 0,  BASEADDR(pBuffer));
    }
    setSinkState(nSinkIndex, false);
    return true;
}

void NativeResize::trigerNext(uint32_t nSrcIndex) {
    // ALOGI("Hooked passdown at NativeResize pad: %d",  nSrcIndex);
}

void NativeResize::setFormat(int nPort, MediaCap& res, int type) {
    IMediaModule::setFormat(nPort, res, type);
    ALOGI("setFormat %d %d %d, pad_type %d", res.format, res.width, res.height, type);

    if (type == PadTypeSrc) {
        params.out_size[nPort].width = res.width;
        params.out_size[nPort].height = res.height;
        params.mi_settings[nPort].width = res.width;
        params.mi_settings[nPort].height = res.height;
        params.format_conv[nPort].out_format = res.format;
        params.mi_settings[nPort].out_format = res.format;
    } else {
        params.src_w = res.width;
        params.src_h = res.height;
        params.in_format = res.format;
        // allocate buffer queue at sink pad
        mPadQueue[nPort].release();
        mPadQueue[nPort].create(res.width, res.height, res.format, 3);

    }
}

bool NativeResize::needBackwardFormat(int nPort) {
    return false;
}

IMPLEMENT_DYNAMIC_CLASS(NativeResize)

#endif
