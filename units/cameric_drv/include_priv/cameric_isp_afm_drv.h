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

#ifndef __CAMERIC_ISP_AFM_DRV_H__
#define __CAMERIC_ISP_AFM_DRV_H__

/**
 * @file cameric_isp_afm_drv.h
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
 * @defgroup cameric_isp_afm_drv CamerIc ISP AFM Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_afm_drv_api.h>


typedef enum CamerIcIspAfmSignal_e
{
    CAMERIC_ISP_AFM_SIGNAL_INVALID          = 0,
    CAMERIC_ISP_AFM_SIGNAL_MEASURMENT       = 1,
    CAMERIC_ISP_AFM_SIGNAL_LUMA_OVERFLOW    = 2,
    CAMERIC_ISP_AFM_SIGNAL_SUM_OVERFLOW     = 3
} CamerIcIspAfmSignal_t;



/*******************************************************************************
 *
 *          CamerIcIspAfmContext_t
 *
 * @brief
 *
 */
typedef struct CamerIcIspAfmContext_s
{
    CamerIcEventCb_t                EventCb;
	CamerIcAfmMeasuringResult_t     MeasResult;
} CamerIcIspAfmContext_t;



/*****************************************************************************/
/**
 *          CamerIcIspAfmInit()
 *
 * @brief   Initialize CamerIc ISP Auto focus module driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspAfmInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspAfmRelease()
 *
 * @brief   Initialize CamerIc ISP Auto focus module driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspAfmRelease
(
    CamerIcDrvHandle_t handle
);

/*****************************************************************************/
/**
 *          CamerIcIspAfmSignal()
 *
 * @brief
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
void CamerIcIspAfmSignal
(
    const CamerIcIspAfmSignal_t signal,
    CamerIcDrvHandle_t          handle
);



/* @} cameric_isp_afm_drv */

#endif /* __CAMERIC_ISP_AFM_DRV_H__ */

