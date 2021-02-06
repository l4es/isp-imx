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

#ifndef __CAMERIC_ISP_GREEN_EQUILIBRATION_DRV_H__
#define __CAMERIC_ISP_GREEN_EQUILIBRATION_DRV_H__

/**
 * @file cameric_isp_green_equilibration_drv.h
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
 * @defgroup cameric_isp_vs_drv CamerIc ISP GREEN_EQUILIBRATION Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_green_equilibration_drv_api.h>


/******************************************************************************
 * Is the hardware green equilibration module available ?
 *****************************************************************************/
#if defined(MRV_GREENEQUILIBRATION_VERSION)

/******************************************************************************
 * green equilibration module is available.
 *****************************************************************************/
/*****************************************************************************/
/**
 * @brief   CamerIc ISPgreen equilibration module
 *          internal driver context.
 *
 *****************************************************************************/
typedef struct CamerIcIspGreenEqlrContext_s
{
    bool_t                          enabled;    /**< green equilibration module enabled */

} CamerIcIspGreenEqlrContext_t;


/*****************************************************************************/
/**
 * @brief   This function initializes CamerIc ISP GREEN_EQUILIBRATION driver context.
 *
 * @param   handle              CamerIc driver handle
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         Context successfully generated/initialized
 * @retval  RET_WRONG_HANDLE    driver handle is invalid
 * @retval  RET_OUTOFMEM        out of memory (memory allocation failed)
 *
 *****************************************************************************/
RESULT CamerIcIspGreenEqlrInit
(
    CamerIcDrvHandle_t handle
);


/*****************************************************************************/
/**
 * @brief   This function releases/frees CamerIc ISP GREEN_EQUILIBRATION driver context.
 *
 * @param   handle              CamerIc driver handle
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         Context successfully deleted / Memory freed
 * @retval  RET_WRONG_HANDLE    driver handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcIspGreenEqlrRelease
(
    CamerIcDrvHandle_t handle
);




#else  /* #if defined(MRV_GREEN_EQUILIBRATION_VERSION)  */

/******************************************************************************
 * green equilibration module is not available.
 *****************************************************************************/
#define CamerIcIspGreenEqlrInit( hnd )    ( RET_NOTSUPP )
#define CamerIcIspGreenEqlrRelease( hnd ) ( RET_NOTSUPP )

#endif /* #if defined(MRV_GREEN_EQUILIBRATION_VERSION)  */

/* @} cameric_isp_vs_drv */

#endif /* __CAMERIC_ISP_GREEN_EQUILIBRATION_DRV_H__ */


