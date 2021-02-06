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

#include "FakeSource.h"

#include <unistd.h>
#include <inttypes.h>
#include "log.h"

#define LOGTAG "FakeSource"

FakeSource::FakeSource() {
    mSink.push_back(MediaPad(PAD_TYPE_INT));
    mSrc.push_back(MediaPad(PAD_TYPE_IMAGE));
    mSrc.push_back(MediaPad(PAD_TYPE_IMAGE));
}

FakeSource::~FakeSource() {
}

bool FakeSource::start() {
    mRefCount++;
    if (bRunning) return false;
    bRunning = true;
    mThread = std::thread([this]() { loop(); });
    return true;
}

bool FakeSource::stop() {
    ALOGD("enter %s", __func__);
    if (--mRefCount == 0) {
        if (!bRunning) return false;
        bRunning = false;
        mThread.join();
    }
    return true;
}

int FakeSource::jsonRequest(int port, int ctrlId, Json::Value& request, Json::Value& response) {
    ALOGI("%s %d", __func__, ctrlId);
    int ret = -1;
    return ret;
}

void FakeSource::loop() {
    auto res = getFormat(0, PadTypeSrc);
    ALOGI("res %d %d %d", res.width, res.height, res.format);
    uint64_t addr;
    MediaUserBuffer presetImage;
    presetImage.create(res.width, res.height, res.format);
    unsigned char* data = presetImage.getBuffer();
    for (int i = 0; i < res.height; i++) {
        for (int j = 0; j < res.width; j++) {
            data[i*res.width + j] = j / (res.width/4) * 63;
        }
    }
    memset(data + res.width*res.height, 172, presetImage.mSize - res.width*res.height);

    int index = 0;
    while (bRunning) {
        float delay_us = 1000*1000.0f/mFps;
        delay_us -= 1000; //memcpy
        usleep(1);
        addr = 0;
        if (isEndpoint(0)) {
            addr = GPOP(this, 1, 0);
        } else {
            MediaAddrBuffer* pBuffer = NULL;
            for (auto& remotePad : mConnections[0]) {
                int port = remotePad.nSinkIndex;
                auto pNext = remotePad.pNext;
                if ((pBuffer = (MediaAddrBuffer*)pNext->mPadQueue[port].dequeueBuffer()) != NULL) {
                    addr = pBuffer->baseAddress;
                    break;
                }
            }
        }
        if (addr == 0) continue;
        MediaAddrBuffer temp;
        temp.vcreate(res.width, res.height, res.format);
        temp.baseAddress = addr;
        presetImage.copyTo(&temp);
        mSrc[0].image = &temp;
        trigerNext(0);
        ALOGI("fakesource ouput: %d", index++);
    }
}

bool FakeSource::run(uint32_t nSinkIndex) {
    if ( !getSinkState(nSinkIndex) ) {
        return false;
    }

    setSinkState(nSinkIndex, false);
    return true;
}

void FakeSource::setFormat(int nPort, MediaCap& res, int type) {
    IMediaModule::setFormat(nPort, res, type);
}

IMPLEMENT_DYNAMIC_CLASS(FakeSource)
