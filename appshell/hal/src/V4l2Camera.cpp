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

#include <linux/videodev2.h>
#include <linux/media.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <memory.h>
#include <inttypes.h>


#ifdef APPMODE_V4L2

#include "V4l2Camera.h"
#include <assert.h>

#include <log.h>

#define LOGTAG "V4l2Camera"

V4l2Camera::~V4l2Camera() {

}

int V4l2Camera::open(int id) {
    char szFile[16];
    sprintf(szFile, "/dev/video%d", id);
    fd = ::open(szFile, O_RDWR | O_NONBLOCK);

    if (fd < 0) {
        ALOGE("can't open camera: %s", szFile);
        exit(1);
    }

    v4l2_capability caps;
    {
        int result = ::ioctl(fd, VIDIOC_QUERYCAP, &caps);
        if (result  < 0) {
            ALOGE("failed to get device caps for %s (%d = %s)", szFile, errno, strerror(errno));
            exit(1);
        }
    }

    // Report device properties
    ALOGI("Open Device: %s (fd=%d)", szFile, fd);
    ALOGI("  Driver: %s", caps.driver);
    ALOGI("  Card: %s", caps.card);
    ALOGI("  Version: %u.%u.%u",
            (caps.version >> 16) & 0xFF,
            (caps.version >> 8)  & 0xFF,
            (caps.version)       & 0xFF);
    ALOGI("  All Caps: %08X", caps.capabilities);
    ALOGI("  Dev Caps: %08X", caps.device_caps);

    if (!(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
        !(caps.capabilities & V4L2_CAP_STREAMING)) {
        ALOGE("Streaming capture not supported by %s.", szFile);
        exit(1);
    }
#ifdef SUPPORT_PASS_JSON
    VideoFile::inst()->open();
    request_pa = VideoFile::inst()->alloc(VIV_JSON_BUFFER_SIZE);
    response_pa = VideoFile::inst()->alloc(VIV_JSON_BUFFER_SIZE);

    request_va = (char*)VideoFile::inst()->mmap(request_pa, VIV_JSON_BUFFER_SIZE);
    response_va = (char*)VideoFile::inst()->mmap(response_pa, VIV_JSON_BUFFER_SIZE);
    ALOGI("allocate json address \n0x%" PRIx64 "\n 0x%" PRIx64 "\n %p %p", request_pa, response_pa, reinterpret_cast<void*>(request_va), reinterpret_cast<void*>(response_va));

#endif
    return 0;
}

void V4l2Camera::close(int id) {
    if (fd)
        ::close(fd);
#ifdef SUPPORT_PASS_JSON
    VideoFile::inst()->free(request_pa);
    VideoFile::inst()->free(response_pa);
#endif
}

int V4l2Camera::start() {
    auto& res = mCaps[0];
    const int videoType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_format format;
    format.type = videoType;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    format.fmt.pix.width = res.width;
    format.fmt.pix.height = res.height;
    if (::ioctl(fd, VIDIOC_S_FMT, &format) < 0) {
        ALOGE("VIDIOC_S_FMT: %s", strerror(errno));
        return -1;
    }

    int bufferCount = 4;
    v4l2_requestbuffers bufrequest;
    bufrequest.type = videoType;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = bufferCount;
    if (::ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0) {
        ALOGE("VIDIOC_REQBUFS: %s", strerror(errno));
        return -1;
    }

    v4l2_buffer buffer = {};
    for (int i = 0; i < bufferCount; i++) {
        memset(&buffer, 0, sizeof(buffer));
        buffer.type     = videoType;
        buffer.memory   = V4L2_MEMORY_MMAP;
        buffer.index    = i;
        if (::ioctl(fd, VIDIOC_QUERYBUF, &buffer) < 0) {
            ALOGE("VIDIOC_QUERYBUF: %s", strerror(errno));
            return 1;
        }
        ALOGI("Buffer description:");
        ALOGI("  offset: %d", buffer.m.offset);
        ALOGI("  length: %d", buffer.length);
        if (::ioctl(fd, VIDIOC_QBUF, &buffer) < 0) {
            ALOGE("VIDIOC_QBUF: %s", strerror(errno));
            return 1;
        }
        void* data = mmap(
                NULL,
                buffer.length,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                fd,
                buffer.m.offset
        );
        if (data == MAP_FAILED) {
            ALOGE("mmap: %s", strerror(errno));
            return 1;
        }
        ALOGI("map buffer %p", data);
        mappedBuffers.push_back(data);
    }

    int type = buffer.type;
    if (::ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        ALOGE("VIDIOC_STREAMON: %s", strerror(errno));
        return 1;
    }

    bRunning = true;
    mThread = std::thread([this]() { loop(); });
    return 0;
}

int V4l2Camera::connectCamera() {
    return 0;
}

int V4l2Camera::disconnectCamera() {
    return 0;
}

int V4l2Camera::stop() {
    
    bRunning = false;
    mThread.join();

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ::ioctl(fd, VIDIOC_STREAMOFF, &type);
    return 0;
}

#ifdef SUPPORT_PASS_JSON
int V4l2Camera::ioctl(int cmdID, Json::Value& jsonRequest, Json::Value& jsonResponse) {
    EAutoLock l(&mMutex);
    ALOGW("call %s %d", __func__, cmdID);

    int eventFile = VideoFile::inst()->open();
    strcpy(request_va, jsonRequest.toStyledString().c_str());
    struct viv_control_event event;
    event.request = request_pa;
    event.response = response_pa;
    event.id = cmdID;
    int ret = ::ioctl(eventFile, VIV_VIDIOC_CONTROL_EVENT, &event);
    if (ret != 0)
        return ret;
    Json::Reader reader;
    ALOGW("call %s %d: response: %s", __func__, cmdID, response_va);
    std::string strResponse = response_va;
    reader.parse(strResponse, jsonResponse, true);
    return jsonResponse["MC_RET"].asInt();
}
#else
int V4l2Camera::ioctl(int cmdID, Json::Value& jsonRequest, Json::Value& jsonResponse) {
    ALOGW("call %s %d", __func__, cmdID);
    return 0;
}
#endif

void V4l2Camera::loop() {
    BufIdentity outBuf;
    struct v4l2_buffer buffer = {};
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = 0;
    int frameid = 0;
    while (bRunning) {
        fd_set fds;
        FD_ZERO (&fds);
        FD_SET (fd, &fds);
        select (fd + 1, &fds, NULL, NULL, NULL);

        if (::ioctl(fd, VIDIOC_DQBUF, &buffer) < 0) {
            ALOGE("VIDIOC_DQBUF: %s", strerror(errno));
        } else {
            ALOGI("VIDIOC_DQBUF success: %d", frameid++);
            if (mBufferCallback) {
                outBuf.width = mCaps[0].width;
                outBuf.height = mCaps[0].height;
                outBuf.format = mCaps[0].format;
                int size = outBuf.height*outBuf.width*cameraFormatBpp[outBuf.format]/8;
                outBuf.address_usr = (uint64_t)mappedBuffers[buffer.index];
                outBuf.buff_size = size;
                mBufferCallback(&outBuf, 0);
            }

            ::ioctl(fd, VIDIOC_QBUF, &buffer);
        }
    }
}

#endif
