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

#ifndef __CAMERIC_JPE_DRV_H__
#define __CAMERIC_JPE_DRV_H__

/**
 * @file cameric_jpe_drv.h
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
 * @defgroup cameric_jpe_drv CamerIc JPE Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_jpe_drv_api.h>



typedef enum CamerIcJpeState_e
{
    CAMERIC_JPE_STATE_INVALID           = 0,
    CAMERIC_JPE_STATE_INIT              = 1,
    CAMERIC_JPE_STATE_CONFIGURED        = 2,
    CAMERIC_JPE_STATE_GEN_HEADER        = 3,
    CAMERIC_JPE_STATE_ENC_DATA          = 4,
    CAMERIC_JPE_STATE_MAX
} CamerIcJpeState_t;



/******************************************************************************/
/**
 * @brief driver internal context of CamerIC JPE module.
 *
 *****************************************************************************/
typedef struct CamerIcJpeContext_s
{
    bool_t                          enabled;

    CamerIcRequestCb_t              RequestCb;              /**< request callback */
	CamerIcEventCb_t        	    EventCb;				/**< event callback */

    HalIrqCtx_t                     HalStatusIrqCtx;        /**< status irq ctx */
    HalIrqCtx_t                     HalErrorIrqCtx;         /**< error irq ctx */

    CamerIcJpeHeader_t              hdrType;
    CamerIcJpeState_t               encState;

    CamerIcJpeConfig_t              config;                 /**< configuration */
} CamerIcJpeContext_t;



/*****************************************************************************/
/**
 * @brief   This function enables the clock for the CamerIC JPE module.
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcJpeEnableClock
(
    CamerIcDrvHandle_t  handle
);



/*****************************************************************************/
/**
 * @brief   This function disables the clock for the CamerIC JPE module.
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcJpeDisableClock
(
    CamerIcDrvHandle_t  handle
);



/*****************************************************************************/
/**
 * @brief   This function initializes CamerIC JPE (jpeg-encode) driver context.
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                  	Return the result of the function call.
 * @retval	RET_SUCCESS			operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcJpeInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 * @brief   This function releases/frees CamerIC JPE (jpeg-encode) driver
 *          context .
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                  	Return the result of the function call.
 * @retval	RET_SUCCESS			operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcJpeRelease
(
    CamerIcDrvHandle_t handle
);


/*****************************************************************************/
/**
 * @brief   This function initializes and starts interrupt handling of JPE
 *          driver module.
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                  	Return the result of the function call.
 * @retval	RET_SUCCESS			operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcJpeStart
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 * @brief  	This function stops and releases interrupt handling of JPE
 *          driver module.
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                  	Return the result of the function call.
 * @retval	RET_SUCCESS			operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcJpeStop
(
    CamerIcDrvHandle_t handle
);


/* @} cameric_jpe_drv */

#endif /* __CAMERIC_JPE_DRV_H__ */

