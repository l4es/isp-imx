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
 * @defgroup cameric_isp_lsc_drv CamerIc ISP LSC Driver Internal API
 * @{
 *
 */

#ifndef __CAMERIC_ISP_LSC_DRV_H__
#define __CAMERIC_ISP_LSC_DRV_H__

#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_lsc_drv_api.h>


/******************************************************************************
 * Is the hardware LSC module available ?
 *****************************************************************************/
#if defined(MRV_LSC_VERSION)

/******************************************************************************
 * LSC module is available.
 *****************************************************************************/

/*****************************************************************************/
/**
 * @brief Internal context of the LSC Module.
 *
 */
typedef struct CamerIcIspLscContext_s
{
    bool_t      enabled;
} CamerIcIspLscContext_t;



/*****************************************************************************/
/**
 * @brief   Initialize CamerIc ISP LSC driver context.
 *
 * @param   handle          CamerIc driver handle.
 *
 * @return                  Return the result of the function call.
 * @retval                  RET_SUCCESS
 * @retval                  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspLscInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 * @brief   Release/Free CamerIc ISP LSC driver context.
 *
 * @param   handle          CamerIc driver handle.
 *
 * @return                  Return the result of the function call.
 * @retval                  RET_SUCCESS
 * @retval                  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspLscRelease
(
    CamerIcDrvHandle_t handle
);

#else  /* #if defined(MRV_LSC_VERSION) */

/******************************************************************************
 * LSC module is not available.
 *****************************************************************************/

#define CamerIcIspLscInit( hnd )        ( RET_NOTSUPP )
#define CamerIcIspLscRelease( hnd )     ( RET_NOTSUPP )

#endif /* #if defined(MRV_LSC_VERSION) */

/* @} cameric_isp_lsc_drv */

#endif /* __CAMERIC_ISP_LSC_DRV_H__ */
