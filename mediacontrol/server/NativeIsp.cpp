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

#include "NativeIsp.h"

#include <iostream>
#include <memory.h>
#include "log.h"
#include "BufferManager.h"

#define LOGTAG "NativeIsp"

#ifdef CAM_INTERFACE
NativeIsp::NativeIsp() {
    mSink.push_back(MediaPad(PAD_TYPE_IMAGE));
    mSrc.push_back(MediaPad(PAD_TYPE_IMAGE));
}

NativeIsp::~NativeIsp() {
    delete mDriver;
}

void NativeIsp::updateBuffer() {
    uint64_t addr;
    auto& res = mPadCaps[0][0];
    if (isEndpoint(0)) {
        while ((addr = GPOP(this, 1, 0)) != 0) {
            ALOGI("push external sink buffer %p into isp",  (unsigned char*) addr);
            mDriver->queueBuffer(0, res.width, res.height, res.format, res.width*res.height, addr);
        }
    } else {
        MediaAddrBuffer* pBuffer = NULL;
        for (auto& remotePad : mConnections[0]) {
             int port = remotePad.nSinkIndex;
             auto pNext = remotePad.pNext;
             while ((pBuffer = (MediaAddrBuffer*)pNext->mPadQueue[port].dequeueBuffer()) != NULL) {
                 ALOGI("push %s sink buffer %p into isp", pNext->getClassName(), (unsigned char*) pBuffer->baseAddress);
                 mDriver->queueBuffer(0, pBuffer->mWidth, pBuffer->mHeight, pBuffer->mFormat,
                               pBuffer->mSize, pBuffer->baseAddress);
             }
        }
    }
}

bool NativeIsp::start() {
    mRefCount++;
    updateBuffer();
    return mDriver->start();
}

bool NativeIsp::stop() {
    if (--mRefCount == 0) {
        mDriver->stop();
        GCLEAR(this);
        GCLEAR(mDriver);
    }
    return true;
}

bool NativeIsp::open() {
    mDriver = CameraInterface::createObject(0);
    mDriver->registerBufferCallback(std::bind(&NativeIsp::onFrameAvailable, this, std::placeholders::_1) );
    return mDriver->open();
}

bool NativeIsp::close() {
    return mDriver->close();
}

bool NativeIsp::load(const char* szName) {
    return mDriver->load(szName);
}

void NativeIsp::onFrameAvailable(void* data){
    MediaBuffer* image = (MediaBuffer*)data;
    ALOGD("onFrameAvailable %d", image->mPort);
    mSrc[image->mPort].image = image;
    trigerNext(image->mPort);
    updateBuffer();
}

bool NativeIsp::run() {
    if (mSink[0].image == NULL) {
        return false;
    }

    if (!getSinkState(0)) {
        return false;
    }

    setSinkState(0, false);
    return true;
}

void NativeIsp::setFormat(int nPort, MediaCap& res, int type) {
    IMediaModule::setFormat(nPort, res, type);
    mDriver->setFormat(nPort, res.width, res.height, res.format);  // only set output for ISP
}

bool NativeIsp::setMode(int msgType, void* param) {
    mDriver->setMode(msgType, param);
    return true;
}

IMPLEMENT_DYNAMIC_CLASS(NativeIsp)
#endif

