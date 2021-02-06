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

#ifndef __CAMERIC_ISP_WDR3_DRV_API_H__
#define __CAMERIC_ISP_WDR3_DRV_API_H__

/**
 * @file    cameric_isp_wdr3_drv_api.h
 *
 * @brief   This file contains the CamerIC ISP WDR driver API definitions.
 *
 *****************************************************************************/
/**
 * @cond cameric_isp_wdr3
 *
 * @defgroup cameric_isp_wdr3_drv_api CamerIC ISP WDR3 Driver API definitions
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct CamerIcIspWdr3Config_s {
    uint8_t               strength;			/**< strength */
    uint8_t               globalStrength;	/**< globalStrength */
    uint8_t               maxGain;			/**< maxGain */
} CamerIcIspWdr3Config_t;


/*****************************************************************************/
/**
 *          CamerIcIspWdr3Enable()
 *
 * @brief   Enable WDR3 Module
 *
 * @param   handle          CamerIc driver handle
 *
 * @return                  Return the result of the function call.
 * @retval                  RET_SUCCESS
 * @retval                  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspWdr3Enable
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspWdr3Disable()
 *
 * @brief   Disable WDR3 Module
 *
 * @param   handle          CamerIc driver handle
 *
 * @return                  Return the result of the function call.
 * @retval                  RET_SUCCESS
 * @retval                  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspWdr3Disable
(
    CamerIcDrvHandle_t handle
);


/*****************************************************************************/
/**
*          CamerIcIspWdr3Config()
*
* @brief   Config WDR3 Module
*
* @param   handle          CamerIc driver handle
*
* @return                  Return the result of the function call.
* @retval                  RET_SUCCESS
* @retval                  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIspWdr3Config
(
	CamerIcDrvHandle_t 		handle,
    CamerIcIspWdr3Config_t  *pWdr3Cfg
);


#ifdef __cplusplus
}
#endif



#endif /* __CAMERIC_ISP_WDR3_DRV_API_H__ */

