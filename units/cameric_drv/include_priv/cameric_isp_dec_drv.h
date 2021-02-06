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

#ifndef __CAMERIC_ISP_DEC_DRV_H__
#define __CAMERIC_ISP_DEC_DRV_H__

/**
 * @file cameric_isp_dec_drv.h
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
 * @defgroup cameric_isp_dec_drv CamerIc ISP DEC Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_dec_drv_api.h>

#define AHBDEC_CONTROL_ResetValue                               0x0201018A
#define AHBDEC_CONTROL_EX_ResetValue                            0x00080000
#define AHBDEC_CONTROL_EX2_ResetValue                           0x003FC810

#define AHBDEC_WRITE_CONFIG_ResetValue                          0x00020000
#define AHBDEC_WRITE_EX_CONFIG_ResetValue                       0x00000000




/*****************************************************************************/
/**
 *          CamerIcIspDecInit()
 *
 * @brief   Initialize CamerIc ISP DEC driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspDecInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspDecRelease()
 *
 * @brief   Release/Frdec CamerIc ISP DEC driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspDecRelease
(
    CamerIcDrvHandle_t handle
);

/* @} cameric_isp_dec_drv */

#endif /* __CAMERIC_ISP_DEC_DRV_H__ */

