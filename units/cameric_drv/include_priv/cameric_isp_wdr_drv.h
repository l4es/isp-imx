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

#ifndef __CAMERIC_ISP_WDR_DRV_H__
#define __CAMERIC_ISP_WDR_DRV_H__

/**
 * @file cameric_isp_wdr_drv.h
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
 * @defgroup cameric_isp_wdr_drv CamerIc ISP WDR Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_wdr_drv_api.h>



/*******************************************************************************
 *
 *          CamerIcIspWdrContext_t
 *
 * @brief
 *
 */
typedef struct CamerIcIspWdrContext_s
{
    bool_t      enabled;
    uint16_t    LumOffset;
    uint16_t    RgbOffset;
} CamerIcIspWdrContext_t;



/*****************************************************************************/
/**
 *          CamerIcIspWdrInit()
 *
 * @brief   Initialize CamerIc ISP WDR driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspWdrInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspWdrRelease()
 *
 * @brief   Release/Free CamerIc ISP WDR driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspWdrRelease
(
    CamerIcDrvHandle_t handle
);



/* @} cameric_isp_wdr_drv */

#endif /* __CAMERIC_ISP_WDR_DRV_H__ */

