/******************************************************************************
 *
 * Copyright 2018, VeriSilicon Technologies Co, Ltd. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * VeriSilicon Microelectronics(Shanghai)Co., LTd, 20F.No.560, Songtao Road
 * Pudong New Area Shanghai 201203 P.R.China
 *
 *****************************************************************************/

#ifndef __CAMERIC_ISP_WDR3_DRV_H__
#define __CAMERIC_ISP_WDR3_DRV_H__

/**
 * @file cameric_isp_wdr3_drv.h
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
 * @defgroup cameric_isp_wdr3_drv CamerIc ISP WDR3 Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_wdr3_drv_api.h>




/*******************************************************************************
 *
 *          CamerIcIspWdr3Context_t
 *
 * @brief
 *
 */
typedef struct CamerIcIspWdr3Context_s
{
    bool_t      enabled;
    bool_t      cfg_update;

    uint8_t		strength;
    uint8_t		globalStrength;
    uint8_t		maxGain;

} CamerIcIspWdr3Context_t;



/*****************************************************************************/
/**
 *          CamerIcIspWdr3Init()
 *
 * @brief   Initialize CamerIc ISP WDR driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspWdr3Init
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspWdr3Release()
 *
 * @brief   Release/Free CamerIc ISP WDR driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspWdr3Release
(
    CamerIcDrvHandle_t handle
);



/* @} cameric_isp_wdr3_drv */

#endif /* __CAMERIC_ISP_WDR3_DRV_H__ */

