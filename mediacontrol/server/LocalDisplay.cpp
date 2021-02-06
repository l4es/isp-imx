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

#include "LocalDisplay.h"

#include <unistd.h>

#include <iostream>
#include <map>
#include "log.h"


#define LOGTAG "LocalDisplay"

#define MAX_OUTPUT_PORT 6
LocalDisplay::LocalDisplay() {
    mSink.resize(MAX_OUTPUT_PORT, MediaPad(PAD_TYPE_IMAGE));
}

LocalDisplay::~LocalDisplay() {
}

bool LocalDisplay::run(uint32_t nSinkIndex) {
    static int nFrame[MAX_OUTPUT_PORT] = { 0 };

    if (!getSinkState(nSinkIndex))
        return false;
    ALOGI("get new buffer at pad %d", nSinkIndex);
    MediaBuffer* pBuffer = mSink[nSinkIndex].image;
    if (!pBuffer) return false;

    if (nFrame[nSinkIndex] < 1) {
        char szFile[256];
        const char * szFormatName = (pBuffer->mFormat == MEDIA_PIX_FMT_RGB888 ||
                            pBuffer->mFormat == MEDIA_PIX_FMT_RGB888P) ?
                        "test_stream%d_frame%d_%dx%d.ppm" :
                        "test_stream%d_frame%d_%dx%d.yuv";
        sprintf(szFile, szFormatName, nSinkIndex, nFrame[nSinkIndex], pBuffer->mWidth, pBuffer->mHeight);
        pBuffer->save(szFile);
        nFrame[nSinkIndex]++;
    }

    if (mBufferCallback) {
        pBuffer->mPort = nSinkIndex;
        mBufferCallback(pBuffer);
    }
    setSinkState(nSinkIndex, false);

    return true;
}

void LocalDisplay::setInputFormat(int id, const MediaCap& res) {
}

IMPLEMENT_DYNAMIC_CLASS(LocalDisplay)
