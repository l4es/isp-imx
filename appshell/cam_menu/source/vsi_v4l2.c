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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <assert.h>
#include <memory.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>

#include "vsi_v4l2.h"
#include "vsi_drm.h"
#include "vsi_fb.h"
#include "log.h"

#define LOGTAG "cam_menu"

int Width = 1920;
int Height = 1080;
static int frameid = 0;

static char camDevName[64];
static int camDevice = -1;
static int BufferCount = 4;
static int BufferType = V4L2_BUF_TYPE_VIDEO_CAPTURE;

static struct mapbuffer {
    void *start;
    uint32_t length;
} *mapbuf = NULL;

static enum OUTTYPE ImageProc = OUTPUTDRM;

static int drm_open_flag = 0;
static struct drm_device drm;

#define CLIP_COLOR(x)  x > 255 ? 255 :  x < 0  ? 0 : x;

static void convert422spToBGRX(unsigned char* yuv, unsigned char* bgr, int w, int h)
{
    unsigned char* puv = yuv + w*h;
    int Uin, Vin;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int Y = yuv[i*w+j];
            if (j % 2 == 0) {
                Uin = *puv++;
                Vin = *puv++;
            }
            int u = Uin - 128;
            int v = Vin - 128;

            signed u_b = u * 517;
            signed u_g = -u * 100;
            signed v_g = -v * 208;
            signed v_r = v * 409;

            signed tmp = (Y-16)* 298;
            signed b = (tmp + u_b) >> 8;
            signed g = (tmp + v_g + u_g) >> 8;
            signed r = (tmp + v_r) >> 8;

            int R = CLIP_COLOR(r);
            int G = CLIP_COLOR(g);
            int B = CLIP_COLOR(b);

            bgr[i*w*4 + j*4 + 0] = B;
            bgr[i*w*4 + j*4 + 1] = G;
            bgr[i*w*4 + j*4 + 2] = R;
            bgr[i*w*4 + j*4 + 3] = 255;
        }
    }
}

static int v4l2_read_frame(void)
{
    fd_set fds;
    FD_ZERO (&fds);
    FD_SET (camDevice, &fds);
    select (camDevice + 1, &fds, NULL, NULL, NULL);

    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type     = BufferType;
    buffer.memory   = V4L2_MEMORY_MMAP;
    
    if (ioctl(camDevice, VIDIOC_DQBUF, &buffer) < 0) {
        ALOGE("VIDIOC_DQBUF: %s", strerror(errno));
    } else {
        assert(buffer.index < BufferCount);
        ALOGI("VIDIOC_DQBUF success");
        char szFile[32];
        sprintf(szFile, "test_frame%d.yuv", frameid++);
        ALOGI("framenumber: %d", frameid);
        if (ImageProc == SAVEIMAGE) {
            FILE* pFile = fopen(szFile, "wb");
            fwrite(mapbuf[buffer.index].start, mapbuf[buffer.index].length, 1, pFile);
            fclose(pFile);
        } else if (ImageProc == OUTPUTDRM) {
            struct drm_buffer* buf = &drm.buffers[frameid%2];
            convert422spToBGRX(mapbuf[buffer.index].start, (unsigned char*)buf->fb_base,  Width, Height);
            drmModeSetCrtc(drm.drm_fd, drm.crtc_id, buf->buf_id, 0, 0, &drm.conn_id, 1, &drm.mode);
        } else if (ImageProc == OUTPUTFB) {
            write_data_to_fb(FrameBuffer, Frame_fd, mapbuf[buffer.index].start, Width, Height, Framebpp);    
	}

        ioctl(camDevice, VIDIOC_QBUF, &buffer);
    }

    return 0;
}

int v4l2_openCamera(void)
{
    if (camDevice >=0)
        return camDevice;

    for (int i = 0; i < 20; i++) {
        sprintf(camDevName, "/dev/video%d", i);
        camDevice = open(camDevName, O_RDWR | O_NONBLOCK);
        if (camDevice < 0) {
            ALOGE("can't open video file %s", camDevName);
            continue;
	} else {
            break;
	}
    }

    return 0;
}

int v4l2_querycap(void)
{
    int result;
    struct v4l2_capability caps;

    result = ioctl(camDevice, VIDIOC_QUERYCAP, &caps);
    if (result  < 0) {
        ALOGE("failed to get device caps for %s (%d = %s)", camDevName, errno, strerror(errno));
        return 0;
    }

    if (strcmp((const char*)caps.driver, "viv_v4l2_device") == 0) {
        ALOGI("found viv video dev %s", camDevName);
    }

    // Report device properties
    ALOGI("Open Device: %s (fd=%d)", camDevName, camDevice);
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
        ALOGE("Streaming capture not supported by %s.", camDevName);
        return false;
    }

    return 0;
}

int v4l2_enum_fmt(void)
{
    int i;
    struct v4l2_fmtdesc formatDescriptions;
    formatDescriptions.type = BufferType;

    for (i=0; ; i++) {
        formatDescriptions.index = i;
        if (ioctl(camDevice, VIDIOC_ENUM_FMT, &formatDescriptions) == 0) {
            ALOGI("  %2d: %s 0x%08X 0x%X",
                   i,
                   formatDescriptions.description,
                   formatDescriptions.pixelformat,
                   formatDescriptions.flags
            );
        } else {
            break;
        }
    }

    return 0;
}

int v4l2_set_fmt(unsigned int width, unsigned int height)
{
    struct v4l2_format format;

    format.type = BufferType;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    format.fmt.pix.width = Width;
    format.fmt.pix.height = Height;
    if (ioctl(camDevice, VIDIOC_S_FMT, &format) < 0) {
        ALOGE("VIDIOC_S_FMT: %s", strerror(errno));
        return -1;
    }

    return 0;
}

int v4l2_init_mmap(unsigned int bufferCount)
{
    int i;

    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = BufferType;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = BufferCount;
    if (ioctl(camDevice, VIDIOC_REQBUFS, &bufrequest) < 0) {
        ALOGE("VIDIOC_REQBUFS: %s", strerror(errno));
        return -1;
    }

    mapbuf = calloc(BufferCount, sizeof(*mapbuf));

    for (i = 0; i < BufferCount; i++) {
        struct v4l2_buffer buffer;

        memset(&buffer, 0, sizeof(buffer));
        buffer.type     = BufferType;
        buffer.memory   = V4L2_MEMORY_MMAP;
        buffer.index    = i;
        if (ioctl(camDevice, VIDIOC_QUERYBUF, &buffer) < 0) {
            ALOGE("VIDIOC_QUERYBUF: %s", strerror(errno));
            return false;
        }
        ALOGI("Buffer description:");
        ALOGI("  offset: %d", buffer.m.offset);
        ALOGI("  length: %d", buffer.length);

	mapbuf[i].length = buffer.length;
        mapbuf[i].start = mmap(
                NULL,
                buffer.length,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                camDevice,
                buffer.m.offset
        );
        if (mapbuf[i].start == MAP_FAILED) {
            ALOGE("mmap: %s", strerror(errno));
            return false;
        }
        ALOGI("map buffer %p", mapbuf[i].start);

        if (ioctl(camDevice, VIDIOC_QBUF, &buffer) < 0) {
            ALOGE("VIDIOC_QBUF: %s", strerror(errno));
            return false;
        }
    }

    return 0;
}

void v4l2_process_image(unsigned int imageProcess)
{
    ImageProc = imageProcess;
}

int v4l2_start_capture(void)
{
    int type = BufferType;

    if (ioctl(camDevice, VIDIOC_STREAMON, &type) < 0) {
        ALOGE("VIDIOC_STREAMON: %s", strerror(errno));
        return false;
    }

    if (ImageProc == OUTPUTDRM) {
        open_drm_device(&drm);
        drm_device_prepare(&drm);
        drm_open_flag = 1;
    }

    while (1)
        v4l2_read_frame();

    return 0;
}

int v4l2_stop_capture(void)
{
    int type = BufferType;

    if (ioctl(camDevice, VIDIOC_STREAMOFF, &type) < 0) {
        ALOGE("VIDIOC_STREAMOFF: %s", strerror(errno));
        return false;
    }

    return 0;
}

int v4l2_closeCamera(void)
{
    int i;

    if (camDevice < 0)
        return 0;

    for (i = 0; i < BufferCount; i++) {
        if (-1 == munmap(mapbuf[i].start, mapbuf[i].length)) {
            ALOGE(":VIDIOC_CLOSE: munmap %s", strerror(errno));
	    return false;
	}
    }
    free(mapbuf);

    close(camDevice);

    if ((ImageProc == OUTPUTDRM) || (drm_open_flag == 1)) {
        struct drm_buffer *buf = drm.buffers;
        drmDropMaster(drm.drm_fd);
        drm_destroy_fb(drm.drm_fd, 0, &buf[0]);
        drm_destroy_fb(drm.drm_fd, 1, &buf[1]);
        close(drm.drm_fd);
        drm_open_flag = 0;
    }

    if (ImageProc == OUTPUTFB) {
        exit_Framebuffer();
    }

    return 0;
}

void display_image(void)
{
    v4l2_openCamera();
    v4l2_querycap();
    v4l2_set_fmt(Width, Height);
    v4l2_init_mmap(BufferCount);
    v4l2_start_capture();
}

