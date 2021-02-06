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
 * @xom_ctrl.c
 *
 * @brief
 *   Implementation of xom ctrl.
 *
 *****************************************************************************/

#include <time.h>

#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>
#include <common/align.h>

#include <oslayer/oslayer.h>

#include <bufferpool/media_buffer_queue_ex.h>

#include "xom_ctrl.h"

/******************************************************************************
 * local macro definitions
 *****************************************************************************/

CREATE_TRACER(XOM_CTRL_INFO , "XOM-CTRL: ", INFO,  0);
CREATE_TRACER(XOM_CTRL_ERROR, "XOM-CTRL: ", ERROR, 1);

/******************************************************************************
 * local type definitions
 *****************************************************************************/


/******************************************************************************
 * local variable declarations
 *****************************************************************************/


/******************************************************************************
 * local function prototypes
 *****************************************************************************/

/******************************************************************************
 * xomCtrlGetState()
 *****************************************************************************/
static inline xomCtrlState_t xomCtrlGetState
(
    xomCtrlContext_t    *pXomContext
);

/******************************************************************************
 * xomCtrlSetState()
 *****************************************************************************/
static inline void xomCtrlSetState
(
    xomCtrlContext_t    *pXomContext,
    xomCtrlState_t      newState
);

/******************************************************************************
 * xomCtrlCompleteCommand()
 *****************************************************************************/
static void xomCtrlCompleteCommand
(
    xomCtrlContext_t        *pXomContext,
    xomCtrlCmdID_t          CmdID,
    RESULT                  result
);

/******************************************************************************
 * xomCtrlThreadHandler()
 *****************************************************************************/
static int32_t xomCtrlThreadHandler
(
    void *p_arg
);

/******************************************************************************
 * xomCtrlMapBuffer()
 *****************************************************************************/
static RESULT xomCtrlBufferCallback
(
    xomCtrlContext_t    *pXomContext,
    MediaBuffer_t       *pBuffer
);

/******************************************************************************
 * xomCtrlMapBufferYUV422Semi()
 *****************************************************************************/
static RESULT xomCtrlMapBufferYUV422Semi
(
    xomCtrlContext_t    *pXomContext,
    PicBufMetaData_t    *pPicBufMetaData
);

/******************************************************************************
 * xomCtrlUnMapBufferYUV422Semi()
 *****************************************************************************/
static RESULT xomCtrlUnMapBufferYUV422Semi
(
    xomCtrlContext_t    *pXomContext
);

/******************************************************************************
 * API functions; see header file for detailed comment.
 *****************************************************************************/

/******************************************************************************
 * xomCtrlCreate()
 *****************************************************************************/
RESULT xomCtrlCreate
(
    xomCtrlContext_t    *pXomContext
)
{
    RESULT result;

    TRACE(XOM_CTRL_INFO, "%s (enter)\n", __func__);

    DCT_ASSERT( pXomContext != NULL );

    // finalize init of context
    pXomContext->xomBufferCb    = NULL;
    pXomContext->pContext = NULL;
    pXomContext->FrameSkip = 0;
    pXomContext->FrameIdx = 0;

    // add HAL reference
    result = HalAddRef( pXomContext->HalHandle );
    if (result != RET_SUCCESS)
    {
        TRACE( XOM_CTRL_ERROR, "%s (adding HAL reference failed)\n", __func__ );
        return result;
    }

    // create command queue
    if ( OSLAYER_OK != osQueueInit( &pXomContext->CommandQueue, pXomContext->MaxCommands, sizeof(xomCtrlCmd_t) ) )
    {
        TRACE(XOM_CTRL_ERROR, "%s (creating command queue (depth: %d) failed)\n", __func__, pXomContext->MaxCommands);
        HalDelRef( pXomContext->HalHandle );
        return ( RET_FAILURE );
    }

    // create full buffer queue
    if ( OSLAYER_OK != osQueueInit( &pXomContext->FullBufQueue, pXomContext->MaxBuffers, sizeof(MediaBuffer_t *) ) )
    {
        TRACE(XOM_CTRL_ERROR, "%s (creating buffer queue (depth: %d) failed)\n", __func__, pXomContext->MaxBuffers);
        osQueueDestroy( &pXomContext->CommandQueue );
        HalDelRef( pXomContext->HalHandle );
        return ( RET_FAILURE );
    }

    // create handler thread
    if ( OSLAYER_OK != osThreadCreate( &pXomContext->Thread, xomCtrlThreadHandler, pXomContext ) )
    {
        TRACE(XOM_CTRL_ERROR, "%s (creating handler thread failed)\n", __func__);
        osQueueDestroy( &pXomContext->FullBufQueue );
        osQueueDestroy( &pXomContext->CommandQueue );
        HalDelRef( pXomContext->HalHandle );
        return ( RET_FAILURE );
    }

    TRACE(XOM_CTRL_INFO, "%s (exit)\n", __func__ );

    return ( RET_SUCCESS );
}


/******************************************************************************
 * xomCtrlDestroy()
 *****************************************************************************/
RESULT xomCtrlDestroy
(
    xomCtrlContext_t *pXomContext
)
{
    RESULT result = RET_SUCCESS;
    RESULT lres;
    OSLAYER_STATUS osStatus;

    TRACE(XOM_CTRL_INFO, "%s (enter)\n", __func__);

    DCT_ASSERT( pXomContext != NULL );

    // send handler thread a shutdown command
    // ...prepare command
    xomCtrlCmd_t Command;
    MEMSET( &Command, 0, sizeof(Command) );
    Command.CmdID = XOM_CTRL_CMD_SHUTDOWN;
    // ....send command
    lres = xomCtrlSendCommand( pXomContext, &Command );
    if (lres != RET_SUCCESS)
    {
        TRACE(XOM_CTRL_ERROR, "%s (send command failed -> RESULT=%d)\n", __func__, result);
        UPDATE_RESULT( result, lres);
    }

    // wait for handler thread to have stopped due to the shutdown command given above
    if ( OSLAYER_OK != osThreadWait( &pXomContext->Thread ) )
    {
        TRACE(XOM_CTRL_ERROR, "%s (waiting for handler thread failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // destroy handler thread
    if ( OSLAYER_OK != osThreadClose( &pXomContext->Thread ) )
    {
        TRACE(XOM_CTRL_ERROR, "%s (closing handler thread failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // cancel any further commands waiting in command queue
    do
    {
        // get next command from queue
        xomCtrlCmd_t Command;
        osStatus = osQueueTryRead( &pXomContext->CommandQueue, &Command );

        switch (osStatus)
        {
            case OSLAYER_OK:        // got a command, so cancel it
                xomCtrlCompleteCommand( pXomContext, Command.CmdID, RET_CANCELED );
                break;
            case OSLAYER_TIMEOUT:   // queue is empty
                break;
            default:                // xomething is broken...
                UPDATE_RESULT( result, RET_FAILURE);
                break;
        }
    } while (osStatus == OSLAYER_OK);

    // destroy full buffer queue
    if ( OSLAYER_OK != osQueueDestroy( &pXomContext->FullBufQueue ) )
    {
        TRACE(XOM_CTRL_ERROR, "%s (destroying full buffer queue failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // destroy command queue
    if ( OSLAYER_OK != osQueueDestroy( &pXomContext->CommandQueue ) )
    {
        TRACE(XOM_CTRL_ERROR, "%s (destroying command queue failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // remove HAL reference
    lres = HalDelRef( pXomContext->HalHandle );
    if (lres != RET_SUCCESS)
    {
        TRACE( XOM_CTRL_ERROR, "%s (removing HAL reference failed)\n", __func__ );
        UPDATE_RESULT( result, RET_FAILURE);
    }

    TRACE(XOM_CTRL_INFO, "%s (exit)\n", __func__ );

    return result;
}


/******************************************************************************
 * xomCtrlSendCommand()
 *****************************************************************************/
RESULT xomCtrlSendCommand
(
    xomCtrlContext_t    *pXomContext,
    xomCtrlCmd_t        *pCommand
)
{
    TRACE(XOM_CTRL_INFO, "%s (enter), command=%u \n", __func__, pCommand->CmdID);

    if( (pXomContext == NULL) || (pCommand == NULL) )
    {
        return RET_NULL_POINTER;
    }

    // are we shutting down?
    if ( xomCtrlGetState( pXomContext ) == eXomCtrlStateInvalid )
    {
        return RET_CANCELED;
    }

    // send command
    OSLAYER_STATUS osStatus = osQueueWrite( &pXomContext->CommandQueue, pCommand);
    if (osStatus != OSLAYER_OK)
    {
        TRACE(XOM_CTRL_ERROR, "%s (sending command to queue failed -> OSLAYER_STATUS=%d)\n", __func__, xomCtrlGetState( pXomContext ), osStatus);
    }

    TRACE(XOM_CTRL_INFO, "%s (exit)\n", __func__);

    return ( (osStatus == OSLAYER_OK) ? RET_SUCCESS : RET_FAILURE);
}


/******************************************************************************
 * Local functions
 *****************************************************************************/

/******************************************************************************
 * xomCtrlGetState()
 *****************************************************************************/
static inline xomCtrlState_t xomCtrlGetState
(
    xomCtrlContext_t    *pXomContext
)
{
    DCT_ASSERT( pXomContext != NULL );
    return ( pXomContext->State );
}


/******************************************************************************
 * xomCtrlSetState()
 *****************************************************************************/
static inline void xomCtrlSetState
(
    xomCtrlContext_t    *pXomContext,
    xomCtrlState_t      newState
)
{
    DCT_ASSERT( pXomContext != NULL );
    pXomContext->State = newState;
}


/******************************************************************************
 * xomCtrlCompleteCommand()
 *****************************************************************************/
static void xomCtrlCompleteCommand
(
    xomCtrlContext_t        *pXomContext,
    xomCtrlCmdID_t          CmdID,
    RESULT                  result
)
{
    TRACE(XOM_CTRL_INFO, "%s (enter)\n", __func__);

    DCT_ASSERT( pXomContext != NULL );

    // do callback
    pXomContext->xomCbCompletion( CmdID, result, pXomContext->pUserContext );

    TRACE(XOM_CTRL_INFO, "%s (exit)\n", __func__);
}


/******************************************************************************
 * xomCtrlThreadHandler()
 *****************************************************************************/
static int32_t xomCtrlThreadHandler
(
    void *p_arg
)
{
    TRACE(XOM_CTRL_INFO, "%s (enter)\n", __func__);

    if ( p_arg == NULL )
    {
        TRACE(XOM_CTRL_ERROR, "%s (arg pointer is NULL)\n", __func__);
    }
    else
    {
        xomCtrlContext_t *pXomContext = (xomCtrlContext_t *)p_arg;

        bool_t bExit = BOOL_FALSE;

        // processing loop
        do
        {
            // set default result
            RESULT result = RET_WRONG_STATE;

            // wait for next command
            xomCtrlCmd_t Command;
            OSLAYER_STATUS osStatus = osQueueRead(&pXomContext->CommandQueue, &Command);
            if (OSLAYER_OK != osStatus)
            {
                TRACE(XOM_CTRL_ERROR, "%s (receiving command failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus);
                continue; // for now we simply try again
            }

            // process command
            switch ( Command.CmdID )
            {
                case XOM_CTRL_CMD_START:
                {
                    TRACE(XOM_CTRL_INFO, "%s (begin XOM_CTRL_CMD_START)\n", __func__);

                    switch ( xomCtrlGetState( pXomContext ) )
                    {
                        case eXomCtrlStatePaused:
                        case eXomCtrlStateIdle:
                        {
                            pXomContext->xomBufferCb    = Command.Params.Start.xomBufferCb;
                            pXomContext->pContext = Command.Params.Start.pContext;
                            pXomContext->FrameSkip     = Command.Params.Start.FrameSkip;
                            pXomContext->FrameIdx      = 0;

                            xomCtrlSetState( pXomContext, eXomCtrlStateRunning );

                            result = RET_SUCCESS;
                            break;
                        }
                        default:
                        {
                            TRACE(XOM_CTRL_ERROR, "%s (wrong state %d)\n", __func__, xomCtrlGetState( pXomContext ));
                        }
                    }

                    TRACE(XOM_CTRL_INFO, "%s (end XOM_CTRL_CMD_START)\n", __func__);

                    break;
                }

                case XOM_CTRL_CMD_PAUSE:
                {
                    TRACE(XOM_CTRL_INFO, "%s (begin XOM_CTRL_CMD_PAUSE)\n", __func__);

                    switch ( xomCtrlGetState( pXomContext ) )
                    {
                        case eXomCtrlStateRunning:
                        {
                            xomCtrlSetState( pXomContext, eXomCtrlStatePaused );
                        }
                        case eXomCtrlStatePaused:
                        case eXomCtrlStateIdle:
                        {
                            result = RET_SUCCESS;
                            break;
                        }
                        default:
                        {
                            TRACE(XOM_CTRL_ERROR, "%s (wrong state %d)\n", __func__, xomCtrlGetState( pXomContext ));
                        }
                    }

                    TRACE(XOM_CTRL_INFO, "%s (end XOM_CTRL_CMD_PAUSE)\n", __func__);

                    break;
                }

                case XOM_CTRL_CMD_RESUME:
                {
                    TRACE(XOM_CTRL_INFO, "%s (begin XOM_CTRL_CMD_RESUME)\n", __func__);

                    switch ( xomCtrlGetState( pXomContext ) )
                    {
                        case eXomCtrlStatePaused:
                        {
                            xomCtrlSetState( pXomContext, eXomCtrlStateRunning );
                        }
                        case eXomCtrlStateIdle:
                        {
                            result = RET_SUCCESS;
                            break;
                        }
                        default:
                        {
                            TRACE(XOM_CTRL_ERROR, "%s (wrong state %d)\n", __func__, xomCtrlGetState( pXomContext ));
                        }
                    }

                    TRACE(XOM_CTRL_INFO, "%s (end XOM_CTRL_CMD_RESUME)\n", __func__);

                    break;
                }

                case XOM_CTRL_CMD_STOP:
                {
                    TRACE(XOM_CTRL_INFO, "%s (begin XOM_CTRL_CMD_STOP)\n", __func__);

                    switch ( xomCtrlGetState( pXomContext ) )
                    {
                        case eXomCtrlStatePaused:
                        case eXomCtrlStateRunning:
                        {
                            // reset
                            pXomContext->xomBufferCb = NULL;
                            pXomContext->pContext = NULL;
                            pXomContext->FrameSkip = 0;
                            pXomContext->FrameIdx = 0;

                            // back to idle state
                            xomCtrlSetState( pXomContext, eXomCtrlStateIdle );
                        }
                        case eXomCtrlStateIdle:
                        {
                            // we allow this state as well, as state running may be left automatically
                            result = RET_SUCCESS;
                            break;
                        }
                        default:
                        {
                            TRACE(XOM_CTRL_ERROR, "%s (wrong state %d)\n", __func__, xomCtrlGetState( pXomContext ));
                        }
                    }

                    TRACE(XOM_CTRL_INFO, "%s (end XOM_CTRL_CMD_STOP)\n", __func__);

                    break;
                }

                case XOM_CTRL_CMD_SHUTDOWN:
                {
                    TRACE(XOM_CTRL_INFO, "%s (begin XOM_CTRL_CMD_SHUTDOWN)\n", __func__);

                    switch ( xomCtrlGetState( pXomContext ) )
                    {
                        case eXomCtrlStatePaused:
                        case eXomCtrlStateRunning:
                        {
                            // reset
                            pXomContext->xomBufferCb = NULL;
                            pXomContext->pContext = NULL;
                            pXomContext->FrameSkip = 0;
                            pXomContext->FrameIdx = 0;
                        }
                        case eXomCtrlStateIdle:
                        {
                            xomCtrlSetState( pXomContext, eXomCtrlStateInvalid ); // stop further commands from being send to command queue
                            bExit = BOOL_TRUE;
                            result = RET_PENDING; // avoid completion below
                            break;
                        }
                        default:
                        {
                            TRACE(XOM_CTRL_ERROR, "%s (wrong state %d)\n", __func__, xomCtrlGetState( pXomContext ));
                        }
                    }

                    TRACE(XOM_CTRL_INFO, "%s (end XOM_CTRL_CMD_SHUTDOWN)\n", __func__);

                    break;
                }

                case XOM_CTRL_CMD_PROCESS_BUFFER:
                {
                    TRACE(XOM_CTRL_INFO, "%s (begin XOM_CTRL_CMD_PROCESS_BUFFER, state=%d)\n", __func__, xomCtrlGetState( pXomContext ));

                    switch ( xomCtrlGetState( pXomContext ) )
                    {
                        case eXomCtrlStatePaused:
                        case eXomCtrlStateIdle:
                        {
                            // just discard buffer
                            MediaBuffer_t *pBuffer = NULL;
                            osStatus = osQueueTryRead( &pXomContext->FullBufQueue, &pBuffer );
                            if ( ( osStatus != OSLAYER_OK ) || ( pBuffer == NULL ) )
                            {
                                TRACE(XOM_CTRL_ERROR, "%s (receiving full buffer failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus);
                                break;
                            }
                            MediaBufUnlockBuffer( pBuffer );

                            result = RET_SUCCESS;
                            break;
                        }

                        case eXomCtrlStateRunning:
                        {
                            MediaBuffer_t *pBuffer = NULL;
                            osStatus = osQueueTryRead( &pXomContext->FullBufQueue, &pBuffer );
                            if ( ( osStatus != OSLAYER_OK ) || ( pBuffer == NULL ) )
                            {
                                TRACE(XOM_CTRL_ERROR, "%s (receiving full buffer failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus);
                               break;
                            }

                            if ( pBuffer )
                            {
                                result = RET_SUCCESS;

                                if ( 0 == (pXomContext->FrameIdx % (pXomContext->FrameSkip + 1)) )
                                {
                                    // get image and call external buffer
                                    result = xomCtrlBufferCallback( pXomContext, pBuffer );
                                }
                                ++pXomContext->FrameIdx;

                                // release buffer
                                MediaBufUnlockBuffer( pBuffer );
                            }
                            else
                            {
                                result = RET_FAILURE;
                            }

                            break;
                        }

                        default:
                        {
                            TRACE(XOM_CTRL_ERROR, "%s (wrong state %d)\n", __func__, xomCtrlGetState( pXomContext ));
                        }
                    }

                    TRACE(XOM_CTRL_INFO, "%s (end XOM_CTRL_CMD_PROCESS_BUFFER)\n", __func__);

                    break;
                }

                default:
                {
                    TRACE(XOM_CTRL_ERROR, "%s (illegal command %d)\n", __func__, Command);
                    result = RET_NOTSUPP;
                }
            }

            // complete command?
            if (result != RET_PENDING)
            {
                xomCtrlCompleteCommand( pXomContext, Command.CmdID, result );
            }
        }
        while ( bExit == BOOL_FALSE );  // !bExit
    }

    TRACE(XOM_CTRL_INFO, "%s (exit)\n", __func__);

    return ( 0 );
}

static RESULT xomCtrlBufferCallback
(
    xomCtrlContext_t   *pXomContext,
    MediaBuffer_t      *pBuffer
)
{
    RESULT result = RET_SUCCESS;

    TRACE(XOM_CTRL_INFO, "%s (enter)\n", __func__);

    if ( (pXomContext == NULL) || (pBuffer == NULL) )
    {
        return RET_NULL_POINTER;
    }
    if ( (pXomContext->xomBufferCb == NULL) )
    {
        return RET_NULL_POINTER;
    }

    // get & check buffer meta data
    PicBufMetaData_t *pPicBufMetaData = (PicBufMetaData_t *)(pBuffer->pMetaData);
    if (pPicBufMetaData == NULL)
    {
        return RET_NULL_POINTER;
    }
    result = PicBufIsConfigValid( pPicBufMetaData );
    if (RET_SUCCESS != result)
    {
        return result;
    }

    // check type, select map & unmap functions
    switch (pPicBufMetaData->Type)
    {
        case PIC_BUF_TYPE_RAW8:
        case PIC_BUF_TYPE_RAW16:
        case PIC_BUF_TYPE_RAW10:
        case PIC_BUF_TYPE_JPEG:
        case PIC_BUF_TYPE_YCbCr444:
        case PIC_BUF_TYPE_YCbCr420:
        case PIC_BUF_TYPE_RGB888:
            UPDATE_RESULT( result, RET_NOTSUPP );
            break;
        case PIC_BUF_TYPE_YCbCr422:
            switch (pPicBufMetaData->Layout)
            {
                case PIC_BUF_LAYOUT_PLANAR:
                case PIC_BUF_LAYOUT_COMBINED:
                    UPDATE_RESULT( result, RET_NOTSUPP );
                    break;
                case PIC_BUF_LAYOUT_SEMIPLANAR:
                {
                    // set format/layout dependent function pointers
                    pXomContext->MapBuffer   = xomCtrlMapBufferYUV422Semi;
                    pXomContext->UnMapBuffer = xomCtrlUnMapBufferYUV422Semi;
                    break;
                }
                default:
                    UPDATE_RESULT( result, RET_NOTSUPP );
            };
            break;
        default:
            UPDATE_RESULT( result, RET_NOTSUPP );
    }
    if (result != RET_SUCCESS)
    {
        TRACE(XOM_CTRL_ERROR, "%s unsupported buffer config -> RESULT=%d\n", __func__, result);
        return result;
    }

    // map buffer
    result = pXomContext->MapBuffer( pXomContext, pPicBufMetaData );
    if (RET_SUCCESS != result)
    {
        TRACE(XOM_CTRL_ERROR, "%s MapBuffer() failed -> RESULT=%d\n", __func__, result);
        return result;
    }

    // external buffer callback
    result = pXomContext->xomBufferCb( &(pXomContext->MappedMetaData), pXomContext->pContext );
    if (RET_SUCCESS != result)
    {
        TRACE(XOM_CTRL_ERROR, "%s xomBufferCb() failed -> RESULT=%d\n", __func__, result);
        return result;
    }

    // unmap buffer again
    result = pXomContext->UnMapBuffer( pXomContext );
    if (RET_SUCCESS != result)
    {
        TRACE(XOM_CTRL_ERROR, "%s UnMapBuffer() failed -> RESULT=%d\n", __func__, result);
        return result;
    }

    TRACE(XOM_CTRL_INFO, "%s (exit)\n", __func__ );

    return result;
}

static RESULT xomCtrlMapBufferYUV422Semi
(
    xomCtrlContext_t    *pXomContext,
    PicBufMetaData_t    *pPicBufMetaData
)
{
    RESULT result = RET_SUCCESS;
    RESULT lres;

    TRACE(XOM_CTRL_INFO, "%s (enter)\n", __func__);

    if ( (pXomContext == NULL) || (pPicBufMetaData == NULL) )
    {
        return RET_NULL_POINTER;
    }

    // copy buffer meta data; clear mapped buffer pointers for easier unmapping on errors below
    pXomContext->MappedMetaData = *pPicBufMetaData;
    pXomContext->MappedMetaData.Data.YCbCr.semiplanar.Y.pData = NULL;
    pXomContext->MappedMetaData.Data.YCbCr.semiplanar.CbCr.pData = NULL;
    // note: implementation which assumes that on-board memory is used for buffers!

    // get sizes & base addresses of planes
    uint32_t YCPlaneSize  = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicWidthBytes * pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicHeightPixel;
    uint32_t YBaseAddr    = pPicBufMetaData->Data.YCbCr.semiplanar.Y.BaseAddress;
    uint32_t CbCrBaseAddr = pPicBufMetaData->Data.YCbCr.semiplanar.CbCr.BaseAddress;

    // map luma plane
    lres = HalMapMemory( pXomContext->HalHandle,
                         YBaseAddr,
                         YCPlaneSize,
                         HAL_MAPMEM_READONLY,
                         (void**)&(pXomContext->MappedMetaData.Data.YCbCr.semiplanar.Y.pData)    );
    UPDATE_RESULT( result, lres );

    // map combined chroma plane
    lres = HalMapMemory( pXomContext->HalHandle,
                         CbCrBaseAddr,
                         YCPlaneSize,
                         HAL_MAPMEM_READONLY,
                         (void**)&(pXomContext->MappedMetaData.Data.YCbCr.semiplanar.CbCr.pData) );
    UPDATE_RESULT( result, lres );

    // check for errors
    if (result != RET_SUCCESS)
    {
        TRACE(XOM_CTRL_ERROR, "%s mapping buffer failed (RESULT=%d)\n", __func__, result);
        // unmap partially mapped buffer
        xomCtrlUnMapBufferYUV422Semi( pXomContext );
    }

    TRACE(XOM_CTRL_INFO, "%s (exit)\n", __func__);

    return result;
}

static RESULT xomCtrlUnMapBufferYUV422Semi
(
    xomCtrlContext_t    *pXomContext
)
{
    RESULT result = RET_SUCCESS;
    RESULT lres;

    TRACE(XOM_CTRL_INFO, "%s (enter)\n", __func__);

    if (pXomContext == NULL)
    {
        return RET_NULL_POINTER;
    }
    // note: implementation which assumes that on-board memory is used for buffers!

    // unmap (partially) mapped buffer
    if (pXomContext->MappedMetaData.Data.YCbCr.semiplanar.Y.pData)
    {
        // unmap luma plane
        lres = HalUnMapMemory( pXomContext->HalHandle, pXomContext->MappedMetaData.Data.YCbCr.semiplanar.Y.pData    );
        UPDATE_RESULT( result, lres );
    }
    if (pXomContext->MappedMetaData.Data.YCbCr.semiplanar.CbCr.pData)
    {
        // unmap combined chroma plane
        lres = HalUnMapMemory( pXomContext->HalHandle, pXomContext->MappedMetaData.Data.YCbCr.semiplanar.CbCr.pData );
        UPDATE_RESULT( result, lres );
    }

    TRACE(XOM_CTRL_INFO, "%s (exit)\n", __func__);

    return result;
}
