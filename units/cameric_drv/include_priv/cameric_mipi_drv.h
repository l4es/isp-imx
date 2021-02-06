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
 * @file cameric_mipi_drv.h
 *
 * @brief
 *  Internal interface of MIPI module.
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 *
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup cameric_mipi_drv CamerIc MIPI Driver Internal API
 * @{
 *
 */

#ifndef __CAMERIC_MIPI_DRV_H__
#define __CAMERIC_MIPI_DRV_H__

#include <ebase/types.h>

#include <common/align.h>
#include <common/list.h>
#include <common/mipi.h>
#include <common/return_codes.h>

#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_mipi_drv_api.h>



/******************************************************************************/
/**
 * @brief driver internal context of MIPI Module.
 *
 *****************************************************************************/
typedef struct CamerIcMipiContext_s
{
    int                         fd;                     /**< file description */
    HalIrqCtx_t                 HalIrqCtx;              /**< interrupt context */

    uint32_t                    no_lanes;               /**< number of used lanes */
    MipiVirtualChannel_t        virtual_channel;        /**< current virtual channel id */
    MipiDataType_t              data_type;              /**< curreent data type  */

    bool_t                      compression_enabled;    /**< compression enabled */
    MipiDataCompressionScheme_t compression_scheme;     /**< currently used compression sheme */
    MipiPredictorBlock_t        predictor_block;        /**< currently used predictor block */

    CamerIcEventCb_t            EventCb;                /**< event callback */

    bool_t                      enabled;                /**< mipi module currently enabled */
} CamerIcMipiContext_t;



/*****************************************************************************/
/**
 * @brief   This function enables the clock for the CamerIC MIPI module.
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcMipiEnableClock
(
    CamerIcDrvHandle_t  handle
);



/*****************************************************************************/
/**
 * @brief   This function disables the clock for the CamerIC MIPI module.
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcMipiDisableClock
(
    CamerIcDrvHandle_t  handle
);



/*****************************************************************************/
/**
 * @brief   This function initializes the  CamerIc MIPI driver context.
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcMipiInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 * @brief   This function releases/frees the  CamerIc MIPI driver context.
 *
 * @param   handle              CamerIc driver handle.
 *
 * @return                      Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeded
 * @retval  RET_WRONG_HANDLE    handle is invalid
 *
 *****************************************************************************/
RESULT CamerIcMipiRelease
(
    CamerIcDrvHandle_t handle
);

RESULT CamerIcMipiStart
(
    CamerIcDrvHandle_t handle
);

RESULT CamerIcMipiStop
(
    CamerIcDrvHandle_t handle
);

/* @} cameric_mipi_drv */

#endif /* __CAMERIC_MIPI_DRV_H__ */

