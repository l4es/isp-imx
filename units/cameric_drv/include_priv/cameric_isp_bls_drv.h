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

#ifndef __CAMERIC_ISP_BLS_DRV_H__
#define __CAMERIC_ISP_BLS_DRV_H__

/**
 * @file cameric_isp_bls_drv.h
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
 * @defgroup cameric_isp_bls_drv CamerIc ISP BLS Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_bls_drv_api.h>



/*******************************************************************************
 *
 *          CamerIcIspBlsContext_t
 *
 * @brief
 *
 */
typedef struct CamerIcIspBlsContext_s
{
    bool_t                  enabled;

    uint16_t                isp_bls_a_fixed;    /**< black-level-substraction value */
    uint16_t                isp_bls_b_fixed;    /**< black-level-substraction value */
    uint16_t                isp_bls_c_fixed;    /**< black-level-substraction value */
    uint16_t                isp_bls_d_fixed;    /**< black-level-substraction value */

    CamerIcWindow_t         Window1;            /**< measuring window 1 */
    CamerIcWindow_t         Window2;            /**< measuring window 2 */
} CamerIcIspBlsContext_t;



/*****************************************************************************/
/**
 *          CamerIcIspBlsInit()
 *
 * @brief   Initialize CamerIc ISP BLS driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspBlsInit
(
    CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
 *          CamerIcIspBlsRelease()
 *
 * @brief   Release/Free CamerIc ISP BLS driver context
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcIspBlsRelease
(
    CamerIcDrvHandle_t handle
);

/* @} cameric_isp_bls_drv */

#endif /* __CAMERIC_ISP_BLS_DRV_H__ */
