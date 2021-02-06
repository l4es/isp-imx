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
#include <semaphore.h>

#include "vsi_v4l2.h"
#include "vsi_drm.h"
#include "log.h"

#define LOGTAG "cam_menu"

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

void drm_destroy_fb(int fd, int index, struct drm_buffer *buf)
{
    struct drm_mode_destroy_dumb dreq;

    munmap(buf->fb_base, buf->size);
    drmModeRmFB(fd, buf->buf_id);

    memset(&dreq, 0, sizeof(dreq));
    dreq.handle = buf->handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
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
        ALOGD("DRM buffer[%d] addr=0x%p size=%d w/h=(%d,%d) buf_id=%d",
                 i, buf[i].fb_base, buf[i].size,
                 buf[i].width, buf[i].height, buf[i].buf_id);
    }
    drm->bits_per_pixel = 32;
    drm->bytes_per_pixel = drm->bits_per_pixel >> 3;
    return 0;
}

int open_drm_device(struct drm_device *drm)
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
        goto loop;
    }
    drm->drm_fd = fd;
    drm->card_id = --i;

    ALOGI("Open %s success", dev_name);

    return 0;
}

int drm_device_prepare(struct drm_device *drm)
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

