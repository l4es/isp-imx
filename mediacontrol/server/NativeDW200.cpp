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

#include "NativeDW200.h"

#ifdef WITH_DW200
#include <assert.h>
#include <memory.h>

#include <map>
#include <fstream>
#include <string>
#include <vector>

#include "dw200dev.h"
#include "DW200Driver.h"
#include "DW200ConfigParser.h"
#include "log.h"

#define LOGTAG "NativeDW200"

// only support 2 input, 3 output
NativeDW200::NativeDW200() {
    mSink.resize(3, MediaPad(PAD_TYPE_IMAGE));
    mSrc.resize(3, MediaPad(PAD_TYPE_IMAGE));
}

NativeDW200::~NativeDW200() {
    stop();
}

bool NativeDW200::open() {
    dewarpFd = mDriver.open();
    if (dewarpFd == -1) {
        ALOGE("failed to open dewarp driver.");
        return false;
    }
    mDriver.registerBufferCallback(std::bind(&NativeDW200::onAllFramesDone, this, std::placeholders::_1, std::placeholders::_2));
    return true;
}

bool NativeDW200::close() {
    mDriver.removeBufferCallback();
    mDriver.close(dewarpFd);
    dewarpFd = -1;
    return true;
}

bool NativeDW200::start() {
    mRefCount++;
    if (bRunning) return false;

    if (!mDriver.setParams(&params)) {
        ALOGE("failed to set params");
        return false;
    }

    if (!mDriver.setDistortionMap(dmap)) {
        ALOGE("failed to set distortion map");
        return false;
    }

    mFrameNumber = 0;
    if (!mDriver.start()) {
        ALOGE("failed to start dewarp driver");
        return false;
    }
    bRunning = true;
    return true;
}

bool NativeDW200::stop() {
    if (--mRefCount <= 0) {
        if (!bRunning) return false;
        bRunning = false;
        mDriver.stop();
        GCLEAR(this);
        GCLEAR(&mDriver);
    }

    return true;
}

bool NativeDW200::load(Json::Value& node) {
    ALOGD("enter %s", __func__);
    memset(&params, 0, sizeof(params));
    memset(dmap, 0, sizeof(dmap));
    if (!dw200ParseJsonNode(node, dmap, params))
        return false;
    return true;
}

void NativeDW200::onAllFramesDone(uint64_t src0, uint64_t src1) {
    std::vector<std::string> vec;
    for (int nPort = 0; nPort < 4; nPort++) {
        uint64_t addr = GPOP(&mDriver, 2, nPort);
        if (addr == 0) continue;

        auto& res = mPadCaps[1][nPort];
        ALOGD("onFrameAvailable: w:%d h:%d  y_base: %p", res.width, res.height, (void*)addr);
        GPOP(&mDriver, 2, nPort);
        MediaAddrBuffer buffer;
        buffer.vcreate(res.width, res.height, res.format);
        buffer.baseAddress = addr;
        mSrc[nPort].image = &buffer;
        ALOGI("%s framenumber %d", __func__, mFrameNumber++);
        if (captureFlag) {
            char szFile[256];
            sprintf(szFile, "capture_dewarp_%dx%d_%d.yuv", res.width, res.height, mFrameNumber);
            buffer.save(szFile);
            sprintf(szFile, "capture_dewarp_map_%d.txt", mFrameNumber);
            FILE* pMatFile  = fopen(szFile, "w");
            for (int i = 0; i < 9; i++)
                fprintf(pMatFile, "%.4f, ", dmap[nPort].camera_matrix[i]);
            for (int i = 0; i < 8; i++)
                fprintf(pMatFile, "%.4f, ", dmap[nPort].distortion_coeff[i]);
            fclose(pMatFile);
            captureFlag = false;
        }
        IMediaModule::trigerNext(nPort);
        mSrc[nPort].image = NULL;
        // system("rm dump_src_data* -rf");
    }

    releasePadBuffer(0, src0);
    releasePadBuffer(1, src1);
}

void NativeDW200::onGlobalPadBufferChanged(int type, int port) {
    uint64_t dstAddress = 0;
    while ( (dstAddress = GPOP(this, type, port)) != 0) {
        ALOGI("%s: push external sink buffer %p into dw200", __func__, (unsigned char*) dstAddress);
        GPUSH(&mDriver, type, port, dstAddress);
    }
}

void NativeDW200::onConnectedPadBufferChanged(int port, MediaBufferQueue& queue, const char* remoteClass) {
    MediaAddrBuffer* pBuffer = NULL;
    if ((pBuffer = (MediaAddrBuffer*)queue.dequeueBuffer()) != NULL) {
        ALOGI("%s: push %s sink buffer %p into dw200", __func__, remoteClass, (unsigned char*) pBuffer->baseAddress);
        GPUSH(&mDriver, 1, port, BASEADDR(pBuffer));
    }
}

bool NativeDW200::run(uint32_t nSinkIndex) {
    if (!bRunning)
        return false;
    if (!getSinkState(nSinkIndex))
        return false;

    for (auto& item : mPadQueue[nSinkIndex].mBuffers) {
        if (BASEADDR(item) == BASEADDR(mSink[nSinkIndex].image)) {
            mPadQueue[nSinkIndex].queueBuffer(item);
            MediaBuffer* temp = mPadQueue[nSinkIndex].acquiredBuffer();
            if (temp)
                GPUSH(&mDriver, 0, nSinkIndex, BASEADDR(temp) );
            break;
        }
    }

    setSinkState(nSinkIndex, false);
    return true;
}

void NativeDW200::trigerNext(uint32_t nSrcIndex) {
    //ALOGI("Hooked passdown at NativeResize pad: %d",  nSrcIndex);
}

void NativeDW200::setFormat(int nPort, MediaCap& res, int type) {
    IMediaModule::setFormat(nPort, res, type);
    ALOGI("setFormat %d %d %d,  type %d, port: %d", res.format, res.width, res.height, type, nPort);
    if (type == PadTypeSrc) {
        params.input_res[nPort].width = res.width;
        params.input_res[nPort].height = res.height;
        params.input_res[nPort].format = res.format;
        params.input_res[nPort].enable = true;
    } else {
        params.output_res[nPort].width = res.width;
        params.output_res[nPort].height = res.height;
        params.output_res[nPort].format = res.format;
        params.output_res[nPort].enable = true;
        if (nPort > 0) {
            params.mi_settings[nPort-1].width = res.width;;
            params.mi_settings[nPort-1].height = res.height;
            params.vse_enableResizeOutput[nPort-1] = true;
            params.mi_settings[nPort-1].out_format = res.format;
            params.mi_settings[nPort-1].enable = true;
            params.vse_format_conv[nPort-1].out_format = res.format;
        }

        if (params.dewarp_type == DEWARP_MODEL_SPLIT_SCREEN) {
            params.split_horizon_line = res.height/2;
            params.split_vertical_line_up = res.width/2;
            params.split_vertical_line_down = res.width/2;
        }
        // allocate buffer queue at sink pad
        mPadQueue[nPort].release();
        mPadQueue[nPort].create(res.width, res.height, res.format, 6);
    }
}

int NativeDW200::jsonRequest(int port, int ctrlId, Json::Value& request, Json::Value& response) {

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
    case VIV_V4L_MC_CAPTURE:
        captureFlag = true;
        break;
    default: break;
    }
    return ret_val;
}

bool NativeDW200::needBackwardFormat(int nPort) {
    return nPort != 0;
}

IMPLEMENT_DYNAMIC_CLASS(NativeDW200)
#endif
