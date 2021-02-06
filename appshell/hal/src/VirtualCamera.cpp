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

#include "V4l2Camera.h"
#include "NativeCamera.h"
#include "viv_video_kevent.h"
#include "json_helper.h"

#include "log.h"

#define LOGTAG "VirtualCamera"

VirtualCamera* VirtualCamera::createObject() {
#ifdef APPMODE_V4L2
        return new V4l2Camera();
#elif defined (APPMODE_NATIVE)
        return new NativeCamera();
#endif
}

void parseMetadata(const Json::Value& root, struct isp_metadata* meta)
{
    std::string strKey;
    Json::Value node;
    // exp
    node = root[SECTION_NAME_EXP];
    if (node.isObject()) {
        JH_READ_VAL(meta->exp.enable, node, "enable");
        JH_READ_VAL(strKey, node, "mode");
        if (strKey != "null")
            meta->exp.mode = VIV_METADATA_EXP_MODE[strKey];
        readArrayFromNode(node, "rect", &meta->exp.rc.x);
        readArrayFromNode(node, "mean", meta->exp.mean);
    }

    // exp2
    node = root[SECTION_NAME_EXP2];
    if (node.isObject()) {
        JH_READ_VAL(meta->exp2.enable, node, "enable");
        readArrayFromNode(node, "rect", &meta->exp2.rc.x);
        readArrayFromNode(node, "mean", meta->exp2.mean);
        readArrayFromNode(node, "weight", &meta->exp2.r);
    }

    // awb
    node = root[SECTION_NAME_AWB];
    if (node.isObject()) {
        JH_READ_VAL(meta->awb.enable, node, "enable");
        JH_READ_VAL(strKey, node, "mode");
        if (strKey != "null")
            meta->awb.mode = VIV_METADATA_AWB_MODE[strKey];
        readArrayFromNode(node, "gain", &meta->awb.gain_r);
        readArrayFromNode(node, "mean", &meta->awb.r);
    }
    // afm
    node = root[SECTION_NAME_AFM];
    if (node.isObject()) {
        JH_READ_VAL(meta->afm.enable, node, "enable");
        readArrayFromNode(node, "window 0", &meta->afm.rc[0].x);
        readArrayFromNode(node, "window 1", &meta->afm.rc[1].x);
        readArrayFromNode(node, "window 2", &meta->afm.rc[2].x);
        readArrayFromNode(node, "sum", &meta->afm.sum_a);
        readArrayFromNode(node, "lum", &meta->afm.lum_a);
    }

    // vsm
    node = root[SECTION_NAME_VSM];
    if (node.isObject()) {
        JH_READ_VAL(meta->vsm.enable, node, "enable");
        JH_READ_VAL(meta->vsm.h_seg, node, "h_seg");
        JH_READ_VAL(meta->vsm.v_seg, node, "v_seg");
        JH_READ_VAL(meta->vsm.x, node, "x");
        JH_READ_VAL(meta->vsm.y, node, "y");
        readArrayFromNode(node, "rect", &meta->vsm.rc.x);
    }

    // hist
    node = root[SECTION_NAME_HIST];
    if (node.isObject()) {
        JH_READ_VAL(meta->hist.enable, node, "enable");
        JH_READ_VAL(meta->hist.type, node, "type");
        JH_READ_VAL(strKey, node, "mode");
        if (strKey != "null")
            meta->hist.mode = VIV_METADATA_HIST_MODE[strKey];
        readArrayFromNode(node, "rect", &meta->hist.rc.x);
        readArrayFromNode(node, "mean", meta->hist.mean);
    }
}

void VirtualCamera::updateMetadata(struct isp_metadata* meta, int path) {
    Json::Value jRequest, jResponse;
    jRequest["path"] = path;
    ioctl(ISPCORE_MODULE_DEVICE_GET_METADATA, jRequest, jResponse);
    std::string str = jResponse.toStyledString();
    ALOGW("%s", str.c_str());
}

VideoFile* VideoFile::mInst = NULL;

VideoFile::~VideoFile() {
    if (fd >= 0) {
        ::close(fd);
    }
}
    
int VideoFile::open() {
    if (fd >=0)
        return fd;
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
            ALOGE("failed to get device caps for %s (%d = %s)", szFile, errno, strerror(errno));
            return 0;
        }

        ALOGI("Open Device: %s (fd=%d)", szFile, fd);
        ALOGI("  Driver: %s", caps.driver);

        if (strcmp((const char*)caps.driver, "viv_v4l2_device") == 0) {
            ALOGI("found viv video dev %s", szFile);
            int streamid = -1;
            ioctl(fd, VIV_VIDIOC_S_STREAMID, &streamid);
            break;
        }
    }
    return fd;
}

uint64_t VideoFile::alloc(uint64_t size) {
    struct ext_buf_info buf = {0, size};
    ioctl(fd, VIV_VIDIOC_BUFFER_ALLOC, &buf);
    return buf.addr;
}

void VideoFile::free(uint64_t addr) {
    ioctl(fd, VIV_VIDIOC_BUFFER_FREE, addr);
}

void* VideoFile::mmap(uint64_t addr, uint64_t size) {
    return ::mmap(NULL, (int)size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr);
}

VideoFile* VideoFile::inst() {
    if (mInst == NULL) {
        mInst = new VideoFile();
    }
    return mInst;
}
