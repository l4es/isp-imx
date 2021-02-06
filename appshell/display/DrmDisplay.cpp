/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
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

#ifdef APPMODE_V4L2
# include <linux/videodev2.h>
#endif

#include "log.h"
#define LOGTAG "DrmDisplay"

#ifdef WITH_DRM
#include <MediaBuffer.h>
#include "DrmDisplay.h"
static int drm_create_fb(int fd, int index, struct drm_buffer *buf)
{
    struct drm_mode_create_dumb creq;
    struct drm_mode_destroy_dumb dreq;
    struct drm_mode_map_dumb mreq;
    int ret;

    memset(&creq, 0, sizeof(creq));
    creq.width = buf->width;
    creq.height = buf->height;
    creq.bpp = 32;

    ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
    if (ret < 0) {
        ALOGE("cannot create dumb buffer[%d]", index);
        return ret;
    }

    buf->stride = creq.pitch;
    buf->size = creq.size;
    buf->handle = creq.handle;

    ret = drmModeAddFB(fd, buf->width, buf->height, creq.bpp, creq.bpp,
                buf->stride, buf->handle, &buf->buf_id);

    if (ret < 0) {
        ALOGE("Add framebuffer (%d) fail", index);
        goto destroy_fb;
    }

    memset(&mreq, 0, sizeof(mreq));
    mreq.handle = buf->handle;
    ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
    if (ret) {
        ALOGE("Map buffer[%d] dump ioctl fail", index);
        goto remove_fb;
    }

    buf->fb_base = mmap(0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                            fd, mreq.offset);
    if (buf->fb_base == MAP_FAILED) {
        ALOGE("Cannot mmap dumb buffer[%d]", index);
        goto remove_fb;
    }
    memset(buf->fb_base, 255, buf->size);
    return 0;

remove_fb:
    drmModeRmFB(fd, buf->buf_id);
destroy_fb:
    memset(&dreq, 0, sizeof(dreq));
    dreq.handle = buf->handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    ALOGE("Create DRM buffer[%d] fail", index);
    return ret;
}

static void drm_destroy_fb(int fd, int index, struct drm_buffer *buf)
{
    struct drm_mode_destroy_dumb dreq;

    munmap(buf->fb_base, buf->size);
    drmModeRmFB(fd, buf->buf_id);

    memset(&dreq, 0, sizeof(dreq));
    dreq.handle = buf->handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    buf->fb_base = NULL;
}


static int modeset_find_crtc(struct drm_device *drm,
                drmModeRes *res, drmModeConnector *conn)
{
    drmModeEncoder *encoder;
    int drm_fd = drm->drm_fd;
    int crtc_id, j, i;

    for (i = 0; i < conn->count_encoders; i++) {
        encoder = drmModeGetEncoder(drm_fd, conn->encoders[i]);
        if (!encoder) {
            ALOGE("can't retrieve encoders[%d]", i);
            continue;
        }

        for (j = 0; j < res->count_crtcs; j++) {
            if (encoder->possible_crtcs & (1 << j)) {
                crtc_id = res->crtcs[j];
                if (crtc_id > 0) {
                    drm->crtc_id = crtc_id;
                    drmModeFreeEncoder(encoder);
                    return 0;
                }
            }
            crtc_id = -1;
        }

        if (j == res->count_crtcs && crtc_id == -1) {
            ALOGE("cannot find crtc");
            drmModeFreeEncoder(encoder);
            continue;
        }
        drmModeFreeEncoder(encoder);
    }
    ALOGE("cannot find suitable CRTC for connector[%d]", conn->connector_id);
    return -ENOENT;
}


static int modeset_setup_dev(struct drm_device *drm,
                drmModeRes *res, drmModeConnector *conn)
{
    struct drm_buffer *buf = drm->buffers;
    int i, ret;

    ret = modeset_find_crtc(drm, res, conn);
    if (ret < 0)
        return ret;

    drm->count_modes = conn->count_modes;
    drm->modes = (drmModeModeInfoPtr)malloc(sizeof(drmModeModeInfo) * drm->count_modes);
    memcpy(drm->modes,conn->modes,sizeof(drmModeModeInfo) * drm->count_modes);

    memcpy(&drm->mode, &conn->modes[0], sizeof(drm->mode));
    /* Double buffering */
    for (i = 0; i < 2; i++) {
        buf[i].width  = conn->modes[0].hdisplay;
        buf[i].height = conn->modes[0].vdisplay;
        ret = drm_create_fb(drm->drm_fd, i, &buf[i]);
        if (ret < 0) {
            while(i)
                drm_destroy_fb(drm->drm_fd, i - 1, &buf[i-1]);
            return ret;
        }
        ALOGD("DRM bufffer[%d] addr=0x%p size=%d w/h=(%d,%d) buf_id=%d",
                 i, buf[i].fb_base, buf[i].size,
                 buf[i].width, buf[i].height, buf[i].buf_id);
    }
    drm->bits_per_pixel = 32;
    drm->bytes_per_pixel = drm->bits_per_pixel >> 3;
    return 0;
}

static int open_drm_device(struct drm_device *drm)
{
    char dev_name[15];
    uint64_t has_dumb;
    int fd, i;

    i = 0;
loop:
    sprintf(dev_name, "/dev/dri/card%d", i++);

    fd = open(dev_name, O_RDWR | O_CLOEXEC | O_NONBLOCK);
    if (fd < 0) {
        ALOGE("Open %s fail", dev_name);
        return -1;
    }

    if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 ||
        !has_dumb) {
        ALOGE("drm device '%s' does not support dumb buffers", dev_name);
        close(fd);
        if (i < 4)
            goto loop;
    }
    drm->drm_fd = fd;
    drm->card_id = --i;

    ALOGI("Open %s success", dev_name);

    return 0;
}

static int drm_device_prepare(struct drm_device *drm)
{
    drmModeRes *res;
    drmModeConnector *conn;
    int drm_fd = drm->drm_fd;
    int ret, i;

    ret = drmSetMaster(drm_fd);
    if (ret < 0) {
        ALOGE("drmSetMaster fail: %d %s", ret,  strerror(ret));
        //return ret;
    }

    res = drmModeGetResources(drm_fd);
    if (res == NULL) {
        ALOGE("Cannot retrieve DRM resources");
        drmDropMaster(drm_fd);
        return -errno;
    }

    /* iterate all connectors */
    for (i = 0; i < res->count_connectors; i++) {
        /* get information for each connector */
        conn = drmModeGetConnector(drm_fd, res->connectors[i]);
        if (!conn) {
            ALOGE("Cannot retrieve DRM connector %u:%u (%d)",
                i, res->connectors[i], errno);
            continue;
        }

        /* valid connector? */
        if (conn->connection != DRM_MODE_CONNECTED ||
                    conn->count_modes == 0) {
            drmModeFreeConnector(conn);
            continue;
        }

        /* find a valid connector */
        drm->conn_id = conn->connector_id;
        ret = modeset_setup_dev(drm, res, conn);
        if (ret < 0) {
            ALOGE("mode setup device environment fail");
            drmDropMaster(drm_fd);
            drmModeFreeConnector(conn);
            drmModeFreeResources(res);
            return ret;
        }
        drmModeFreeConnector(conn);
    }
    drmModeFreeResources(res);
    return 0;
}


static int drm_adaptive_resolution(struct drm_device *drm, int width,int height)
{
    int i,ret;
    struct drm_buffer *buf = drm->buffers;

    for (i=0; i<drm->count_modes; i++)
    {
        if ((drm->modes[i].hdisplay == width) && (drm->modes[i].vdisplay == height))
        {
            break;
        }
    }

    if  ((drm->modes[i].hdisplay != width) || (drm->modes[i].vdisplay != height))
    {
         ALOGE("%s:DRM not support resolution %d*%d\n",__func__,width,height);
        return -1;
    }

    memcpy(&drm->mode, &drm->modes[i], sizeof(drm->mode));

    if (buf[0].fb_base != NULL)
    {
        drm_destroy_fb(drm->drm_fd, 0, &buf[0]);
    }
    if (buf[1].fb_base != NULL)
    {
        drm_destroy_fb(drm->drm_fd, 1, &buf[1]);
    }
    
    for (i = 0; i < 2; i++) {
        buf[i].width  = drm->mode.hdisplay;
        buf[i].height = drm->mode.vdisplay;
        ret = drm_create_fb(drm->drm_fd, i, &buf[i]);
        if (ret < 0) {
            while(i)
                drm_destroy_fb(drm->drm_fd, i - 1, &buf[i-1]);
            return ret;
        }
    }
    return 0;
}

DrmDisplay::DrmDisplay() {
    open_drm_device(&drm);
    drm_device_prepare(&drm);
}

DrmDisplay::~DrmDisplay() {
    drmDropMaster(drm.drm_fd);
    close(drm.drm_fd);
    if (drm.modes != NULL)
    {
        free(drm.modes);
        drm.modes = NULL;
    }
}

void DrmDisplay::showBuffer(unsigned char* data, int width, int height, int format, int size) {
    if (!data)
        return;
    struct drm_buffer* buf = &drm.buffers[frameid++%2];
    
    if ((buf->width != (unsigned int)width) || (buf->height != (unsigned int)height))
    {
        drm_adaptive_resolution(&drm,width,height);
    }

#ifndef APPMODE_V4L2
    switch (format) {
        case MEDIA_PIX_FMT_YUV422SP:
            convert422spToBGRX(data, (unsigned char*)buf->fb_base,  width, height, width, height);
            break;
        case MEDIA_PIX_FMT_YUV422I:
            convertYUYVToBGRX(data, (unsigned char*)buf->fb_base,  width, height, width, height);
            break;
        case MEDIA_PIX_FMT_YUV420SP:
            convertNV12ToBGRX(data, (unsigned char*)buf->fb_base,  width, height, width, height);
            break;
        default:
            return;
    }
#else
    switch (format) {
        case V4L2_PIX_FMT_NV16:
            convert422spToBGRX(data, (unsigned char*)buf->fb_base,  width, height, width, height);
            break;
        case V4L2_PIX_FMT_YUYV:
            convertYUYVToBGRX(data, (unsigned char*)buf->fb_base,  width, height, width, height);
            break;
        case V4L2_PIX_FMT_NV12:
            convertNV12ToBGRX(data, (unsigned char*)buf->fb_base,  width, height, width, height);
            break;
        case V4L2_PIX_FMT_SBGGR8:
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SBGGR10:
        case V4L2_PIX_FMT_SGBRG10:
        case V4L2_PIX_FMT_SGRBG10:
        case V4L2_PIX_FMT_SRGGB10:
        case V4L2_PIX_FMT_SBGGR12:
        case V4L2_PIX_FMT_SGBRG12:
        case V4L2_PIX_FMT_SGRBG12:
        case V4L2_PIX_FMT_SRGGB12:
            convertRawToBGRX(data, (unsigned char*)buf->fb_base,  width, height, width, height, format);
            break;
        default:
            return;
    }
#endif
    drmModeSetCrtc(drm.drm_fd, drm.crtc_id, buf->buf_id, 0, 0, &drm.conn_id, 1, &drm.mode);
}
#endif
