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
 * @file cim_ctrl.h
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
#ifndef __CIM_CTRL_H__
#define __CIM_CTRL_H__

#include <ebase/types.h>
#include <oslayer/oslayer.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include "cim_ctrl_common.h"

/**
 * @brief   Internal states of the cim control.
 *
 */
typedef enum CimCtrlState_e
{
    eCimCtrlStateInvalid    = 0x0000,   /**< FSM state is invalid since cim instance does not exist. */
    eCimCtrlStateInitialize	= 0x0001,   /**< FSM is in state initialized. */
    eCimCtrlStateRunning	= 0x0002,   /**< FSM is in state running. */
    eCimCtrlStateStopped	= 0x0003,   /**< FSM is in state stopped. */
} CimCtrlState_t;


/**
 * @brief
 *
 * @note
 *
 */
typedef struct CimCtrlContext_s
{
	CimCtrlState_t          State;

    uint32_t                MaxCommands;
    uint32_t                MaxBuffers;

    CimCtrlCompletionCb_t   cimCbCompletion;
    void                    *pUserContext;

	osQueue                 CommandQueue;
    osThread	            Thread;

    HalHandle_t             HalHandle;
} CimCtrlContext_t;



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
RESULT CimCtrlCreate
(
    CimCtrlContext_t  *pCimCtrlCtx
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
RESULT CimCtrlDestroy
(
    CimCtrlContext_t *pCimContext
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
int32_t CimCtrlThreadHandler
(
	void *p_arg
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
RESULT CimCtrlSendCommand
(
    CimCtrlContext_t    *pCimCtrlCtx,
    CimCtrlCmdId_t      Command
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
void CimCtrlCompleteCommand
(
    CimCtrlContext_t    *pCimCtrlCtx,
    CimCtrlCmdId_t      Command,
    RESULT              result
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
static inline CimCtrlState_t CimCtrlGetState
(
    CimCtrlContext_t *pCimCtrlCtx
)
{
    return ( pCimCtrlCtx->State );
}



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
static inline void CimCtrlSetState
(
    CimCtrlContext_t        *pCimCtrlCtx,
    const CimCtrlState_t    newState
)
{
    pCimCtrlCtx->State = newState;
}



/* @} module_name*/

#endif /* __CIM_CTRL_H__ */

