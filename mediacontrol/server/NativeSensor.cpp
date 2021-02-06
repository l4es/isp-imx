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

#include "NativeSensor.h"

#include <iostream>
#include <fstream>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <memory.h>
#include <linux/videodev2.h>
#include "log.h"
#include "BufferManager.h"
#ifdef ENABLE_IRQ
# include "viv_video_kevent.h"
# include "vvdefs.h"
#endif

#define LOGTAG "NativeSensor"
#ifdef USE_SENSOR

NativeSensor::NativeSensor() {
    mSink.push_back(MediaPad(PAD_TYPE_IMAGE));
    mSrc.push_back(MediaPad(PAD_TYPE_IMAGE));
    mSrc.push_back(MediaPad(PAD_TYPE_IMAGE));
}

NativeSensor::~NativeSensor() {
    //close();
}

// queue all buffers before start streaming.  ispcore will handle internal buffer loop.
void NativeSensor::initBuffers() {
    uint64_t addr;
    auto& res = mPadCaps[1][0];

    int index = 0;

    if (isEndpoint(0)) {
        while ((addr = GPOP(this, 1, 0)) != 0) {
            ALOGI("push external sink buffer %p into isp",  (void*) addr);
            BufIdentity* buf = new BufIdentity;
            MediaAddrBuffer temp;
            temp.vcreate(res.width, res.height, res.format);
            buf->address_usr = addr;
            buf->address_reg = addr;
            buf->buff_size = temp.mSize;
            buf->buffer_idx = index++;
            mDriver.bufferPoolAddEntity(ISPCORE_BUFIO_MP, buf);
            bufList.push_back(buf);
        }
    } else {
        MediaAddrBuffer* pBuffer = NULL;
        for (auto& remotePad : mConnections[0]) {
             int port = remotePad.nSinkIndex;
             auto pNext = remotePad.pNext;
             while ((pBuffer = (MediaAddrBuffer*)pNext->mPadQueue[port].dequeueBuffer()) != NULL) {
                ALOGI("push %s sink buffer %p into isp", pNext->getClassName(), (void*) pBuffer->baseAddress);
                BufIdentity* buf = new BufIdentity;
                buf->address_usr = pBuffer->baseAddress;
                buf->address_reg = pBuffer->baseAddress;
                buf->buff_size = pBuffer->mSize;
                buf->buffer_idx = index++;
                mDriver.bufferPoolAddEntity(ISPCORE_BUFIO_MP, buf);
                bufList.push_back(buf);
             }
        }
    }
}

// bypass dewarp, still has drop frame.
void NativeSensor::onGlobalPadBufferChanged(int type, int port) {
    if (!bRunning)
        return;
    uint64_t addr;
    while ((addr = GPOP(this, 1, port)) != 0) {
        ALOGI("%s: push external sink buffer %p into isp",  __func__, (void*) addr);
        auto iter = fullQueue.find(addr);
        if (iter != fullQueue.end()) {
            mDriver.QBUF(ISPCORE_BUFIO_MP, iter->second);
        }
    }
}

void NativeSensor::onConnectedPadBufferChanged(int port, MediaBufferQueue& queue, const char* remoteClass) {
    if (!bRunning)
        return;
    MediaAddrBuffer* pBuffer = NULL;
    if ((pBuffer = (MediaAddrBuffer*)queue.dequeueBuffer()) != NULL) {
        ALOGI("%s: push %s sink buffer %p into isp", __func__, remoteClass, (void*) pBuffer->baseAddress);
        auto iter = fullQueue.find(pBuffer->baseAddress);
        if (iter != fullQueue.end()) {
            mDriver.QBUF(ISPCORE_BUFIO_MP, iter->second);
        }
    }
}

bool NativeSensor::start() {
    ALOGI("enter %s", __func__);
    mRefCount++;
    if (bRunning) return false;

    int port = 0;
    frameid = 0;
    int ret = 0;
#ifdef ENABLE_IRQ
    int ep = isEndpoint(0) ? 1 : 0;
    if (fd >= 0)
        ioctl(fd, VIV_VIDIOC_S_ENDPOINT, &ep);
#endif
    initBuffers();

    mDriver.bufferPoolKernelAddrMap(ISPCORE_BUFIO_MP, false);
    mDriver.bufferPoolSetBufToEngine(ISPCORE_BUFIO_MP);
    {
        Json::Value jRequest, jResponse;
        if (!bPreloaded) {
            ret = mDriver.post<const char*>(ISPCORE_MODULE_DEVICE_CALIBRATIONLOADANDINIT,
                    "calibration.file", "");
            ret |= mDriver.ioctl(ISPCORE_MODULE_SENSOR_OPEN, jRequest, jResponse);
            if (!ret)
                bPreloaded = true;
        }

        jRequest.clear();
        jRequest[DEVICE_CAMCONNECT_PREVIEW] = true;
        jRequest[DEVICE_WIDTH_PARAMS] = mPadCaps[PadTypeSrc][port].width;
        jRequest[DEVICE_HEIGHT_PARAMS] = mPadCaps[PadTypeSrc][port].height;
        jRequest[DEVICE_FORMAT_PARAMS] = mPadCaps[PadTypeSrc][port].format;
        ret |= mDriver.ioctl(ISPCORE_MODULE_DEVICE_CAMERA_CONNECT, jRequest, jResponse);
    }

    ret = mDriver.post<int>(ISPCORE_MODULE_DEVICE_PREVIEW_START, 0, 0);

    ret |= mDriver.initOutChain(ISPCORE_BUFIO_MP, 0);
    ret |= mDriver.startOutChain(ISPCORE_BUFIO_MP);
    ret |= mDriver.attachChain(ISPCORE_BUFIO_MP);

    bRunning = true;
    mThread = std::thread([this]() { loop(); });
    return 0 == ret;
}

bool NativeSensor::stop() {
    ALOGI("enter %s", __func__);
    if (--mRefCount == 0) {
        bRunning = false;
        mDriver.post<int>(ISPCORE_MODULE_DEVICE_PREVIEW_STOP, 0, 0);
        mThread.join();

        mDriver.post<int>(ISPCORE_MODULE_DEVICE_CAMERA_DISCONNECT, 0, 0);
        bPreloaded = false;
        int ret = mDriver.detachChain(ISPCORE_BUFIO_MP);
        ret |= mDriver.stopOutChain(ISPCORE_BUFIO_MP);
        ret |= mDriver.deInitOutChain(ISPCORE_BUFIO_MP);
        mDriver.bufferPoolClearBufList(ISPCORE_BUFIO_MP);
        GCLEAR(this);
        bufList.clear();
    }
    ALOGI("leave %s", __func__);
    return true;
}

void NativeSensor::loop() {
    buffCtrlEvent_t buffEvent;
    MediaBuffer_t * pBuf;
    while (bRunning) {
        int ret;
        ret = mDriver.waitForBufferEvent(ISPCORE_BUFIO_MP, &buffEvent, 100);
        if (ret !=0 ) {
            //ALOGE("%s waitForBufferEvent timeout", __func__);
            continue;
        }

        if(buffEvent.eventID == BUFF_CTRL_CMD_STOP) {
            ALOGW("get  BUFF_CTRL_CMD_STOP: %d", ret);
            break;
        }

        if(buffEvent.eventID != BUFF_CTRL_CMD_BUFFER_READY) {
            ALOGW("get BUFF_CTRL_CMD_READY: %d", ret);
            continue;
        }

        ret = mDriver.DQBUF(ISPCORE_BUFIO_MP, &pBuf);
        if (0 != ret) {
            ALOGW("%s DQBUF failed: %d", __func__, ret);
            continue;
        }

        auto res = mPadCaps[1][0];
        MediaAddrBuffer mediaBuffer;
        mediaBuffer.vcreate(res.width, res.height, res.format);
        mediaBuffer.baseAddress = (uint64_t)pBuf->baseAddress;
        mediaBuffer.mPort = 0;
        mediaBuffer.mFrameNumber = frameid++;
        ALOGI("%s framenumber %d", __func__, frameid);
        if (captureFlag) {
            char szFile[256];
            sprintf(szFile, "capture_isp_%dx%d_%d.yuv", res.width, res.height, frameid);
            mediaBuffer.save(szFile);
            captureFlag = false;
        }
        mSrc[0].image = (MediaBuffer*)&mediaBuffer;
        fullQueue[pBuf->baseAddress] = pBuf;
        trigerNext(mediaBuffer.mPort);
    }
}

bool NativeSensor::open() {
    void * cam_device_handle;
    int ret = mDriver.initHardware(CAM_ISPCORE_ID_0, &cam_device_handle);
    mDriver.initBufferPoolCtrl(ISPCORE_BUFIO_MP);
    mDriver.setBufferParameters(BUFF_MODE_USRPTR);

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
            int streamid = RESV_STREAMID_ISP0;
            ioctl(fd, VIV_VIDIOC_S_STREAMID, &streamid);
            break;
        }
    }
#endif
    return ret == 0;
}

bool NativeSensor::close() {
    mDriver.releaseHardware();
#ifdef ENABLE_IRQ
    if (fd >= 0)
        ::close(fd);
#endif
    return true;
}

bool NativeSensor::run(uint32_t nSinkIndex) {
    if (mSink[nSinkIndex].image == NULL) {
        return false;
    }

    if (!getSinkState(nSinkIndex)) {
        return false;
    }

    setSinkState(nSinkIndex, false);
    return true;
}

void NativeSensor::setFormat(int nPort, MediaCap& res, int type) {
    ALOGI("setFormat %d %d %d,  type %d, port: %d", res.format, res.width, res.height, type, nPort);
    IMediaModule::setFormat(nPort, res, type);
}

bool searchDevice(int id, std::string& subDevName, const char* type) {
    char szDeviceName[64] = { 0 };
    struct v4l2_capability cap;
    int pos = 0;
    int fd = -1;
    for (int index = 0; index < 30; index++) {
        sprintf(szDeviceName, "/dev/v4l-subdev%d", index);
        fd = open(szDeviceName, O_RDWR | O_NONBLOCK);
        if (fd < 0) {
            continue;
        }
        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) != 0) {
            ALOGW("errorchecking %s: %s", szDeviceName, (char*)cap.driver);
            close(fd);
            continue;
        }

        ALOGW("checking %s: %s", szDeviceName, (char*)cap.driver);
        if (strcmp((char*)cap.driver, type) == 0) {
            if (pos++ == id) {
                subDevName = szDeviceName;
                return true;
            }
        }
        close(fd);
        fd = -1;
    }

    if (fd < 0) {
        ALOGE("can't open %s %d!!!", type, id);
        exit(1);
    }
    return false;
}

bool NativeSensor::load(Json::Value& node) {
    std::string subDevName;
    ispIndex = node["isp.index"].asUInt();
    return true;
}

int NativeSensor::jsonRequest(int port, int ctrlId, Json::Value& request, Json::Value& response) {
    ALOGI("%s %d", __func__, ctrlId);
    int ret = -1;
    if (ctrlId == VIV_V4L_MC_CAPTURE) {
        captureFlag = true;
        return 0;
    }
    if (ctrlId == ISPCORE_MODULE_SENSOR_PRE_LOAD_LIB) {
        if (!bPreloaded) {
            ret = mDriver.post<const char*>(ISPCORE_MODULE_DEVICE_CALIBRATIONLOADANDINIT,
                    "calibration.file", "");
            ret |= mDriver.ioctl(ISPCORE_MODULE_SENSOR_OPEN, request, response);
            if (!ret)
                bPreloaded = true;
        }
        response["result"] = ret;
        return ret;
    }
    if (ctrlId > ISPCORE_MODULE_DEFAULT && ctrlId < ISPCORE_MODULE_END)
        ret = mDriver.ioctl(ctrlId, request, response);

    return ret;
}

IMPLEMENT_DYNAMIC_CLASS(NativeSensor)

#endif
