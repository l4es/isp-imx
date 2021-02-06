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

#ifndef __CAMERIC_ISP_HIST_DRV_H__
#define __CAMERIC_ISP_HIST_DRV_H__

/**
 * @file cameric_isp_hist_drv.h
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
 * @defgroup cameric_isp_hist_drv CamerIc ISP HIST Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_hist_drv_api.h>



#ifdef __cplusplus
extern "C"
{
#endif



/*******************************************************************************
 *
 *          CamerIcIspHistContext_t
 *
 * @brief
 *
 */
typedef struct CamerIcIspHistContext_s
{
    bool_t                  enabled;                            /**< measuring enabled */
    CamerIcIspHistMode_t    mode;                               /**< histogram mode */
    uint16_t                StepSize;                           /**< stepsize calculated from measuirng window */

    CamerIcEventCb_t        EventCb;

    CamerIcWindow_t         Window;                             /**< measuring window */
    CamerIcWindow_t         Grid;                               /**< measuring window */
    CamerIcHistWeights_t    Weights;
    CamerIcHistBins_t       Bins;
} CamerIcIspHistContext_t;



/*****************************************************************************/
/**
 *          CamerIcIspHistInit()
 *
 * @brief   Initialize CamerIc ISP HIST driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspHistInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspHistRelease()
 *
 * @brief   Release/Free CamerIc ISP HIST driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspHistRelease
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspHistSignal()
 *
 * @brief
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
void CamerIcIspHistSignal
(
    CamerIcDrvHandle_t handle
);


#ifdef __cplusplus
}
#endif



/* @} cameric_isp_hist_drv */

#endif /* __CAMERIC_ISP_HIST_DRV_H__ */

