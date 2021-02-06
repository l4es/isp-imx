/******************************************************************************\
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2020> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2020 */

#ifndef __CAMERIC_ISP_ELAWB_DRV_H__
#define __CAMERIC_ISP_ELAWB_DRV_H__

/**
 * @file cameric_isp_elawb_drv.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup cameric_isp_awb_drv CamerIc ISP AWB Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_elawb_drv_api.h>



/*****************************************************************************/
/**
 * @brief   Structure of a measuring elipsis
 *
 *****************************************************************************/
typedef struct CamerIcIspElAwbElipse_s
{
    uint16_t    x;              /**< ellipsoid center x */
    uint16_t    y;              /**< ellipsoid center y */

    uint16_t    a1;             /**< rotation matrix coefficient (1,1) =  sin(angle) */
    uint16_t    a2;             /**< rotation matrix coefficient (1,2) =  cos(angle) */
    uint16_t    a3;             /**< rotation matrix coefficient (2,1) = -sin(angle) */
    uint16_t    a4;             /**< rotation matrix coefficient (2,2) =  cos(angle) */

    uint32_t    r_max_sqr;      /**< max radius square of ellipsiod */
} CamerIcIspElAwbElipse_t;



/*****************************************************************************/
/**
 * @brief
 *
 *****************************************************************************/
typedef struct CamerIcIspElAwbContext_s
{
    bool_t                  enabled;                                        /**< measuring enabled */
    bool_t                  MedianFilter;                                   /**< use median filter in pre filter module */

    CamerIcEventCb_t        EventCb;                                        /**< Event Callback */
    CamerIcWindow_t         Window;                                         /**< measuring window */

    CamerIcIspElAwbElipse_t Elipsis[CAMERIC_ISP_AWB_ELIPSIS_ID_MAX - 1U];
} CamerIcIspElAwbContext_t;



/*****************************************************************************/
/**
 *          CamerIcIspAwbInit()
 *
 * @brief   Initialize CamerIc ISP Auto white balance driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspElAwbInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspExpRelease()
 *
 * @brief   Release/Free CamerIc ISP Auto white balance driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspElAwbRelease
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspAwbSignal()
 *
 * @brief
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
void CamerIcIspElAwbSignal
(
    CamerIcDrvHandle_t handle
);



/* @} cameric_isp_awb_drv */

#endif /* __CAMERIC_ISP_ELAWB_DRV_H__ */

