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

#ifndef __CAMERIC_IE_DRV_H__
#define __CAMERIC_IE_DRV_H__

/**
 * @cond    cameric_ie_drv
 *
 * @file    cameric_ie_drv.h
 *
 * @brief   This file contains driver internal definitions for the CamerIC
 *          driver IE (image effects) module.
 *
 *****************************************************************************/
/**
 * @defgroup cameric_ie_drv CamerIc IE Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_ie_drv_api.h>



/******************************************************************************/
/**
 * @brief driver internal context of Image Effects Module.
 *
 *****************************************************************************/
typedef struct CamerIcIeContext_s
{
    bool_t                              enabled;            /**< IE enabled */

    CamerIcIeConfig_t                   config;             /**< current configuration of IE module */
} CamerIcIeContext_t;



/*****************************************************************************/
/**
 * @brief   This function enables the clock for the CamerIC IE (Image
 *          Effects) module.
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcIeEnableClock
(
    CamerIcDrvHandle_t  handle
);



/*****************************************************************************/
/**
 * @brief   This function disables the clock for the CamerIC IE (Image
 *          effects) module.
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcIeDisableClock
(
    CamerIcDrvHandle_t  handle
);



/*****************************************************************************/
/**
 * @brief   This function initializes CamerIC IE (Image Effects) driver context.
 *
 * @return                  	Return the result of the function call.
 * @retval	RET_SUCCESS			operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcIeInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 * @brief   This function releases/frees the  CamerIC IE (Image Effects)
 *          driver context .
 *
 * @return                  	Return the result of the function call.
 * @retval	RET_SUCCESS			operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcIeRelease
(
    CamerIcDrvHandle_t handle
);



/* @} cameric_ie_drv */

#endif /* __CAMERIC_IE_DRV_H__ */

