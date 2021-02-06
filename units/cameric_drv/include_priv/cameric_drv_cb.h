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

#ifndef __CAMERIC_DRV_CB_H__
#define __CAMERIC_DRV_CB_H__

/**
 * @file cameric_drv.h
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
 * @defgroup cameric_drv_cb CamerIc Driver Internal Callback
 * @{
 *
 */

#include <ebase/types.h>

#include <cameric_drv/cameric_drv_api.h>



/******************************************************************************/
/**
 *          CamerIcRequestCb_t
 *
 *  @brief  Request callback structure
 *
 *  This callback is used to request something from the application software,
 *  e.g. an input or output buffer.
 *
 *  @return RESULT
 *
 *****************************************************************************/
typedef struct CamerIcRequestCb_s
{
    CamerIcRequestFunc_t    func;           /**< pointer to callback function */
    void                    *pUserContext;  /**< user context */
} CamerIcRequestCb_t;



/******************************************************************************/
/**
 *          CamerIcEventCb_t
 *
 *  @brief  Event callback
 *
 *  This callback is used to signal something to the application software,
 *  e.g. an error or an information.
 *
 *  @return void
 *
 *****************************************************************************/
typedef struct CamerIcEventCb_s
{
    CamerIcEventFunc_t  func;           /**< pointer to callback function */
    void                *pUserContext;  /**< user context */
} CamerIcEventCb_t;


/* @} cameric_drv_cb */

#endif /* __CAMERIC_DRV_CB_H__ */

