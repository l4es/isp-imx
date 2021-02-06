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
 * @dom_ctrl.h
 *
 * @brief
 *   Internal stuff used by dom ctrl implementation.
 *
 *****************************************************************************/
/**
 * @page dom_ctrl_page DOM Ctrl
 * The Display Output Module displays image buffers in an X11 window.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref dom_ctrl_api
 * - @ref dom_ctrl_common
 * - @ref dom_ctrl
 *
 * @defgroup dom_ctrl DOM Ctrl
 * @{
 *
 */


#ifndef __DOM_CTRL_H__
#define __DOM_CTRL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "dom_ctrl_common.h"
#include "dom_ctrl_api.h"
#include "dom_ctrl_vidplay_api.h"
#include "list.h"
#include <appshell_ibd/ibd_api.h>


//Copy from units. TODO: align to cam_device_api

#define ALIGN_UP(addr, align)		( ((addr) + ((align)-1)) & ~(uintptr_t)((align)-1) ) //!< Aligns addr to next higher aligned addr; align must be a power of two.
#define ALIGN_DOWN(addr, align)		( ((addr)              ) & ~(uintptr_t)((align)-1) ) //!< Aligns addr to next lower aligned addr; align must be a power of two.

#define ALIGN_SIZE_1K               ( 0x400 )
#define ALIGN_UP_1K(addr)			( ALIGN_UP(addr, ALIGN_SIZE_1K) )

#define MAX_ALIGNED_SIZE(size, align) ( ALIGN_UP(size, align) + align ) //!< Calcs max size of memory required to be able to hold a block of size bytes with a start address aligned to align.

/**
 * @brief   Internal states of the dom control.
 *
 */
typedef enum
{
    edomCtrlStateInvalid      = 0,  //!< FSM state is invalid since DOM instance does not exist, is currently being created or is currently being shutdown.
    edomCtrlStateIdle,              //!< FSM is in state Idle.
    edomCtrlStateRunning            //!< FSM is in state Running.
} domCtrlState_t;


/**
 * @brief   Context of dom control instance. Holds all information required for operation.
 *
 * @note
 *
 */
typedef struct domCtrlContext_s
{
    int                         domId;
    domCtrlState_t              State;              //!< Holds internal state.

    domCtrlConfig_t             Config;             //!< Local copy of config data.

    osQueue                     CommandQueue;       //!< Command queue; holds elements of type @ref domCtrlCmdId_t.
    osThread                    Thread;             //!< Command processing thread.
    domCtrlVidplayHandle_t      hDomCtrlVidplay;    //!< Handle of graphic subsystem dependent video player.

    osQueue                     FullBufQueue;

    bool                        InputQueueHighWM;   //!< Holds whether high watermark state is active.
    void*                       pCurDisplayBuffer;  //!< Holds currently displayed buffer until either a new one is displayed or the display is cleared.

    List                        *pDrawContextList;
    osMutex                     drawMutex;

    domImagePresentation_t      ImgPresent;

#ifdef DOM_FPS
    int32_t                     FpsStartTimeMs;     //!< Start time for FPS measurement cycle.
    uint32_t                    FpsDisplayed;       //!< Number of frames displayed in this cycle.
    uint32_t                    FpsSkipped;         //!< Number of frames skipped in this cycle.
#endif // DOM_FPS
    int                         memFd;
} domCtrlContext_t;

/*****************************************************************************/
/**
 * @brief TODO
 *
 *****************************************************************************/
typedef struct domCtrlDrawContext_s
{
    void                    *p_next;          /**< pointer for appending this object to a list */

    uint32_t                NumDrawCmds;
    ibdCmd_t                *pIbdDrawCmds;
} domCtrlDrawContext_t;



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
extern RESULT domCtrlCreate
(
    domCtrlContext_t    *pdomContext
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
extern RESULT domCtrlDestroy
(
    domCtrlContext_t    *pdomContext
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
extern RESULT domCtrlSendCommand
(
    domCtrlContext_t    *pdomContext,
    domCtrlCmdId_t      CmdID
);

/* @} dom_ctrl */

#ifdef __cplusplus
}
#endif

#endif /* __DOM_CTRL_H__ */
