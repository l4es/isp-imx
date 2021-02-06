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

#ifndef __CAMERIC_ISP_EXP_DRV_H__
#define __CAMERIC_ISP_EXP_DRV_H__

/**
 * @file cameric_isp_exp_drv.h
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
 * @defgroup cameric_isp_exp_drv CamerIc ISP EXP Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_exp_drv_api.h>



/*******************************************************************************
 *
 *          CamerIcIspExpContext_t
 *
 * @brief
 *
 */
typedef struct CamerIcIspExpContext_s
{
    bool_t                          enabled;    /**< measuring enabled */
    bool_t                          autostop;   /**< stop measuring after a complete frame */
    CamerIcIspExpMeasuringMode_t    mode;       /**< measuring mode */

    CamerIcEventCb_t                EventCb;

    CamerIcWindow_t                 Window;     /**< measuring window */
    CamerIcWindow_t                 Grid;       /**< measuring window */

    CamerIcMeanLuma_t               Luma;
} CamerIcIspExpContext_t;



/*****************************************************************************/
/**
 *          CamerIcIspExpInit()
 *
 * @brief   Initialize CamerIc ISP EXPOSURE driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspExpInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspExpRelease()
 *
 * @brief   Release/Free CamerIc ISP EXPOSURE driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspExpRelease
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspExpSignal()
 *
 * @brief
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
void CamerIcIspExpSignal
(
    CamerIcDrvHandle_t handle
);



/* @} cameric_isp_exp_drv */

#endif /* __CAMERIC_ISP_WDR_DRV_H__ */

