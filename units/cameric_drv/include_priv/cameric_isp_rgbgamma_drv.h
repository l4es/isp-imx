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

#ifndef __CAMERIC_ISP_RGBGAMMA_DRV_H__
#define __CAMERIC_ISP_RGBGAMMA_DRV_H__

#ifdef ISP_RGBGC
/**
 * @file cameric_isp_rgbgamma_drv.h, ISP NANO only
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
 * @defgroup cameric_isp_rgbgamma_drv CamerIc ISP RGBGAMMA Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_rgbgamma_drv_api.h>



#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/**
 *          CamerIcIspRgbGammaInit()
 *
 * @brief   Initialize CamerIc ISP Gcmono driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspRgbGammaInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspRgbGammaRelease()
 *
 * @brief   Release/Free CamerIc ISP Gcmono driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspRgbGammaRelease
(
    CamerIcDrvHandle_t handle
);



#ifdef __cplusplus
}
#endif



/* @} cameric_isp_rgbgamma_drv */

#endif /* ISP_RGBGAMMA */
#endif /* __CAMERIC_ISP_RGBGAMMA_DRV_H__ */


