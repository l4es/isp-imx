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

/******************************************************************************
 *
 * Original author:
 * Dream Chip Technologies GmbH
 *
 * Additional work by:
 * Liu Su <su.liu@verisilicon.com>
 *
 *****************************************************************************/
/**
 * @file xom_ctrl_api.h
 *
 * @brief
 *   Definition of XOM ctrl API.
 *
 *****************************************************************************/


#ifndef __XOM_CTRL_API_H__
#define __XOM_CTRL_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <ebase/types.h>
#include <common/return_codes.h>
#include <common/picture_buffer.h>

#include <bufferpool/media_buffer.h>

#include <hal/hal_api.h>

#include "xom_ctrl_common.h"

typedef struct xomCtrlConfig_s
{
    uint32_t                MaxPendingCommands; //!< Number of commands that can be queued and thus be pending at a time.
    uint32_t                MaxBuffers;
    xomCtrlCompletionCb_t   xomCbCompletion;    //!< Callback function for command completion.
    void                    *pUserContext;      //!< User context passed on to completion callback.
    HalHandle_t             HalHandle;          //!< HAL session to use for HW access

    xomCtrlHandle_t         xomCtrlHandle;      //!< Handle to created xom context, set by @ref xomCtrlInit if successfull, undefined otherwise.
} xomCtrlConfig_t;

extern RESULT xomCtrlInit
(
    xomCtrlConfig_t *pConfig            //!< Reference to configuration structure.
);

extern RESULT xomCtrlShutDown
(
    xomCtrlHandle_t xomCtrlHandle       //!< Handle to xom context as returned by @ref xomCtrlInit.
);

extern RESULT xomCtrlStart
(
    xomCtrlHandle_t     xomCtrlHandle,  //!< Handle to xom context as returned by @ref xomCtrlInit.
    xomCtrlBufferCb_t   xomBufferCb,    //!< External buffer callback
    void                *pContext,//!< Sample context passed on to sample callback.
    uint8_t             FrameSkip      //!< Skip consecutive samples
);

extern RESULT xomCtrlStop
(
    xomCtrlHandle_t xomCtrlHandle       //!< Handle to xom context as returned by @ref xomCtrlInit.
);

extern RESULT xomCtrlPause
(
    xomCtrlHandle_t xomCtrlHandle       //!< Handle to xom context as returned by @ref xomCtrlInit.
);

extern RESULT xomCtrlResume
(
    xomCtrlHandle_t xomCtrlHandle       //!< Handle to xom context as returned by @ref xomCtrlInit.
);

extern RESULT  xomCtrlShowBuffer
(
    xomCtrlHandle_t         xomCtrlHandle,
    MediaBuffer_t           *pBuffer
);

/* @} xom_ctrl_api */

#ifdef __cplusplus
}
#endif

#endif /* __XOM_CTRL_API_H__ */
