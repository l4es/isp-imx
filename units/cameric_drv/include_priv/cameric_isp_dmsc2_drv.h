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

/**
 * @file cameric_isp_lsc_drv.h
 *
 * @brief
 *  Internal interface of LSC Module.
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup cameric_isp_lsc_drv CamerIc ISP DMSC Driver Internal API
 * @{
 *
 */

#ifndef __CAMERIC_ISP_DMSC2_DRV_H__
#define __CAMERIC_ISP_DMSC2_DRV_H__

#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_dmsc2_drv_api.h>

/**
 * @brief Internal context of the DMSC Module.
 *
 */


/*****************************************************************************/
/**
 * @brief   Initialize CamerIc ISP DMSC driver context.
 *
 * @param   handle          CamerIc driver handle.
 *
 * @return                  Return the result of the function call.
 * @retval                  RET_SUCCESS
 * @retval                  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspDmscInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 * @brief   Release/Free CamerIc ISP DMSC driver context.
 *
 * @param   handle          CamerIc driver handle.
 *
 * @return                  Return the result of the function call.
 * @retval                  RET_SUCCESS
 * @retval                  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspDmscRelease
(
    CamerIcDrvHandle_t handle
);


/* @} cameric_isp_lsc_drv */

#endif /* __CAMERIC_ISP_DMSC_DRV_H__ */

