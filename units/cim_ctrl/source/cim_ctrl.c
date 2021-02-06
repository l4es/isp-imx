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
 * @file cim_ctrl.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/

#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <oslayer/oslayer.h>
#include <common/return_codes.h>

#include "cim_ctrl.h"

/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER( CIM_CTRL_INFO , "CIM-CTRL: ", INFO,      1 );
CREATE_TRACER( CIM_CTRL_WARN , "CIM-CTRL: ", WARNING,   1 );
CREATE_TRACER( CIM_CTRL_ERROR, "CIM-CTRL: ", ERROR,     1 );


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
 * local function
 *****************************************************************************/


/******************************************************************************
 * See header file for detailed comment.
 *****************************************************************************/

/******************************************************************************
 * CimCtrlCreate()
 *****************************************************************************/
RESULT CimCtrlCreate
(
    CimCtrlContext_t  *pCimCtrlCtx
)
{
    TRACE( CIM_CTRL_INFO, "%s (enter)\n", __func__ );

    DCT_ASSERT( pCimCtrlCtx != NULL );

    /* create command queue */
    if ( OSLAYER_OK != osQueueInit( &pCimCtrlCtx->CommandQueue, pCimCtrlCtx->MaxCommands, sizeof(CimCtrlCmdId_t) ) )
    {
        TRACE( CIM_CTRL_ERROR, "%s (creating command queue (depth: %d) failed)\n", __func__, pCimCtrlCtx->MaxCommands );
        return ( RET_FAILURE );
    }

    /* create handler thread */
    if ( OSLAYER_OK != osThreadCreate( &pCimCtrlCtx->Thread, CimCtrlThreadHandler, pCimCtrlCtx ) )
    {
        TRACE( CIM_CTRL_ERROR, "%s (thread not created)\n", __func__);
        osQueueDestroy( &pCimCtrlCtx->CommandQueue );
        return ( RET_FAILURE );
    }

    TRACE( CIM_CTRL_INFO, "%s (exit)\n", __func__ );

    return ( RET_SUCCESS );
}



/******************************************************************************
 * CimCtrlDestroy()
 *****************************************************************************/
RESULT CimCtrlDestroy
(
    CimCtrlContext_t *pCimCtrlCtx
)
{
    OSLAYER_STATUS osStatus;
    RESULT result;

    TRACE( CIM_CTRL_INFO, "%s (enter)\n", __func__ );

    DCT_ASSERT( pCimCtrlCtx != NULL );

    /* send handler thread a shutdown command */
    result = CimCtrlSendCommand( pCimCtrlCtx, CIM_CTRL_CMD_SHUTDOWN );
    if ( result != RET_SUCCESS )
    {
        TRACE( CIM_CTRL_ERROR, "%s (send command failed -> RESULT=%d)\n", __func__, result);
    }

    /* wait for handler thread to have stopped due to the shutdown command given above */
    if ( OSLAYER_OK != osThreadWait( &pCimCtrlCtx->Thread ) )
    {
        TRACE( CIM_CTRL_ERROR, "%s (waiting for handler thread failed)\n", __func__);
    }

    /* destroy handler thread */
    if ( OSLAYER_OK != osThreadClose( &pCimCtrlCtx->Thread ) )
    {
        TRACE( CIM_CTRL_ERROR, "%s (closing handler thread failed)\n", __func__);
    }

    /* cancel any commands waiting in command queue */
    do
    {
        /* get next command from queue */
        CimCtrlCmdId_t Command;
        osStatus = osQueueTryRead( &pCimCtrlCtx->CommandQueue, &Command );

        switch (osStatus)
        {
            case OSLAYER_OK:        /* got a command, so cancel it */
                CimCtrlCompleteCommand( pCimCtrlCtx, Command, RET_CANCELED );
                break;
            case OSLAYER_TIMEOUT:   /* queue is empty */
                break;
            default:                /* something is broken... */
                UPDATE_RESULT( result, RET_FAILURE);
                break;
        }
    } while (osStatus == OSLAYER_OK);

    /* destroy command queue */
    if ( OSLAYER_OK != osQueueDestroy( &pCimCtrlCtx->CommandQueue ) )
    {
        TRACE( CIM_CTRL_ERROR, "%s (destroying command queue failed)\n", __func__ );
    }

    TRACE( CIM_CTRL_INFO, "%s (exit)\n", __func__ );

    return ( result );
}



/******************************************************************************
 * CimCtrlThreadHandler()
 *****************************************************************************/
int32_t CimCtrlThreadHandler
(
	void *p_arg
)
{
    TRACE( CIM_CTRL_INFO, "%s (enter)\n", __func__ );

    if ( p_arg )
    {
        CimCtrlContext_t *pCimCtrlCtx = (CimCtrlContext_t *)p_arg;
        bool_t bExit = BOOL_FALSE;

        do
        {
            /* set default result */
            RESULT result = RET_WRONG_STATE;
            bool_t completeCmd = BOOL_TRUE;

            /* wait for next command */
            CimCtrlCmdId_t Command;
            OSLAYER_STATUS osStatus = osQueueRead( &pCimCtrlCtx->CommandQueue, &Command );
            if (OSLAYER_OK != osStatus)
            {
                TRACE( CIM_CTRL_ERROR, "%s (receiving command failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus );
                continue; /* for now we simply try again */
            }

            TRACE( CIM_CTRL_INFO, "%s (received command %d)\n", __func__, Command );
            switch ( CimCtrlGetState( pCimCtrlCtx ) )
            {

                case eCimCtrlStateInitialize:
                    {
                        TRACE( CIM_CTRL_INFO, "%s (enter init state)\n", __func__ );

                        switch ( Command )
                        {
                            case CIM_CTRL_CMD_SHUTDOWN:
                                {
                                    CimCtrlSetState( pCimCtrlCtx, eCimCtrlStateInvalid );
                                    bExit = BOOL_TRUE;
                                    result = RET_SUCCESS;
                                    completeCmd = BOOL_FALSE;

                                    break;
                                }

                            case CIM_CTRL_CMD_START:
                                {
                                    CimCtrlSetState( pCimCtrlCtx, eCimCtrlStateRunning );
                                    result = RET_SUCCESS;

                                    break;
                                }

                            default:
                                {
                                    TRACE( CIM_CTRL_ERROR, "%s (invalid command %d)\n", __func__, (int32_t)Command);
                                    break;
                                }
                        }

                        TRACE( CIM_CTRL_INFO, "%s (exit init state)\n", __func__ );

                        break;
                    }

                case eCimCtrlStateRunning:
                    {
                        TRACE( CIM_CTRL_INFO, "%s (enter run state)\n", __func__ );

                        switch ( Command )
                        {
                             case CIM_CTRL_CMD_STOP:
                                {
                                     CimCtrlSetState( pCimCtrlCtx, eCimCtrlStateStopped );

                                    result = RET_SUCCESS;
                                    break;
                                }

                            default:
                                {
                                    TRACE( CIM_CTRL_ERROR, "%s (invalid command %d)\n", __func__, (int32_t)Command);
                                    break;
                                }
                        }

                        TRACE( CIM_CTRL_INFO, "%s (exit run state)\n", __func__ );

                        break;
                    }

                case eCimCtrlStateStopped:
                    {
                        TRACE( CIM_CTRL_INFO, "%s (enter stop state)\n", __func__);

                        switch ( Command )
                        {
                            case CIM_CTRL_CMD_SHUTDOWN:
                                {
                                    CimCtrlSetState( pCimCtrlCtx, eCimCtrlStateInvalid );
                                    bExit       = BOOL_TRUE;
                                    result      = RET_SUCCESS;
                                    completeCmd = BOOL_FALSE;

                                    break;
                                }

                            case CIM_CTRL_CMD_START:
                                {
                                    CimCtrlSetState( pCimCtrlCtx, eCimCtrlStateRunning );
                                    result = RET_SUCCESS;

                                    break;
                                }

                            default:
                                {
                                    TRACE( CIM_CTRL_ERROR, "%s (invalid command %d)\n", __func__, (int32_t)Command);
                                    break;
                                }
                        }

                        TRACE( CIM_CTRL_INFO, "%s (exit stop state)\n", __func__ );

                        break;
                    }

                default:
                    {
                        TRACE( CIM_CTRL_ERROR, "%s (illegal state %d)\n", __func__, (int32_t)CimCtrlGetState( pCimCtrlCtx ) );
                        break;
                    }
            }

            if ( completeCmd == BOOL_TRUE )
            {
                /* complete command */
                CimCtrlCompleteCommand( pCimCtrlCtx, Command, result );
            }
        }
        while ( bExit == BOOL_FALSE );  /* !bExit */
    }

    TRACE( CIM_CTRL_INFO, "%s (exit)\n", __func__ );

    return ( 0 );
}



/******************************************************************************
 * CimCtrlSendCommand()
 *****************************************************************************/
RESULT CimCtrlSendCommand
(
    CimCtrlContext_t    *pCimCtrlCtx,
    CimCtrlCmdId_t      Command
)
{
    TRACE( CIM_CTRL_INFO, "%s (enter)\n", __func__ );

    if( pCimCtrlCtx == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    /* are we shutting down? */
    if ( CimCtrlGetState( pCimCtrlCtx ) == eCimCtrlStateInvalid )
    {
        return ( RET_CANCELED );
    }

    /* send command */
    OSLAYER_STATUS osStatus = osQueueWrite( &pCimCtrlCtx->CommandQueue, &Command );
    if ( osStatus != OSLAYER_OK )
    {
        TRACE( CIM_CTRL_ERROR, "%s (sending command to queue failed -> OSLAYER_STATUS=%d)\n", __func__, CimCtrlGetState( pCimCtrlCtx ), osStatus );
    }

    TRACE( CIM_CTRL_INFO, "%s (exit)\n", __func__ );

    return ( (osStatus == OSLAYER_OK) ? RET_SUCCESS : RET_FAILURE );
}




/******************************************************************************
 * CimCtrlCompleteCommand()
 *****************************************************************************/
void CimCtrlCompleteCommand
(
    CimCtrlContext_t    *pCimCtrlCtx,
    CimCtrlCmdId_t      Command,
    RESULT              result
)
{
    DCT_ASSERT( pCimCtrlCtx != NULL );
    DCT_ASSERT( pCimCtrlCtx->cimCbCompletion != NULL );
    pCimCtrlCtx->cimCbCompletion( Command, result, pCimCtrlCtx->pUserContext );
}


