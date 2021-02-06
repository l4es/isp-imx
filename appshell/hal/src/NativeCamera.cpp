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
#include "NativeCamera.h"
#include <assert.h>

#ifdef APPMODE_NATIVE

#include <log.h>

#define LOGTAG "NativeCamera"

NativeCamera::~NativeCamera() {

}

int NativeCamera::open(int id) {
    // initialize the CamDevice
    {
        Json::Value jRequest, jResponse;
        CAM_DEVICE::getHwResources(jRequest, jResponse);
    }
    {
        Json::Value jRequest, jResponse;
        CAM_DEVICE::getHwStatus(jRequest, jResponse);
    }

    void * cam_device_handle;
    int ret = mDevice.initHardware(CAM_ISPCORE_ID_0, &cam_device_handle);
    mDevice.setBufferParameters(BUFF_MODE_PHYLINEAR);
    mDevice.initBufferPoolCtrl(ISPCORE_BUFIO_MP);

    mDevice.bufferPoolKernelAddrMap(ISPCORE_BUFIO_MP, false);
    mDevice.bufferPoolSetBufToEngine(ISPCORE_BUFIO_MP);

    ret = mDevice.post<const char*>(ISPCORE_MODULE_DEVICE_CALIBRATIONLOADANDINIT, "calibration.file", "OV2775_8M_02_720p.xml");
    fd = VideoFile::inst()->open();
    return ret;
}

void NativeCamera::close(int id) {
    mDevice.releaseHardware();
}

int NativeCamera::start() {
    mDevice.initOutChain(ISPCORE_BUFIO_MP, 0);
    mDevice.startOutChain(ISPCORE_BUFIO_MP);
    mDevice.attachChain(ISPCORE_BUFIO_MP);

    bRunning = true;
    mThread = std::thread([this]() { loop(); });
    return mDevice.post<int>(ISPCORE_MODULE_DEVICE_PREVIEW_START,  0, 0);
}

int NativeCamera::connectCamera() {
    Json::Value jRequest, jResponse;
    jRequest[DEVICE_CAMCONNECT_PREVIEW] = true;
    jRequest[DEVICE_WIDTH_PARAMS] = mCaps[0].width;
    jRequest[DEVICE_HEIGHT_PARAMS] = mCaps[0].height;
    jRequest[DEVICE_FORMAT_PARAMS] = mCaps[0].format;
    jRequest[DEVICE_FRAME_PARAMS] = 0;
    jRequest[HDR_ENABLE_PARAMS] = true;
    jRequest[SENSOR_BAYER_PATTERN_PARAMS] = "GRBG";
    return mDevice.ioctl(ISPCORE_MODULE_DEVICE_CAMERA_CONNECT, jRequest, jResponse);
}

int NativeCamera::disconnectCamera() {
    return mDevice.post<int>(ISPCORE_MODULE_DEVICE_CAMERA_DISCONNECT,  0, 0);
}

int NativeCamera::stop() {
    mDevice.detachChain(ISPCORE_BUFIO_MP);
    mDevice.stopOutChain(ISPCORE_BUFIO_MP);
    mDevice.deInitOutChain(ISPCORE_BUFIO_MP);
    bRunning = false;
    mThread.join();
    return mDevice.post<int>(ISPCORE_MODULE_DEVICE_PREVIEW_STOP,  0, 0);
}

int NativeCamera::ioctl(int cmdID, Json::Value& jsonRequest, Json::Value& jsonResponse) {
    return mDevice.ioctl(cmdID, jsonRequest, jsonResponse);
}

void NativeCamera::loop() {
    buffCtrlEvent_t buffEvent;
    MediaBuffer_t * pBuf;
    BufIdentity outBuf;
    while (bRunning) {
        int ret;
        ret = mDevice.waitForBufferEvent(ISPCORE_BUFIO_MP, &buffEvent, 1000);
        if (ret != 0) continue;
        if( buffEvent.eventID != BUFF_CTRL_CMD_BUFFER_READY) {
            continue;
        }
        ret = mDevice.DQBUF(ISPCORE_BUFIO_MP, &pBuf);
        if ( 0 != ret ) continue;

        if (mBufferCallback) {
            outBuf.width = mCaps[0].width;
            outBuf.height = mCaps[0].height;
            outBuf.format = mCaps[0].format;
            int size = outBuf.height*outBuf.width*cameraFormatBpp[outBuf.format]/8;
            outBuf.address_usr = (uint64_t)VideoFile::inst()->mmap(pBuf->baseAddress, size);
            //ALOGE("get data 0x%08x %d", pBuf->baseAddress, size)

            outBuf.buff_size = size;
            mBufferCallback(&outBuf, ISPCORE_BUFIO_MP);
        }
        ret = mDevice.QBUF(ISPCORE_BUFIO_MP, pBuf);
    }
}

#endif
