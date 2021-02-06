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
 * @file cam_engine_subctrls.h
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup module_name Module Name
 * @{
 *
 */

#ifndef __CAM_ENGINE_SUBCTRLS_H__
#define __CAM_ENGINE_SUBCTRLS_H__

#include <ebase/types.h>
#include <ebase/dct_assert.h>
#include <common/misc.h>
#include <oslayer/oslayer.h>

#include <common/return_codes.h>

#include "cam_engine.h"
#include <cam_device/cam_device_buf_defs.h>

/*****************************************************************************/
/**
 *          CamEngineSubCtrlsSetup
 *
 * @brief   Create and Initialize all Cam-Engines Sub-Controls
 *
 * @param   pCamEngineCtx   context of the cam-engine instance
 *          mode            viewfinder mode
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeed
 * @retval  RET_WRONG_HANDLE    pCamEngine is NULL or invalid
 * @retval  RET_CANCELED        cam-instance is shutting down
 * @retval  RET_FAILURE         write to command-queue failed
 *
 *****************************************************************************/
RESULT CamEngineSubCtrlsSetup
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 *          CamEngineSubCtrlsStart
 *
 * @brief   Start all Cam-Engines Sub-Controls
 *
 * @param   pCamEngineCtx   context of the cam-engine instance
 *          mode            viewfinder mode
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeed
 * @retval  RET_WRONG_HANDLE    pCamEngine is NULL or invalid
 * @retval  RET_CANCELED        cam-instance is shutting down
 * @retval  RET_FAILURE         write to command-queue failed
 *
 *****************************************************************************/
RESULT CamEngineSubCtrlsStart
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 *          CamEngineSubCtrlsStop
 *
 * @brief   Stop all Cam-Engines Sub-Controls
 *
 * @param   pCamEngineCtx   context of the cam-engine instance
 *          mode            viewfinder mode
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeed
 * @retval  RET_WRONG_HANDLE    pCamEngine is NULL or invalid
 * @retval  RET_CANCELED        cam-instance is shutting down
 * @retval  RET_FAILURE         write to command-queue failed
 *
 *****************************************************************************/
RESULT CamEngineSubCtrlsStop
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 *          CamEngineSubCtrlsRelease
 *
 * @brief   Release all Cam-Engines Sub-Controls
 *
 * @param   pCamEngineCtx   context of the cam-engine instance
 *          mode            viewfinder mode
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeed
 * @retval  RET_WRONG_HANDLE    pCamEngine is NULL or invalid
 * @retval  RET_CANCELED        cam-instance is shutting down
 * @retval  RET_FAILURE         write to command-queue failed
 *
 *****************************************************************************/
RESULT CamEngineSubCtrlsRelease
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   TODO
 *
 *****************************************************************************/
RESULT CamEngineSubCtrlsRegisterBufferCb
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineBufferCb_t fpCallback,
    void                *pBufferCbCtx
);


/*****************************************************************************/
/**
 * @brief   TODO
 *
 *****************************************************************************/
RESULT CamEngineSubCtrlsDeRegisterBufferCb
(
    CamEngineContext_t  *pCamEngineCtx
);


/* @} module_name*/

#endif /* __CAM_ENGINE_SUBCTRLS_H__ */

