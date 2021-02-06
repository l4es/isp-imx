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

#include "NativeDewarp.h"

#ifdef WITH_DWE

#include <assert.h>
#include <memory.h>

#include <map>
#include <fstream>
#include <string>
#include <vector>

#include "dewarpdev.h"
#include "DewarpDriver.h"
#include "DewarpConfigParser.h"
#include "log.h"
#ifdef ENABLE_IRQ
# include <fcntl.h>
# include <linux/videodev2.h>
# include "viv_video_kevent.h"
#endif

#define LOGTAG "NativeDewarp"

// only support 1 input, 1 output
NativeDewarp::NativeDewarp() {
    mSink.resize(3, MediaPad(PAD_TYPE_IMAGE));
    mSrc.resize(3, MediaPad(PAD_TYPE_IMAGE));
}

NativeDewarp::~NativeDewarp() {
    stop();
}

bool NativeDewarp::open() {
    dewarpFd = mDriver.open();
    if (dewarpFd == -1) {
        ALOGE("failed to open dewarp driver.");
        return false;
    }
    mDriver.registerBufferCallback(std::bind(&NativeDewarp::onFrameAvailable, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

#ifdef ENABLE_IRQ
    fd = -1;
    for (int i = 0; i < 20; i++) {
        char szFile[64];
        sprintf(szFile, "/dev/video%d", i);
        fd = ::open(szFile, O_RDWR | O_NONBLOCK);
        if (fd < 0) {
            ALOGE("can't open video file %s", szFile);
            continue;
        }
        v4l2_capability caps;
        int result = ioctl(fd, VIDIOC_QUERYCAP, &caps);
        if (result  < 0) {
            ALOGE("failed to get device caps for %s (%d = %s)",
                    szFile, errno, strerror(errno));
            return result;
        }

        ALOGI("Open Device: %s (fd=%d)", szFile, fd);
        ALOGI("  Driver: %s", caps.driver);

        if (strcmp((const char*)caps.driver, "viv_v4l2_device") == 0) {
            ALOGI("found viv video dev %s", szFile);
            int streamid = RESV_STREAMID_DWE; /*specific streamid assinged for dewarp*/
            ioctl(fd, VIV_VIDIOC_S_STREAMID, &streamid);
            break;
        }
    }
#endif
    return true;
}

bool NativeDewarp::close() {
    mDriver.removeBufferCallback();
    mDriver.close(dewarpFd);
    dewarpFd = -1;
#ifdef ENABLE_IRQ
    if (fd >= 0)
        ::close(fd);
#endif
    return true;
}

bool NativeDewarp::start() {
    mRefCount++;
    if (bRunning) return false;

#ifdef ENABLE_IRQ
    int ep = isEndpoint(0) ? 1 : 0;
    if (fd >= 0)
        ioctl(fd, VIV_VIDIOC_S_ENDPOINT, &ep);
#endif

    if (!mDriver.setParams(&params)) {
        ALOGE("failed to set params");
        return false;
    }

    if (!mDriver.setDistortionMap(dmap)) {
        ALOGE("failed to set distortion map");
        return false;
    }

    mFrameNumber = 0;
    const int buffernum = 4;

    for (auto& item : mPadCaps[0]) {
        if (item.second.width != 0 && item.second.height != 0) {
            mPadQueue[item.first].create(item.second.width, item.second.height, item.second.format, buffernum);
        }
    }

    if (!mDriver.start()) {
        ALOGE("failed to start dewarp driver");
        return false;
    }
    bRunning = true;
    return true;
}

bool NativeDewarp::stop() {
    ALOGI("enter %s", __func__);
    if (--mRefCount <= 0) {
        if (!bRunning) return false;
        bRunning = false;
        mDriver.stop();
        GCLEAR(this);
        GCLEAR(&mDriver);
        for (auto& item : mPadQueue) {
            item.second.release();
        }
        mPadQueue.clear();

    }
    ALOGI("leave %s", __func__);
    return true;
}

bool NativeDewarp::load(Json::Value& node) {
    ALOGD("enter %s", __func__);
    memset(&params, 0, sizeof(params));
    memset(dmap, 0, sizeof(dmap));
    std::vector<uint32_t> userMap;
    if (!dweParseJsonNode(node, userMap, dmap, params))
        return false;
    return true;
}

void NativeDewarp::onFrameAvailable(uint64_t addrSrc, uint64_t addrDst, int port) {
    auto& res = mPadCaps[1][port];
    // ALOGD("onFrameAvailable: w:%d h:%d  y_base: %p", res.width, res.height, (void*)addrDst);
    GPOP(&mDriver, 2, port);
    MediaAddrBuffer buffer;
    buffer.vcreate(res.width, res.height, res.format);
    buffer.baseAddress = addrDst;
    mSrc[port].image = &buffer;
    ALOGI("%s framenumber %d", __func__, mFrameNumber);
    mFrameNumber++;
    if (captureFlag) {
        char szFile[256];
        sprintf(szFile, "capture_dewarp_%dx%d_%d.yuv", res.width, res.height, mFrameNumber);
        buffer.save(szFile);
        sprintf(szFile, "capture_dewarp_map_%d.txt", mFrameNumber);
        FILE* pMatFile  = fopen(szFile, "w");
        for (int i = 0; i < 9; i++)
            fprintf(pMatFile, "%.4f, ", dmap[port].camera_matrix[i]);
        for (int i = 0; i < 8; i++)
            fprintf(pMatFile, "%.4f, ", dmap[port].distortion_coeff[i]);
        fclose(pMatFile);
        captureFlag = false;
    }
    releasePadBuffer(port, addrSrc);
    IMediaModule::trigerNext(port);
    mSrc[port].image = NULL;
}

void NativeDewarp::onGlobalPadBufferChanged(int type, int port) {
    uint64_t dstAddress = 0;
    while ( (dstAddress = GPOP(this, type, port)) != 0) {
        ALOGI("%s: push external sink buffer %p into dewarp", __func__, (void*) dstAddress);
        GPUSH(&mDriver, type, port, dstAddress);
    }
}

void NativeDewarp::onConnectedPadBufferChanged(int port, MediaBufferQueue& queue, const char* remoteClass) {
    MediaAddrBuffer* pBuffer = NULL;
    if ((pBuffer = (MediaAddrBuffer*)queue.dequeueBuffer()) != NULL) {
        ALOGI("%s: push %s sink buffer %p into dewarp", __func__, remoteClass, (void*) pBuffer->baseAddress);
        GPUSH(&mDriver, 1, port, BASEADDR(pBuffer));
    }
}

bool NativeDewarp::run(uint32_t nSinkIndex) {
    if (!bRunning) {
        return false;
    }

    if (!getSinkState(nSinkIndex))
        return false;
    MediaBuffer* pBuffer = mSink[nSinkIndex].image;

    for (auto& item : mPadQueue[nSinkIndex].mBuffers) {
        if (BASEADDR(item) == BASEADDR(pBuffer)) {
            mPadQueue[nSinkIndex].queueBuffer(item);
            //ALOGD("queue one src buffer channel %d w:%d h:%d s:%d size:%d  y_base: %p  %d %d  %d", i,
                //   item->mWidth, item->mHeight,item->mStride, item->mSize,
                //  item->getBuffer(), pBuffer->mStride, pBuffer->mSize, pBuffer->mBufferType);
            MediaBuffer* temp = mPadQueue[nSinkIndex].acquiredBuffer();
            if (temp) GPUSH(&mDriver, 0, nSinkIndex, BASEADDR(temp) );
            break;
        }
    }

    setSinkState(nSinkIndex, false);
    return true;
}

void NativeDewarp::trigerNext(uint32_t nSrcIndex) {
    //ALOGI("Hooked passdown at NativeResize pad: %d",  nSrcIndex);
}

void NativeDewarp::setFormat(int nPort, MediaCap& res, int type) {
    IMediaModule::setFormat(nPort, res, type);
    ALOGI("setFormat %d %d %d,  type %d, port: %d", res.format, res.width, res.height, type, nPort);
    if (type == PadTypeSrc) {
        params.image_size_dst.width = res.width;
        params.image_size_dst.height = res.height;
        params.pix_fmt_out = res.format;
    } else {
        params.image_size.width = res.width;
        params.image_size.height = res.height;
        params.pix_fmt_in = res.format;
        if (params.dewarp_type == DEWARP_MODEL_SPLIT_SCREEN) {
            params.split_horizon_line = res.height/2;
            params.split_vertical_line_up = res.width/2;
            params.split_vertical_line_down = res.width/2;
        }
    }
}

int NativeDewarp::jsonRequest(int port, int ctrlId, Json::Value& request, Json::Value& response) {
    int ret_val = 0;
    switch (ctrlId) {
    case VIV_V4L_DWE_SET_PARAM: {
        Json::Value node = request["dwe"];
        params.dewarp_type = node["mode"].asInt();
        params.hflip = node["hflip"].asBool();
        params.vflip = node["vflip"].asBool();
        params.bypass = node["bypass"].asBool();
        for (int i = 0; i < 9; i++) {
            dmap[port].camera_matrix[i] = node["mat"][i].asDouble();
        }
        for (int i = 9; i < 17; i++) {
             dmap[port].distortion_coeff[i-9] = node["mat"][i].asDouble();
        }

        if (!mDriver.setParams(&params)) {
            ALOGE("failed to set params");
        }

        if (!mDriver.setDistortionMap(dmap)) {
            ALOGE("failed to set distortion map");
        }

        break;
    }
    case VIV_V4L_DWE_GET_PARAM: {
        Json::Value ret;
        ret["mode"] = params.dewarp_type;
        ret["hflip"] = params.hflip;
        ret["vflip"] = params.vflip;
        ret["bypass"] = params.bypass;
        for (int i = 0; i < 9; i++) {
            ret["mat"][i] = dmap[port].camera_matrix[i];
        }
        for (int i = 9; i < 17; i++) {
            ret["mat"][i] = dmap[port].distortion_coeff[i-9];
        }

        response["dwe"] = ret;
        break;
    }
    case VIV_V4L_DWE_SET_HFLIP: {
        Json::Value node = request["dwe"];
        params.hflip = node["hflip"].asBool();
        mDriver.setFlip(port, params.hflip, params.vflip);
        break;
    }
    case VIV_V4L_DWE_SET_VFLIP : {
        Json::Value node = request["dwe"];
        params.vflip = node["vflip"].asBool();
        mDriver.setFlip(port, params.hflip, params.vflip);
        break;
    }
    case VIV_V4L_DWE_SET_BYPASS : {
        Json::Value node = request["dwe"];
        params.bypass = node["bypass"].asBool();
        if (!mDriver.setParams(&params)) {
            ALOGE("failed to set params");
        }

        if (!mDriver.setDistortionMap(dmap)) {
            ALOGE("failed to set distortion map");
        }
        break;
    }
    case VIV_V4L_DWE_SET_MODE : {
        Json::Value node = request["dwe"];
        params.dewarp_type = node["mode"].asInt();
        if (!mDriver.setParams(&params)) {
            ALOGE("failed to set params");
        }

        if (!mDriver.setDistortionMap(dmap)) {
            ALOGE("failed to set distortion map");
        }
        break;
    }
    case VIV_V4L_DWE_SET_MATRIX: {
        Json::Value node = request["dwe"];
        for (int i = 0; i < 9; i++) {
            dmap[port].camera_matrix[i] = node["mat"][i].asDouble();
        }
        for (int i = 9; i < 17; i++) {
             dmap[port].distortion_coeff[i-9] = node["mat"][i].asDouble();
        }
        if (!mDriver.setDistortionMap(dmap)) {
            ALOGE("failed to set distortion map");
        }
        break;
    }
    case VIV_V4L_MC_CAPTURE:
        captureFlag = true;
        break;
    default: break;
    }
    return ret_val;
}

IMPLEMENT_DYNAMIC_CLASS(NativeDewarp)

#endif
