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
 * @file mom_ctrl.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/

#include <ebase/trace.h>
#include <common/misc.h>

#include <bufferpool/media_buffer.h>
#include <bufferpool/media_buffer_pool.h>
#include <bufferpool/media_buffer_queue_ex.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_drv_api.h>
#include <cameric_drv/cameric_mi_drv_api.h>

#include "mom_ctrl.h"
#include "mom_ctrl_cb.h"

/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER( MOM_CTRL_INFO , "MOM-CTRL: ", INFO,  0 );
CREATE_TRACER( MOM_CTRL_ERROR, "MOM-CTRL: ", ERROR, 1 );


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
static void AddToMediaBufferQueue( List *pList, void *param )
{
#if 0
    ScmiReturnType scmiResult = MediaBufQueueExPutFullBuffer( (MediaBufQueueEx_t *)pList, (MediaBuffer_t *)param );
    DCT_ASSERT( scmiResult == SCMI_RET_SUCCESS);
#endif
    OSLAYER_STATUS osResult = osQueueWrite( (osQueue *)pList, &(param) );
    DCT_ASSERT( OSLAYER_OK == osResult );
}



/******************************************************************************
 * MomCtrlThreadHandler()
 *****************************************************************************/
static int32_t MomCtrlThreadHandler
(
    void *p_arg
)
{
    TRACE(MOM_CTRL_INFO, "%s (enter)\n", __func__);

    if ( p_arg )
    {
        MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)p_arg;

        bool_t bExit = BOOL_FALSE;

        do
        {
            /* set default result */
            RESULT result = RET_WRONG_STATE;
            bool_t completeCmd = BOOL_TRUE;

            /* wait for next command */
            MomCtrlCmdId_t Command;
            OSLAYER_STATUS osStatus = osQueueRead( &pMomCtrlCtx->CommandQueue, &Command );
            if (OSLAYER_OK != osStatus)
            {
                TRACE( MOM_CTRL_ERROR, "%s (receiving command failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus );
                continue; /* for now we simply try again */
            }

            TRACE( MOM_CTRL_INFO, "%s (received command %d)\n", __func__, Command );
            switch ( MomCtrlGetState( pMomCtrlCtx ) )
            {
                case eMomCtrlStateInitialize:
                    {
                        TRACE(MOM_CTRL_INFO, "%s (enter init state)\n", __func__);

                        switch ( Command )
                        {
                            case MOM_CTRL_CMD_SHUTDOWN:
                                {
                                    MomCtrlSetState( pMomCtrlCtx, eMomCtrlStateInvalid );
                                    completeCmd = BOOL_FALSE;
                                    bExit = BOOL_TRUE;
                                    break;
                                }

                            case MOM_CTRL_CMD_START:
                                {
                                    MediaBuffer_t *pBuffer;
                                    uint32_t i;

                                    if ( pMomCtrlCtx->pPicBufPoolMainPath )
                                    {
                                        for ( i=0; i<pMomCtrlCtx->NumBuffersMainPath; i++ )
                                        {
                                            pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolMainPath );
                                            if ( pBuffer != NULL )
                                            {
                                                OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_MAINPATH-1], &pBuffer );
                                                DCT_ASSERT( osStatus == OSLAYER_OK );
                                            }
                                            TRACE( MOM_CTRL_INFO, "%s (req buffer %p)\n", __func__, pBuffer);
                                        }
                                    }

                                    if ( pMomCtrlCtx->pPicBufPoolSelfPath )
                                    {
                                        for ( i=0; i<pMomCtrlCtx->NumBuffersSelfPath; i++ )
                                        {
                                            pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolSelfPath );
                                            if ( pBuffer != NULL )
                                            {
                                                OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_SELFPATH-1], &pBuffer );
                                                DCT_ASSERT( osStatus == OSLAYER_OK );
                                            }
                                            TRACE( MOM_CTRL_INFO, "%s (req buffer %p)\n", __func__, pBuffer);
                                        }
                                    }

									if ( pMomCtrlCtx->pPicBufPoolSelfPath2 )
                                    {
                                        for ( i=0; i<pMomCtrlCtx->NumBuffersSelfPath2; i++ )
                                        {
                                            pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolSelfPath2 );
                                            if ( pBuffer != NULL )
                                            {
                                                OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_SELFPATH2-1], &pBuffer );
                                                DCT_ASSERT( osStatus == OSLAYER_OK );
                                            }
                                            TRACE( MOM_CTRL_INFO, "%s (req buffer %p)\n", __func__, pBuffer);
                                        }
                                    }

									if ( pMomCtrlCtx->pPicBufPoolRdiPath )
                                    {
                                        for ( i=0; i<pMomCtrlCtx->NumBuffersRdiPath; i++ )
                                        {
                                            pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolRdiPath );
                                            if ( pBuffer != NULL )
                                            {
                                                OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_RDIPATH-1], &pBuffer );
                                                DCT_ASSERT( osStatus == OSLAYER_OK );
                                            }
                                            TRACE( MOM_CTRL_INFO, "%s (req buffer %p)\n", __func__, pBuffer);
                                        }
                                    }

									if ( pMomCtrlCtx->pPicBufPoolMetaPath )
                                    {
                                        for ( i=0; i<pMomCtrlCtx->NumBuffersMetaPath; i++ )
                                        {
                                            pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolMetaPath );
                                            if ( pBuffer != NULL )
                                            {
                                                OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_METAPATH-1], &pBuffer );
                                                DCT_ASSERT( osStatus == OSLAYER_OK );
                                            }
                                            TRACE( MOM_CTRL_INFO, "%s (req buffer %p)\n", __func__, pBuffer);
                                        }
                                    }

                                    MomCtrlSetState( pMomCtrlCtx, eMomCtrlStateRunning );
                                    result = RET_SUCCESS;

                                    break;
                                }

                            default:
                                {
                                    TRACE( MOM_CTRL_ERROR, "%s (invalid command %d)\n", __func__, (int32_t)Command);
                                    break;
                                }
                        }

                        TRACE(MOM_CTRL_INFO, "%s (exit init state)\n", __func__);

                        break;
                    }

                case eMomCtrlStateRunning:
                    {
                        TRACE(MOM_CTRL_INFO, "%s (enter run state)\n", __func__);

                        switch ( Command )
                        {
                            case MOM_CTRL_CMD_PROC_FULL_BUFFER_MAINPATH:
                            case MOM_CTRL_CMD_PROC_FULL_BUFFER_SELFPATH:
                            case MOM_CTRL_CMD_PROC_FULL_BUFFER_SELFPATH2:
                            case MOM_CTRL_CMD_PROC_FULL_BUFFER_RDIPATH:
                            case MOM_CTRL_CMD_PROC_FULL_BUFFER_METAPATH:
                                {
                                    OSLAYER_STATUS osStatus;
                                    MediaBuffer_t *pBuffer = NULL;
                                    uint32_t path;

                                    if ( MOM_CTRL_CMD_PROC_FULL_BUFFER_MAINPATH == Command )
                                    {
                                        path    = MOM_CTRL_PATH_MAINPATH;
                                    } else if ( MOM_CTRL_CMD_PROC_FULL_BUFFER_SELFPATH == Command ) {
                                        path    = MOM_CTRL_PATH_SELFPATH;
                                    } else if ( MOM_CTRL_CMD_PROC_FULL_BUFFER_SELFPATH2 == Command ) {
										path    = MOM_CTRL_PATH_SELFPATH2;
									} else if ( MOM_CTRL_CMD_PROC_FULL_BUFFER_RDIPATH == Command ) {
										path    = MOM_CTRL_PATH_RDIPATH;
									} else if ( MOM_CTRL_CMD_PROC_FULL_BUFFER_METAPATH == Command ) {
										path    = MOM_CTRL_PATH_METAPATH;
									} else{
                                        path = MOM_CTRL_PATH_MAX;
                                    }

//                                    while ( (osStatus = osQueueTryRead( &pMomCtrlCtx->FullBufQueue[(path-1)], &pBuffer )) == OSLAYER_OK )
                                    osStatus = osQueueTryRead( &pMomCtrlCtx->FullBufQueue[(path-1)], &pBuffer );
                                    if ( OSLAYER_OK == osStatus )
                                    {
                                        osMutex *pMutex = &(pMomCtrlCtx->PathLock[(path-1)]);
                                        List *pList = &(pMomCtrlCtx->PathQueues[(path-1)]);

                                        pBuffer->isFull = BOOL_TRUE;

                                        if ( !ListEmpty( pList ) )
                                        {
                                            //MediaBufPoolBufferFilled( pPool, pBuffer );

                                            osMutexLock( &pMomCtrlCtx->BufferLock );

                                            osMutexLock( pMutex );
                                            ListForEach( pList, AddToMediaBufferQueue, ((void *)pBuffer) );
                                            osMutexUnlock( pMutex );

                                            osMutexUnlock( &pMomCtrlCtx->BufferLock );

                                            // MediaBufUnlockBuffer( pBuffer );
                                        }
                                        else
                                        {
                                            //MediaBufPoolBufferFilled( pPool, pBuffer );

                                            osMutexLock( &pMomCtrlCtx->BufferLock );
                                            if ( ( MOM_CTRL_PATH_MAINPATH == path ) && ( NULL != pMomCtrlCtx->BufferCbMainPath.fpCallback ) ) {
                                                (pMomCtrlCtx->BufferCbMainPath.fpCallback)( path-1, pBuffer, pMomCtrlCtx->BufferCbMainPath.pBufferCbCtx );
                                            } else if ( ( MOM_CTRL_PATH_SELFPATH == path ) && ( NULL != pMomCtrlCtx->BufferCbSelfPath.fpCallback ) ) {
                                                (pMomCtrlCtx->BufferCbSelfPath.fpCallback)( path-1, pBuffer, pMomCtrlCtx->BufferCbSelfPath.pBufferCbCtx );
                                            } else if ( ( MOM_CTRL_PATH_SELFPATH2 == path ) && ( NULL != pMomCtrlCtx->BufferCbSelfPath2.fpCallback ) ) {
                                                (pMomCtrlCtx->BufferCbSelfPath2.fpCallback)( path-1, pBuffer, pMomCtrlCtx->BufferCbSelfPath2.pBufferCbCtx );
                                            }else if ( ( MOM_CTRL_PATH_RDIPATH == path ) && ( NULL != pMomCtrlCtx->BufferCbRdiPath.fpCallback ) ) {
                                                (pMomCtrlCtx->BufferCbRdiPath.fpCallback)( path-1, pBuffer, pMomCtrlCtx->BufferCbRdiPath.pBufferCbCtx );
                                            }else if ( ( MOM_CTRL_PATH_METAPATH == path ) && ( NULL != pMomCtrlCtx->BufferCbMetaPath.fpCallback ) ) {
                                                (pMomCtrlCtx->BufferCbMetaPath.fpCallback)( path-1, pBuffer, pMomCtrlCtx->BufferCbMetaPath.pBufferCbCtx );
                                            }
                                            osMutexUnlock( &pMomCtrlCtx->BufferLock );
                                            MediaBufUnlockBuffer( pBuffer );
                                        }
                                    }

                                    completeCmd = BOOL_FALSE;
                                    result = RET_SUCCESS;
                                    break;
                                }

                            case MOM_CTRL_CMD_STOP:
                                {
                                    MediaBuffer_t *pBuffer;
/*
                                    if ( pMomCtrlCtx->pPicBufPoolMainPath )
                                    {
                                        TRACE( MOM_CTRL_ERROR, "%s: fill-level MP %d\n", __func__, pMomCtrlCtx->pPicBufPoolMainPath->fillLevel );
                                        while ( pMomCtrlCtx->pPicBufPoolMainPath->fillLevel )
                                        {
                                            osSleep( 10 );
                                        }
                                    }

                                    if ( pMomCtrlCtx->pPicBufPoolSelfPath )
                                    {
                                        TRACE( MOM_CTRL_ERROR, "%s: fill-level SP %d\n", __func__, pMomCtrlCtx->pPicBufPoolSelfPath->fillLevel );
                                        while ( pMomCtrlCtx->pPicBufPoolSelfPath->fillLevel )
                                        {
                                            osSleep( 10 );
                                        }
                                    }
*/
                                    MomCtrlSetState( pMomCtrlCtx, eMomCtrlStateStopped );

                                    /* flush main path queues */
                                    if ( pMomCtrlCtx->pPicBufPoolMainPath )
                                    {
                                        /* flush full buffer queue */
                                        while  ( OSLAYER_OK == osQueueTimedRead( &pMomCtrlCtx->FullBufQueue[MOM_CTRL_PATH_MAINPATH-1], &pBuffer, 10 ) )
                                        {
                                            DCT_ASSERT( pBuffer != NULL );
                                            MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolMainPath, pBuffer );
                                        }

                                        /* flush empty buffers */
                                        while  ( OSLAYER_OK == osQueueTimedRead( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_MAINPATH-1], &pBuffer, 10 ) )
                                        {
                                            DCT_ASSERT( pBuffer != NULL );
                                            MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolMainPath, pBuffer );
                                        }
                                    }

                                    /* flush self path queues */
                                    if ( pMomCtrlCtx->pPicBufPoolSelfPath )
                                    {

                                        /* flush full buffer queue */
                                        while  ( OSLAYER_OK == osQueueTimedRead( &pMomCtrlCtx->FullBufQueue[MOM_CTRL_PATH_SELFPATH-1], &pBuffer, 10 ) )
                                        {
                                            DCT_ASSERT( pBuffer != NULL );
                                            MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolSelfPath, pBuffer );
                                        }

                                        /* flush empty buffers */
                                        while  ( OSLAYER_OK == osQueueTimedRead( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_SELFPATH-1], &pBuffer, 10 ) )
                                        {
                                            DCT_ASSERT( pBuffer != NULL );
                                            MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolSelfPath, pBuffer );
                                        }
                                    }

									/* flush self path2 queues */
                                    if ( pMomCtrlCtx->pPicBufPoolSelfPath2 )
                                    {

                                        /* flush full buffer queue */
                                        while  ( OSLAYER_OK == osQueueTimedRead( &pMomCtrlCtx->FullBufQueue[MOM_CTRL_PATH_SELFPATH2-1], &pBuffer, 10 ) )
                                        {
                                            DCT_ASSERT( pBuffer != NULL );
                                            MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolSelfPath2, pBuffer );
                                        }

                                        /* flush empty buffers */
                                        while  ( OSLAYER_OK == osQueueTimedRead( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_SELFPATH2-1], &pBuffer, 10 ) )
                                        {
                                            DCT_ASSERT( pBuffer != NULL );
                                            MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolSelfPath2, pBuffer );
                                        }
                                    }

									/* flush rdi path queues */
                                    if ( pMomCtrlCtx->pPicBufPoolRdiPath )
                                    {

                                        /* flush full buffer queue */
                                        while  ( OSLAYER_OK == osQueueTimedRead( &pMomCtrlCtx->FullBufQueue[MOM_CTRL_PATH_RDIPATH-1], &pBuffer, 10 ) )
                                        {
                                            DCT_ASSERT( pBuffer != NULL );
                                            MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolRdiPath, pBuffer );
                                        }

                                        /* flush empty buffers */
                                        while  ( OSLAYER_OK == osQueueTimedRead( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_RDIPATH-1], &pBuffer, 10 ) )
                                        {
                                            DCT_ASSERT( pBuffer != NULL );
                                            MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolRdiPath, pBuffer );
                                        }
                                    }

									/* flush meta path queues */
                                    if ( pMomCtrlCtx->pPicBufPoolMetaPath )
                                    {

                                        /* flush full buffer queue */
                                        while  ( OSLAYER_OK == osQueueTimedRead( &pMomCtrlCtx->FullBufQueue[MOM_CTRL_PATH_METAPATH-1], &pBuffer, 10 ) )
                                        {
                                            DCT_ASSERT( pBuffer != NULL );
                                            MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolMetaPath, pBuffer );
                                        }

                                        /* flush empty buffers */
                                        while  ( OSLAYER_OK == osQueueTimedRead( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_METAPATH-1], &pBuffer, 10 ) )
                                        {
                                            DCT_ASSERT( pBuffer != NULL );
                                            MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolMetaPath, pBuffer );
                                        }
                                    }

                                    result = RET_SUCCESS;
                                    break;
                                }

                            default:
                                {
                                    TRACE( MOM_CTRL_ERROR, "%s (invalid command %d)\n", __func__, (int32_t)Command);
                                    break;
                                }
                        }
                        TRACE(MOM_CTRL_INFO, "%s (exit run state)\n", __func__);

                        break;
                    }

                case eMomCtrlStateStopped:
                    {
                        TRACE(MOM_CTRL_INFO, "%s (enter stop state)\n", __func__);
                        switch ( Command )
                        {
                            case MOM_CTRL_CMD_SHUTDOWN:
                                {
                                    MomCtrlSetState( pMomCtrlCtx, eMomCtrlStateInvalid );
                                    completeCmd = BOOL_FALSE;
                                    bExit       = BOOL_TRUE;
                                    break;
                                }

                            case MOM_CTRL_CMD_START:
                                {
                                    MediaBuffer_t *pBuffer;
                                    uint32_t i;

                                    if ( pMomCtrlCtx->pPicBufPoolMainPath )
                                    {
                                        for ( i=0; i<pMomCtrlCtx->NumBuffersMainPath; i++ )
                                        {
                                            pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolMainPath );
                                            if ( pBuffer != NULL )
                                            {
                                                OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_MAINPATH-1], &pBuffer );
                                                DCT_ASSERT( osStatus == OSLAYER_OK );
                                            }
                                            TRACE( MOM_CTRL_INFO, "%s (req buffer %p)\n", __func__, pBuffer);
                                        }
                                    }

                                    if ( pMomCtrlCtx->pPicBufPoolSelfPath )
                                    {
                                        for ( i=0; i<pMomCtrlCtx->NumBuffersSelfPath; i++ )
                                        {
                                            pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolSelfPath );
                                            if ( pBuffer != NULL )
                                            {
                                                OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_SELFPATH-1], &pBuffer );
                                                DCT_ASSERT( osStatus == OSLAYER_OK );
                                            }
                                            TRACE( MOM_CTRL_INFO, "%s (req buffer %p)\n", __func__, pBuffer);
                                        }
                                    }

									if ( pMomCtrlCtx->pPicBufPoolSelfPath2 )
                                    {
                                        for ( i=0; i<pMomCtrlCtx->NumBuffersSelfPath2; i++ )
                                        {
                                            pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolSelfPath2 );
                                            if ( pBuffer != NULL )
                                            {
                                                OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_SELFPATH2-1], &pBuffer );
                                                DCT_ASSERT( osStatus == OSLAYER_OK );
                                            }
                                            TRACE( MOM_CTRL_INFO, "%s (req buffer %p)\n", __func__, pBuffer);
                                        }
                                    }

                                    MomCtrlSetState( pMomCtrlCtx, eMomCtrlStateRunning );
                                    result = RET_SUCCESS;

                                    break;
                                }

                            default:
                                {
                                    TRACE( MOM_CTRL_ERROR, "%s (invalid command %d)\n", __func__, (int32_t)Command);
                                    break;
                                }
                        }

                        TRACE( MOM_CTRL_INFO, "%s (exit stop state)\n", __func__ );

                        break;
                    }

                default:
                    {
                        TRACE( MOM_CTRL_ERROR, "%s (illegal state %d)\n", __func__, (int32_t)MomCtrlGetState( pMomCtrlCtx ) );
                        break;
                    }
            }
            if ( completeCmd == BOOL_TRUE )
            {
                /* complete command */
                MomCtrlCompleteCommand( pMomCtrlCtx, Command, result );
            }
        }
        while ( bExit == BOOL_FALSE );  /* !bExit */
    }

    TRACE(MOM_CTRL_INFO, "%s (exit)\n", __func__);

    return ( 0 );
}



/******************************************************************************
 * See header file for detailed comment.
 *****************************************************************************/

/******************************************************************************
 * MomCtrlCreate()
 *****************************************************************************/
RESULT MomCtrlCreate
(
    MomCtrlContext_t  *pMomCtrlCtx
)
{
    RESULT result;
    uint32_t i;

    TRACE( MOM_CTRL_INFO, "%s (enter)\n", __func__ );

    DCT_ASSERT( pMomCtrlCtx != NULL );

    /* create command queue */
    if ( OSLAYER_OK != osQueueInit( &pMomCtrlCtx->CommandQueue, pMomCtrlCtx->MaxCommands, sizeof(MomCtrlCmdId_t) ) )
    {
        TRACE( MOM_CTRL_ERROR, "%s (creating command queue (depth: %d) failed)\n", __func__, pMomCtrlCtx->MaxCommands );
        return ( RET_FAILURE );
    }

    /* create queue list and local objects 0..(Max-1) */
    uint32_t MaxBuffers = MAX( pMomCtrlCtx->NumBuffersMainPath, pMomCtrlCtx->NumBuffersSelfPath );
	MaxBuffers = MAX( MaxBuffers, pMomCtrlCtx->NumBuffersSelfPath2 );
    MaxBuffers = MAX( MaxBuffers, pMomCtrlCtx->NumBuffersRdiPath );
    MaxBuffers = MAX( MaxBuffers, pMomCtrlCtx->NumBuffersMetaPath );

    for (i = MOM_CTRL_PATH_INVALID; i<(MOM_CTRL_PATH_MAX-1); i++ )
    {
        ListInit( &(pMomCtrlCtx->PathQueues[i]) );
        if ( OSLAYER_OK != osMutexInit( &pMomCtrlCtx->PathLock[i] ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (creating path-lock (%d) failed)\n", __func__, (i-1));
            osQueueDestroy( &pMomCtrlCtx->CommandQueue );
            /* remove in reverse direction i..0 */
            while ( i > MOM_CTRL_PATH_INVALID )
            {
                (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i-1] );
                (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i-1] );
                (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i-1] );
                i--;
            }
            return ( RET_FAILURE );
        }

        /* create empty buffer queues */
        if ( OSLAYER_OK != osQueueInit( &pMomCtrlCtx->EmptyBufQueue[i], MaxBuffers, sizeof(MediaBuffer_t *) ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (creating empty buffer queue (depth: %d) failed)\n", __func__, MaxBuffers );
            osQueueDestroy( &pMomCtrlCtx->CommandQueue );
            /* remove in reverse direction i..MIN-1 */
            (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i] );
            while ( i > MOM_CTRL_PATH_INVALID )
            {
                (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i-1] );
                (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i-1] );
                (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i-1] );
                i--;
            }
            return ( RET_FAILURE );
        }

        /* create full buffer queues */
        if ( OSLAYER_OK != osQueueInit( &pMomCtrlCtx->FullBufQueue[i], MaxBuffers, sizeof(MediaBuffer_t *) ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (creating full buffer queue (depth: %d) failed)\n", __func__, MaxBuffers );
            osQueueDestroy( &pMomCtrlCtx->CommandQueue );
            /* remove in reverse direction i..MIN-1 */
            (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i] );
            (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i] );
            while ( i > MOM_CTRL_PATH_INVALID )
            {
                (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i-1] );
                (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i-1] );
                (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i-1] );
                i--;
            }
            return ( RET_FAILURE );
        }
    }

    /* register callback at main path buffer pool */
    if ( pMomCtrlCtx->pPicBufPoolMainPath != NULL )
    {
        if ( SCMI_RET_SUCCESS != MediaBufPoolRegisterCb( pMomCtrlCtx->pPicBufPoolMainPath, MomCtrlPictureBufferPoolNotifyCbMainPath, pMomCtrlCtx ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (can't register callback to buffer pool)\n", __func__ );
            osQueueDestroy( &pMomCtrlCtx->CommandQueue );
            for (i = MOM_CTRL_PATH_INVALID; i<(MOM_CTRL_PATH_MAX-1); i++ )
            {
                (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i] );
                (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i] );
                (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i] );
            }

            return ( RET_FAILURE );
        }
    }

    /* register callback at sp path buffer pool */
    if ( pMomCtrlCtx->pPicBufPoolSelfPath != NULL )
    {
        if ( SCMI_RET_SUCCESS != MediaBufPoolRegisterCb( pMomCtrlCtx->pPicBufPoolSelfPath, MomCtrlPictureBufferPoolNotifyCbSelfPath, pMomCtrlCtx ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (can't register callback to buffer pool)\n", __func__ );
            osQueueDestroy( &pMomCtrlCtx->CommandQueue );
            for (i = MOM_CTRL_PATH_INVALID; i<(MOM_CTRL_PATH_MAX-1); i++ )
            {
                (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i] );
                (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i] );
                (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i] );
            }
            if ( pMomCtrlCtx->pPicBufPoolMainPath != NULL )
            {
                MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolMainPath, MomCtrlPictureBufferPoolNotifyCbMainPath );
            }
            return ( RET_FAILURE );
        }
    }

	/* register callback at sp2 path buffer pool */
    if ( pMomCtrlCtx->pPicBufPoolSelfPath2 != NULL )
    {
        if ( SCMI_RET_SUCCESS != MediaBufPoolRegisterCb( pMomCtrlCtx->pPicBufPoolSelfPath2, MomCtrlPictureBufferPoolNotifyCbSelfPath2, pMomCtrlCtx ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (can't register callback to buffer pool)\n", __func__ );
            osQueueDestroy( &pMomCtrlCtx->CommandQueue );
            for (i = MOM_CTRL_PATH_INVALID; i<(MOM_CTRL_PATH_MAX-1); i++ )
            {
                (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i] );
                (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i] );
                (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i] );
            }

            if ( pMomCtrlCtx->pPicBufPoolMainPath != NULL )
            {
                MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolMainPath, MomCtrlPictureBufferPoolNotifyCbMainPath );
            }
            return ( RET_FAILURE );
        }
    }

    /* register callback at Rdi path buffer pool */
    if ( pMomCtrlCtx->pPicBufPoolRdiPath != NULL )
    {
        if ( SCMI_RET_SUCCESS != MediaBufPoolRegisterCb( pMomCtrlCtx->pPicBufPoolRdiPath, MomCtrlPictureBufferPoolNotifyCbRdiPath, pMomCtrlCtx ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (can't register callback to buffer pool)\n", __func__ );
            osQueueDestroy( &pMomCtrlCtx->CommandQueue );
            for (i = MOM_CTRL_PATH_INVALID; i<(MOM_CTRL_PATH_MAX-1); i++ )
            {
                (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i] );
                (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i] );
                (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i] );
            }

            return ( RET_FAILURE );
        }
    }

    /* register callback at Meta path buffer pool */
    if ( pMomCtrlCtx->pPicBufPoolMetaPath != NULL )
    {
        if ( SCMI_RET_SUCCESS != MediaBufPoolRegisterCb( pMomCtrlCtx->pPicBufPoolMetaPath, MomCtrlPictureBufferPoolNotifyCbMetaPath, pMomCtrlCtx ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (can't register callback to buffer pool)\n", __func__ );
            osQueueDestroy( &pMomCtrlCtx->CommandQueue );
            for (i = MOM_CTRL_PATH_INVALID; i<(MOM_CTRL_PATH_MAX-1); i++ )
            {
                (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i] );
                (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i] );
                (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i] );
            }

            return ( RET_FAILURE );
        }
    }

     /* register request callback at marvin driver */
    result = CamerIcMiRegisterRequestCb( pMomCtrlCtx->hCamerIc, MomCtrlCamerIcDrvRequestCb, (void *)pMomCtrlCtx );
    if ( result != RET_SUCCESS )
    {
        TRACE( MOM_CTRL_ERROR, "%s (can't register request-callback)\n", __func__ );
        osQueueDestroy( &pMomCtrlCtx->CommandQueue );
        for (i = MOM_CTRL_PATH_INVALID; i<(MOM_CTRL_PATH_MAX-1); i++ )
        {
            (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i] );
            (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i] );
            (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i] );
        }
        if ( pMomCtrlCtx->pPicBufPoolMainPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolMainPath, MomCtrlPictureBufferPoolNotifyCbMainPath );
        }
        if (  pMomCtrlCtx->pPicBufPoolSelfPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolSelfPath, MomCtrlPictureBufferPoolNotifyCbSelfPath );
        }
		if (  pMomCtrlCtx->pPicBufPoolSelfPath2 != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolSelfPath2, MomCtrlPictureBufferPoolNotifyCbSelfPath2 );
        }
        if (  pMomCtrlCtx->pPicBufPoolRdiPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolRdiPath, MomCtrlPictureBufferPoolNotifyCbRdiPath );
        }
        if (  pMomCtrlCtx->pPicBufPoolMetaPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolMetaPath, MomCtrlPictureBufferPoolNotifyCbMetaPath );
        }

        return ( result );
    }

    /* register event callback at marvin driver */
    result = CamerIcMiRegisterEventCb( pMomCtrlCtx->hCamerIc, MomCtrlCamerIcDrvEventCb, (void *)pMomCtrlCtx );
    if ( result != RET_SUCCESS )
    {
        TRACE( MOM_CTRL_ERROR, "%s (can't register event-callback)\n", __func__ );
        osQueueDestroy( &pMomCtrlCtx->CommandQueue );
        for (i = MOM_CTRL_PATH_INVALID; i<(MOM_CTRL_PATH_MAX-1); i++ )
        {
            (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i] );
            (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i] );
            (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i] );
        }
        if ( pMomCtrlCtx->pPicBufPoolMainPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolMainPath, MomCtrlPictureBufferPoolNotifyCbMainPath );
        }
        if (  pMomCtrlCtx->pPicBufPoolSelfPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolSelfPath, MomCtrlPictureBufferPoolNotifyCbSelfPath );
        }
		if (  pMomCtrlCtx->pPicBufPoolSelfPath2 != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolSelfPath2, MomCtrlPictureBufferPoolNotifyCbSelfPath2 );
        }
        if (  pMomCtrlCtx->pPicBufPoolRdiPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolRdiPath, MomCtrlPictureBufferPoolNotifyCbRdiPath );
        }
        if (  pMomCtrlCtx->pPicBufPoolMetaPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolMetaPath, MomCtrlPictureBufferPoolNotifyCbMetaPath );
        }

        (void)CamerIcMiDeRegisterRequestCb( pMomCtrlCtx->hCamerIc );

        return ( result );
    }

    /* create buffer lock */
    if ( OSLAYER_OK != osMutexInit( &pMomCtrlCtx->BufferLock ) )
    {
        TRACE( MOM_CTRL_ERROR, "%s (creating buffer-lock failed)\n", __func__ );
        osQueueDestroy( &pMomCtrlCtx->CommandQueue );
        for (i = MOM_CTRL_PATH_INVALID; i<(MOM_CTRL_PATH_MAX-1); i++ )
        {
            (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i] );
            (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i] );
            (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i] );
        }
        if ( pMomCtrlCtx->pPicBufPoolMainPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolMainPath, MomCtrlPictureBufferPoolNotifyCbMainPath );
        }
        if (  pMomCtrlCtx->pPicBufPoolSelfPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolSelfPath, MomCtrlPictureBufferPoolNotifyCbSelfPath );
        }
		if (  pMomCtrlCtx->pPicBufPoolSelfPath2 != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolSelfPath2, MomCtrlPictureBufferPoolNotifyCbSelfPath2 );
        }
        if (  pMomCtrlCtx->pPicBufPoolRdiPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolRdiPath, MomCtrlPictureBufferPoolNotifyCbRdiPath );
        }
        if (  pMomCtrlCtx->pPicBufPoolMetaPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolMetaPath, MomCtrlPictureBufferPoolNotifyCbMetaPath );
        }
        (void)CamerIcMiDeRegisterRequestCb( pMomCtrlCtx->hCamerIc );
        (void)CamerIcMiDeRegisterEventCb( pMomCtrlCtx->hCamerIc );

        return ( RET_FAILURE );
    }

    /* create handler thread */
    if ( OSLAYER_OK != osThreadCreate(&pMomCtrlCtx->Thread, MomCtrlThreadHandler, pMomCtrlCtx) )
    {
        TRACE( MOM_CTRL_ERROR, "%s (thread not created)\n", __func__);
        (void)osMutexDestroy( &pMomCtrlCtx->BufferLock );
        osQueueDestroy( &pMomCtrlCtx->CommandQueue );
        for (i = MOM_CTRL_PATH_INVALID; i<(MOM_CTRL_PATH_MAX-1); i++ )
        {
            (void)osMutexDestroy( &pMomCtrlCtx->PathLock[i] );
            (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i] );
            (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i] );
        }
        if ( pMomCtrlCtx->pPicBufPoolMainPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolMainPath, MomCtrlPictureBufferPoolNotifyCbMainPath );
        }
        if (  pMomCtrlCtx->pPicBufPoolSelfPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolSelfPath, MomCtrlPictureBufferPoolNotifyCbSelfPath );
        }
		if (  pMomCtrlCtx->pPicBufPoolSelfPath2 != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolSelfPath2, MomCtrlPictureBufferPoolNotifyCbSelfPath2 );
        }
        if (  pMomCtrlCtx->pPicBufPoolRdiPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolRdiPath, MomCtrlPictureBufferPoolNotifyCbRdiPath );
        }
        if (  pMomCtrlCtx->pPicBufPoolMetaPath != NULL )
        {
            MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolMetaPath, MomCtrlPictureBufferPoolNotifyCbMetaPath );
        }

        (void)CamerIcMiDeRegisterRequestCb( pMomCtrlCtx->hCamerIc );
        (void)CamerIcMiDeRegisterEventCb( pMomCtrlCtx->hCamerIc );

        return ( RET_FAILURE );
    }

    TRACE( MOM_CTRL_INFO, "%s (exit)\n", __func__ );

    return ( RET_SUCCESS );
}



/******************************************************************************
 * MomCtrlDestroy()
 *****************************************************************************/
RESULT MomCtrlDestroy
(
    MomCtrlContext_t *pMomCtrlCtx
)
{
    OSLAYER_STATUS osStatus;
    RESULT result;
    uint32_t i;

    TRACE( MOM_CTRL_INFO, "%s (enter)\n", __func__ );

    DCT_ASSERT( pMomCtrlCtx != NULL );

    /* send handler thread a shutdown command */
    result = MomCtrlSendCommand( pMomCtrlCtx, MOM_CTRL_CMD_SHUTDOWN );
    if ( result != RET_SUCCESS )
    {
        TRACE( MOM_CTRL_ERROR, "%s (send command failed -> RESULT=%d)\n", __func__, result);
    }

    /* wait for handler thread to have stopped due to the shutdown command given above */
    if ( OSLAYER_OK != osThreadWait( &pMomCtrlCtx->Thread ) )
    {
        TRACE( MOM_CTRL_ERROR, "%s (waiting for handler thread failed)\n", __func__);
    }

    /* destroy handler thread */
    if ( OSLAYER_OK != osThreadClose( &pMomCtrlCtx->Thread ) )
    {
        TRACE( MOM_CTRL_ERROR, "%s (closing handler thread failed)\n", __func__);
    }

    /* cancel any commands waiting in command queue */
    do
    {
        /* get next command from queue */
        MomCtrlCmdId_t Command;
        osStatus = osQueueTryRead( &pMomCtrlCtx->CommandQueue, &Command );

        switch (osStatus)
        {
            case OSLAYER_OK:        /* got a command, so cancel it */
                MomCtrlCompleteCommand( pMomCtrlCtx, Command, RET_CANCELED );
                break;
            case OSLAYER_TIMEOUT:   /* queue is empty */
                break;
            default:                /* something is broken... */
                UPDATE_RESULT( result, RET_FAILURE);
                break;
        }
    } while (osStatus == OSLAYER_OK);

    /* destroy command queue */
    if ( OSLAYER_OK != osQueueDestroy( &pMomCtrlCtx->CommandQueue ) )
    {
        TRACE( MOM_CTRL_ERROR, "%s (destroying command queue failed)\n", __func__ );
    }

    result = CamerIcMiDeRegisterRequestCb( pMomCtrlCtx->hCamerIc );
    DCT_ASSERT( result == RET_SUCCESS );

    result = CamerIcMiDeRegisterEventCb( pMomCtrlCtx->hCamerIc );
    DCT_ASSERT( result == RET_SUCCESS );

    /* disconnect from main path input pool */
    if ( pMomCtrlCtx->pPicBufPoolMainPath != NULL )
    {
        if ( OSLAYER_OK != MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolMainPath, MomCtrlPictureBufferPoolNotifyCbMainPath ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (disconnecting from media buffer pool failed)\n", __func__ );
        }
    }

    /* disconnect from self path input pool */
    if ( pMomCtrlCtx->pPicBufPoolSelfPath != NULL )
    {
        if ( OSLAYER_OK != MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolSelfPath, MomCtrlPictureBufferPoolNotifyCbSelfPath ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (disconnecting from media buffer pool failed)\n", __func__ );
        }
    }

	/* disconnect from self2 path input pool */
    if ( pMomCtrlCtx->pPicBufPoolSelfPath2 != NULL )
    {
        if ( OSLAYER_OK != MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolSelfPath2, MomCtrlPictureBufferPoolNotifyCbSelfPath2 ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (disconnecting from media buffer pool failed)\n", __func__ );
        }
    }

	/* disconnect from rdi path input pool */
    if ( pMomCtrlCtx->pPicBufPoolRdiPath != NULL )
    {
        if ( OSLAYER_OK != MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolSelfPath2, MomCtrlPictureBufferPoolNotifyCbRdiPath ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (disconnecting from media buffer pool failed)\n", __func__ );
        }
    }

	/* disconnect from rdi path input pool */
    if ( pMomCtrlCtx->pPicBufPoolMetaPath != NULL )
    {
        if ( OSLAYER_OK != MediaBufPoolDeregisterCb( pMomCtrlCtx->pPicBufPoolRdiPath, MomCtrlPictureBufferPoolNotifyCbMetaPath ) )
        {
            TRACE( MOM_CTRL_ERROR, "%s (disconnecting from media buffer pool failed)\n", __func__ );
        }
    }


    /* create queue list and local objects 0..(Max-1) */    for (i = MOM_CTRL_PATH_INVALID; i<(MOM_CTRL_PATH_MAX-1); i++ )
    {
        DCT_ASSERT( ListEmpty(&(pMomCtrlCtx->PathQueues[i])) );

        (void)osMutexDestroy(  &pMomCtrlCtx->PathLock[i] );
        (void)osQueueDestroy( &pMomCtrlCtx->EmptyBufQueue[i] );
        (void)osQueueDestroy( &pMomCtrlCtx->FullBufQueue[i] );
    }

    (void)osMutexDestroy( &pMomCtrlCtx->BufferLock );

    TRACE( MOM_CTRL_INFO, "%s (exit)\n", __func__ );

    return ( result );
}



/******************************************************************************
 * MomCtrlSendCommand()
 *****************************************************************************/
RESULT MomCtrlSendCommand
(
    MomCtrlContext_t    *pMomCtrlCtx,
    MomCtrlCmdId_t      Command
)
{
    TRACE( MOM_CTRL_INFO, "%s (enter)\n", __func__ );

    if( pMomCtrlCtx == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    /* are we shutting down? */
    if ( MomCtrlGetState( pMomCtrlCtx ) == eMomCtrlStateInvalid )
    {
        return ( RET_CANCELED );
    }

    /* send command */
    OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->CommandQueue, &Command );
    if ( osStatus != OSLAYER_OK )
    {
        TRACE(MOM_CTRL_ERROR, "%s (sending command to queue failed -> OSLAYER_STATUS=%d)\n", __func__, MomCtrlGetState( pMomCtrlCtx ), osStatus);
    }

    TRACE( MOM_CTRL_INFO, "%s (exit)\n", __func__ );

    return ( (osStatus == OSLAYER_OK) ? RET_SUCCESS : RET_FAILURE );
}



/******************************************************************************
 * MomCtrlCompleteCommand()
 *****************************************************************************/
void MomCtrlCompleteCommand
(
    MomCtrlContext_t    *pMomCtrlCtx,
    MomCtrlCmdId_t      Command,
    RESULT              result
)
{
    DCT_ASSERT( pMomCtrlCtx != NULL );
    DCT_ASSERT( pMomCtrlCtx->momCbCompletion != NULL );

    pMomCtrlCtx->momCbCompletion( Command, result, pMomCtrlCtx->pUserContext );
}

