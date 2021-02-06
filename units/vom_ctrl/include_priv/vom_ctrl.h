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
 * @vom_ctrl.h
 *
 * @brief
 *   Internal stuff used by vom ctrl implementation.
 *
 *****************************************************************************/
/**
 * @page vom_ctrl_page VOM Ctrl
 * The Video Output Module displays image buffers handed in via QuadMVDU_FX on
 * a connected HDMI display device.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref vom_ctrl_api
 * - @ref vom_ctrl_common
 * - @ref vom_ctrl
 * - @ref vom_ctrl_mvdu
 *
 * @defgroup vom_ctrl VOM Ctrl
 * @{
 *
 */


#ifndef __VOM_CTRL_H__
#define __VOM_CTRL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <ebase/types.h>
#include <common/return_codes.h>
#include <common/cea_861.h>
#include <common/hdmi_3d.h>

#include <oslayer/oslayer.h>
#include <bufferpool/media_buffer_queue_ex.h>

#include <hal/hal_api.h>

#include "vom_ctrl_common.h"
#include "vom_ctrl_mvdu.h"

/**
 * @brief   Internal states of the vom control.
 *
 */
typedef enum
{
    eVomCtrlStateInvalid      = 0,  //!< FSM state is invalid since VOM instance does not exist, is currently being created or is currently being shutdown.
    eVomCtrlStateIdle,              //!< FSM is in state Idle.
    eVomCtrlStateRunning            //!< FSM is in state Running.
} vomCtrlState_t;


/**
 * @brief   Context of vom control instance. Holds all information required for operation.
 *
 * @note
 *
 */
typedef struct vomCtrlContext_s
{
    vomCtrlState_t                      State;              //!< Holds internal state.

    uint32_t                            MaxCommands;        //!< Max number of commands that can be queued.
    MediaBufQueueEx_t                   *pInputQueue;       //!< Reference to buffer input queue.
    volatile bool_t                     InputQueueHighWM;   //!< Buffer input queue high watermark state var.
    vomCtrlCompletionCb_t               vomCbCompletion;    //!< Buffer completion callback.
    void                                *pUserContext;      //!< User context to pass in to buffer completion callback.
    const Cea861VideoFormatDetails_t    *pVideoFormat;      //!< Output video format to use.
    bool_t                              Enable3D;           //!< Enable 3D display mode.
    Hdmi3DVideoFormat_t                 VideoFormat3D;      //!< The HDMI 3D display mode to use; undefined if 3D not enabled.
    HalHandle_t                         HalHandle;          //!< HAL session to use for HW access.
    vomCtrlMvduHandle_t                 MvduHandle;         //!< Handle to video output unit.
    osQueue                             CommandQueue;       //!< Command queue.
    osThread                            Thread;             //!< Command processing thread.
    osQueue                     		FullBufQueue;
    uint32_t                            MaxBuffers;
} vomCtrlContext_t;



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
extern RESULT vomCtrlCreate
(
    vomCtrlContext_t    *pVomContext
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
extern RESULT vomCtrlDestroy
(
    vomCtrlContext_t    *pVomContext
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
extern RESULT vomCtrlSendCommand
(
    vomCtrlContext_t        *pVomContext,
    const vomCtrlCmdId_t    Command
);


/* @} vom_ctrl */

#ifdef __cplusplus
}
#endif

#endif /* __VOM_CTRL_H__ */
