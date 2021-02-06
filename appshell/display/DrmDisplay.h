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
#ifndef _APPSHELL_V4LDRMTEST_DRMDISPLAY_H_
#define _APPSHELL_V4LDRMTEST_DRMDISPLAY_H_

#include "IDisplay.h"
#ifdef WITH_DRM
#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <drm/drm_fourcc.h>

struct drm_buffer {
    void *fb_base;

    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t size;

    uint32_t handle;
    uint32_t buf_id;
};

struct drm_device {
    int drm_fd;

    int32_t crtc_id;
    int32_t card_id;
    uint32_t conn_id;

    __u32 bits_per_pixel;
    __u32 bytes_per_pixel;

    int count_modes;
	drmModeModeInfoPtr modes;

    drmModeModeInfo mode;
    drmModeCrtc *saved_crtc;

    /* double buffering */
    struct drm_buffer buffers[2];
    __u32 nr_buffer;
    __u32 front_buf;
};

class DrmDisplay : public IDisplay {
public:
    DrmDisplay();
    ~DrmDisplay();
    void showBuffer(unsigned char* data, int width, int height, int format, int size);
    struct drm_device drm;
    int frameid = 0;
};

#endif
#endif // _APPSHELL_V4LDRMTEST_DRMDISPLAY_H_

