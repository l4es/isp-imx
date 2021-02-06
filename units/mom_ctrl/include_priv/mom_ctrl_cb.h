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
 * @mom_ctrl.h
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
#ifndef __MOM_CTRL_CB_H__
#define __MOM_CTRL_CB_H__

#include <ebase/types.h>
#include <oslayer/oslayer.h>

#include <common/return_codes.h>


/*****************************************************************************/
/**
 * 			MomCtrlPictureBufferPoolNotifyCbMainPath
 *
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
void MomCtrlPictureBufferPoolNotifyCbMainPath
(
    MediaBufQueueExEvent_t  event,
    void                    *pUserContext,
    const MediaBuffer_t     *pMediaBuffer
);


/*****************************************************************************/
/**
 * 			MomCtrlPictureBufferPoolNotifyCbSelfPath
 *
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
void MomCtrlPictureBufferPoolNotifyCbSelfPath
(
    MediaBufQueueExEvent_t  event,
    void                    *pUserContext,
    const MediaBuffer_t     *pMediaBuffer
);


void MomCtrlPictureBufferPoolNotifyCbSelfPath2
(
    MediaBufQueueExEvent_t  event,
    void                    *pUserContext,
    const MediaBuffer_t     *pMediaBuffer
);



void MomCtrlPictureBufferPoolNotifyCbRdiPath
(
    MediaBufQueueExEvent_t  event,
    void                    *pUserContext,
    const MediaBuffer_t     *pMediaBuffer
);



void MomCtrlPictureBufferPoolNotifyCbMetaPath
(
    MediaBufQueueExEvent_t  event,
    void                    *pUserContext,
    const MediaBuffer_t     *pMediaBuffer
);


/*****************************************************************************/
/**
 *			MomCtrlCamerIcDrvEventCb
 *
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
void MomCtrlCamerIcDrvEventCb
(
    const uint32_t event,
    void *param,
    void *pUserContext
);



/*****************************************************************************/
/**
 *			MomCtrlCamerIcDrvRequestCb
 *
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT MomCtrlCamerIcDrvRequestCb
(
    const uint32_t request,
    void **param,
    void *pUserContext
);


/* @} module_name*/

#endif /* __MOM_CTRL_CB_H__ */
