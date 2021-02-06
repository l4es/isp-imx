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
 * @file XOM_ctrl_api.c
 *
 * @brief
 *   Implementation of XOM ctrl API.
 *
 *****************************************************************************/

#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include "xom_ctrl.h"
#include "xom_ctrl_api.h"

CREATE_TRACER(XOM_CTRL_API_INFO , "XOM-CTRL-API: ", INFO,  0);
CREATE_TRACER(XOM_CTRL_API_ERROR, "XOM-CTRL-API: ", ERROR, 1);


/***** local functions ***********************************************/

/***** API implementation ***********************************************/

/******************************************************************************
 * xomCtrlInit()
 *****************************************************************************/
RESULT xomCtrlInit
(
    xomCtrlConfig_t *pConfig
)
{
    RESULT result = RET_FAILURE;

    xomCtrlContext_t *pXomContext;

    TRACE( XOM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if ( pConfig == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( ( pConfig->xomCbCompletion == NULL ) ||
         ( pConfig->MaxPendingCommands == 0 ) )
    {
        return ( RET_INVALID_PARM );
    }

    // allocate control context
    pXomContext = malloc( sizeof(xomCtrlContext_t) );
    if ( pXomContext == NULL )
    {
        TRACE( XOM_CTRL_API_ERROR, "%s (allocating control context failed)\n", __func__ );
        return ( RET_OUTOFMEM );
    }
    MEMSET( pXomContext, 0, sizeof(xomCtrlContext_t) );

    // pre initialize control context
    pXomContext->State           = eXomCtrlStateInvalid;
    pXomContext->MaxCommands     = pConfig->MaxPendingCommands;
    pXomContext->MaxBuffers      = pConfig->MaxBuffers;
    pXomContext->xomCbCompletion = pConfig->xomCbCompletion;
    pXomContext->pUserContext    = pConfig->pUserContext;
    pXomContext->HalHandle       = pConfig->HalHandle;

    // create control process
    result = xomCtrlCreate( pXomContext );
    if (result != RET_SUCCESS)
    {
        TRACE( XOM_CTRL_API_ERROR, "%s (creating control process failed)\n", __func__ );
        free( pXomContext );
    }
    else
    {
        // control context is initialized, we're ready and in idle state
        pXomContext->State = eXomCtrlStateIdle;

        // success, so let's return the control context handle
        pConfig->xomCtrlHandle = (xomCtrlHandle_t)pXomContext;
    }

    TRACE( XOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( result );
}


/******************************************************************************
 * xomCtrlStart()
 *****************************************************************************/
RESULT xomCtrlStart
(
    xomCtrlHandle_t     xomCtrlHandle,
    xomCtrlBufferCb_t   xomBufferCb,
    void                *pContext,
    uint8_t             FrameSkip
)
{
    TRACE(XOM_CTRL_API_INFO, "%s (enter)\n", __func__);

    if (xomCtrlHandle == NULL)
    {
        return RET_NULL_POINTER;
    }

    xomCtrlContext_t *pXomContext = (xomCtrlContext_t *)xomCtrlHandle;

    // prepare command
    xomCtrlCmd_t Command;
    MEMSET( &Command, 0, sizeof(Command) );
    Command.CmdID = XOM_CTRL_CMD_START;
    Command.Params.Start.xomBufferCb = xomBufferCb;
    Command.Params.Start.pContext = pContext;
    Command.Params.Start.FrameSkip = FrameSkip;

    // send command
    RESULT result = xomCtrlSendCommand( pXomContext, &Command );
    if (result != RET_SUCCESS)
    {
         TRACE(XOM_CTRL_API_ERROR, "%s (send command failed -> RESULT=%d)\n", __func__, result);
    }

    TRACE(XOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return (result != RET_SUCCESS) ? result : RET_PENDING;
}


/******************************************************************************
 * xomCtrlStop()
 *****************************************************************************/
RESULT xomCtrlStop
(
    xomCtrlHandle_t xomCtrlHandle
)
{
    TRACE(XOM_CTRL_API_INFO, "%s (enter)\n", __func__);

    if( xomCtrlHandle == NULL )
    {
        return RET_NULL_POINTER;
    }

    xomCtrlContext_t *pXomContext = (xomCtrlContext_t *)xomCtrlHandle;

    // prepare command
    xomCtrlCmd_t Command;
    MEMSET( &Command, 0, sizeof(Command) );
    Command.CmdID = XOM_CTRL_CMD_STOP;

    // send command
    RESULT result = xomCtrlSendCommand( pXomContext, &Command );
    if (result != RET_SUCCESS)
    {
         TRACE(XOM_CTRL_API_ERROR, "%s (send command failed -> RESULT=%d)\n", __func__, result);
    }

    TRACE(XOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return (result != RET_SUCCESS) ? result : RET_PENDING;
}


/******************************************************************************
 * xomCtrlPause()
 *****************************************************************************/
RESULT xomCtrlPause
(
    xomCtrlHandle_t xomCtrlHandle
)
{
    TRACE(XOM_CTRL_API_INFO, "%s (enter)\n", __func__);

    if( xomCtrlHandle == NULL )
    {
        return RET_NULL_POINTER;
    }

    xomCtrlContext_t *pXomContext = (xomCtrlContext_t *)xomCtrlHandle;

    // prepare command
    xomCtrlCmd_t Command;
    MEMSET( &Command, 0, sizeof(Command) );
    Command.CmdID = XOM_CTRL_CMD_PAUSE;

    // send command
    RESULT result = xomCtrlSendCommand( pXomContext, &Command );
    if (result != RET_SUCCESS)
    {
         TRACE(XOM_CTRL_API_ERROR, "%s (send command failed -> RESULT=%d)\n", __func__, result);
    }

    TRACE(XOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return (result != RET_SUCCESS) ? result : RET_PENDING;
}


/******************************************************************************
 * xomCtrlResume()
 *****************************************************************************/
RESULT xomCtrlResume
(
    xomCtrlHandle_t xomCtrlHandle
)
{
    TRACE(XOM_CTRL_API_INFO, "%s (enter)\n", __func__);

    if( xomCtrlHandle == NULL )
    {
        return RET_NULL_POINTER;
    }

    xomCtrlContext_t *pXomContext = (xomCtrlContext_t *)xomCtrlHandle;

    // prepare command
    xomCtrlCmd_t Command;
    MEMSET( &Command, 0, sizeof(Command) );
    Command.CmdID = XOM_CTRL_CMD_RESUME;

    // send command
    RESULT result = xomCtrlSendCommand( pXomContext, &Command );
    if (result != RET_SUCCESS)
    {
         TRACE(XOM_CTRL_API_ERROR, "%s (send command failed -> RESULT=%d)\n", __func__, result);
    }

    TRACE(XOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return (result != RET_SUCCESS) ? result : RET_PENDING;
}


/******************************************************************************
 * xomCtrlShutDown()
 *****************************************************************************/
RESULT xomCtrlShutDown
(
    xomCtrlHandle_t xomCtrlHandle
)
{
    TRACE(XOM_CTRL_API_INFO, "%s (enter)\n", __func__);

    if( xomCtrlHandle == NULL )
    {
        return RET_NULL_POINTER;
    }

    xomCtrlContext_t *pXomContext = (xomCtrlContext_t *)xomCtrlHandle;

    RESULT result = xomCtrlDestroy( pXomContext );
    if (result != RET_SUCCESS)
    {
         TRACE(XOM_CTRL_API_ERROR, "%s (destroying control process failed -> RESULT=%d)\n", __func__, result);
    }

    // release context memory
    free( pXomContext );

    TRACE(XOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return result;
}


/******************************************************************************
 * xomCtrlShowBuffer()
 *****************************************************************************/
RESULT  xomCtrlShowBuffer
(
    xomCtrlHandle_t         xomCtrlHandle,
    MediaBuffer_t           *pBuffer
)
{
    xomCtrlContext_t *pXomContext = (xomCtrlContext_t *)xomCtrlHandle;

    TRACE( XOM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if( pXomContext == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if( pBuffer == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( ( eXomCtrlStateIdle    != pXomContext->State )
      && ( eXomCtrlStatePaused  != pXomContext->State )
      && ( eXomCtrlStateRunning != pXomContext->State ) )
    {
        return ( RET_WRONG_STATE );
    }

    MediaBufLockBuffer( pBuffer );
    OSLAYER_STATUS osStatus = osQueueTryWrite( &pXomContext->FullBufQueue, &pBuffer );
    if ( osStatus == OSLAYER_OK )
    {
        // prepare command
        xomCtrlCmd_t Command;
        MEMSET( &Command, 0, sizeof(Command) );
        Command.CmdID = XOM_CTRL_CMD_PROCESS_BUFFER;

        RESULT result = xomCtrlSendCommand( pXomContext, &Command );
        if (result != RET_SUCCESS)
        {
            TRACE(XOM_CTRL_API_ERROR, "%s (send command failed -> RESULT=%d)\n", __func__, result);
        }
    }
    else
    {
        MediaBufUnlockBuffer( pBuffer );
    }

    TRACE( XOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( RET_PENDING );
}
