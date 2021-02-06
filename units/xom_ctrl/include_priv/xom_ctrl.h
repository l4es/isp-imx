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
 * @xom_ctrl.h
 *
 * @brief
 *   Internal stuff used by XOM ctrl implementation.
 *
 *****************************************************************************/


#ifndef __XOM_CTRL_H__
#define __XOM_CTRL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <ebase/types.h>
#include <common/return_codes.h>
#include <common/cea_861.h>
#include <common/picture_buffer.h>

#include <oslayer/oslayer.h>

#include <hal/hal_api.h>

#include "xom_ctrl_common.h"

/**
 * @brief   Internal states of the xom control.
 *
 */
typedef enum
{
    eXomCtrlStateInvalid      = 0,  //!< FSM state is invalid since XOM instance does not exist, is currently being created or is currently being shutdown.
    eXomCtrlStateIdle,              //!< FSM is in state Idle.
    eXomCtrlStatePaused,            //!< FSM is in state Paused.
    eXomCtrlStateRunning            //!< FSM is in state Running.
} xomCtrlState_t;

/**
 * @brief   Buffer type & layout dependent buffer handling.
 *
 * @note
 *
 */
typedef RESULT (* xomCtrlMapBuffer_t)
(
    struct xomCtrlContext_s    *pXomContext,
    PicBufMetaData_t           *pPicBufMetaData
);

typedef RESULT (* xomCtrlUnMapBuffer_t)
(
    struct xomCtrlContext_s    *pXomContext
);

/**
 * @brief   Context of xom control instance. Holds all information required for operation.
 *
 * @note
 *
 */
typedef struct xomCtrlContext_s
{
    xomCtrlState_t             State;              //!< Holds internal state.

    uint32_t                   MaxCommands;        //!< Max number of commands that can be queued.
    uint32_t                   MaxBuffers;
    xomCtrlCompletionCb_t      xomCbCompletion;    //!< Buffer completion callback.
    void                       *pUserContext;      //!< User context to pass in to buffer completion callback.
    HalHandle_t                HalHandle;          //!< HAL session to use for HW access.

    osQueue                    CommandQueue;       //!< Command queue; holds elements of type @ref xomCtrlCmd_t.
    osThread                   Thread;             //!< Command processing thread.

    osQueue                    FullBufQueue;

    xomCtrlBufferCb_t          xomBufferCb;        //!< External buffer callback.
    void                       *pContext;    //!< Sample context passed on to sample callback.
    uint8_t                    FrameSkip;         //!< Skip consecutive samples.

    uint8_t                    FrameIdx;          //!< Sample index.

    PicBufMetaData_t           MappedMetaData;     //!< Media buffer meta data descriptor for mapped buffer.
    xomCtrlMapBuffer_t         MapBuffer;          //!< Suitable handling function for type & layout of buffer.
    xomCtrlUnMapBuffer_t       UnMapBuffer;        //!< Suitable handling function for type & layout of buffer.
} xomCtrlContext_t;



/*****************************************************************************/
/**
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
extern RESULT xomCtrlCreate
(
    xomCtrlContext_t    *pXomContext
);


/*****************************************************************************/
/**
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
extern RESULT xomCtrlDestroy
(
    xomCtrlContext_t    *pXomContext
);


/*****************************************************************************/
/**
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
extern RESULT xomCtrlSendCommand
(
    xomCtrlContext_t    *pXomContext,
    xomCtrlCmd_t        *pCommand
);


/* @} xom_ctrl */

#ifdef __cplusplus
}
#endif

#endif /* __XOM_CTRL_H__ */
