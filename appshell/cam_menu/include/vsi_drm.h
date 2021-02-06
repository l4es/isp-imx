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

#ifndef __VSI_DRM_H__
#define __VSI_DRM_H__

#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

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

    drmModeModeInfo mode;
    drmModeCrtc *saved_crtc;

    /* double buffering */
    struct drm_buffer buffers[2];
    __u32 nr_buffer;
    __u32 front_buf;
};

/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
extern int open_drm_device
(
    struct drm_device *drm
);

/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
extern int drm_device_prepare
(
    struct drm_device *drm
);

/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
extern void drm_destroy_fb
(
    int fd,
    int index,
    struct drm_buffer *buf
);

extern void drm_display_image(void);

#endif /* __VSI_DRM_H__ */

