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

#ifndef __CAMERIC_ISP_COLOR_ADJUST_DRV_H__
#define __CAMERIC_ISP_COLOR_ADJUST_DRV_H__

/**
 * @file cameric_isp_color_adjust_drv.h
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
 * @defgroup cameric_isp_vs_drv CamerIc ISP COLOR_ADJUST Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_color_adjust_drv_api.h>


/******************************************************************************
 * Is the hardware color adjust module available ?
 *****************************************************************************/
#if defined(MRV_COLOR_ADJUST_VERSION)

/******************************************************************************
 * color adjust module is available.
 *****************************************************************************/
/*****************************************************************************/
/**
 * @brief   CamerIc ISPcolor adjust module
 *          internal driver context.
 *
 *****************************************************************************/
typedef struct CamerIcIspCaContext_s
{
    bool_t                          enabled;    /**< color adjust module enabled */

} CamerIcIspCaContext_t;


/*****************************************************************************/
/**
 * @brief   This function initializes CamerIc ISP COLOR_ADJUST driver context.
 *
 * @param   handle              CamerIc driver handle
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         Context successfully generated/initialized
 * @retval  RET_WRONG_HANDLE    driver handle is invalid
 * @retval  RET_OUTOFMEM        out of memory (memory allocation failed)
 *
 *****************************************************************************/
RESULT CamerIcIspCaInit
(
    CamerIcDrvHandle_t handle
);


/*****************************************************************************/
/**
 * @brief   This function releases/frees CamerIc ISP COLOR_ADJUST driver context.
 *
 * @param   handle              CamerIc driver handle
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         Context successfully deleted / Memory freed
 * @retval  RET_WRONG_HANDLE    driver handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcIspCaRelease
(
    CamerIcDrvHandle_t handle
);




#else  /* #if defined(MRV_COLOR_ADJUST_VERSION)  */

/******************************************************************************
 * color adjust module is not available.
 *****************************************************************************/
#define CamerIcIspCaInit( hnd )    ( RET_NOTSUPP )
#define CamerIcIspCaRelease( hnd ) ( RET_NOTSUPP )

#endif /* #if defined(MRV_COLOR_ADJUST_VERSION)  */

/* @} cameric_isp_vs_drv */

#endif /* __CAMERIC_ISP_COLOR_ADJUST_DRV_H__ */



