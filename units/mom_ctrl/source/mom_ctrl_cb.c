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
 * @mom_ctrl.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/

#include <ebase/trace.h>
#include <common/return_codes.h>

#include <bufferpool/media_buffer.h>
#include <bufferpool/media_buffer_pool.h>
#include <bufferpool/media_buffer_queue_ex.h>

#include "mom_ctrl_cb.h"
#include "mom_ctrl.h"


/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER( MOM_CTRL_CB_INFO , "MOM-CTRL-CB: ", INFO      , 0 );
CREATE_TRACER( MOM_CTRL_CB_WARN , "MOM-CTRL-CB: ", WARNING   , 1 );
CREATE_TRACER( MOM_CTRL_CB_ERROR, "MOM-CTRL-CB: ", ERROR     , 1 );

CREATE_TRACER( MOM_CTRL_CB_DEBUG, "", INFO, 1 );


#define container_of(ptr, type, member) \
    ((type *)(((uintptr_t)(uint8_t *)(ptr)) - (uintptr_t)(&((type *)0)->member)))


#define CAMERIC_MI_REQUEST_GET_EMPTY_BUFFER_TIMEOUT 0



/******************************************************************************
 * MomCtrlPictureBufferPoolNotifyCbMainPath()
 *****************************************************************************/
void MomCtrlPictureBufferPoolNotifyCbMainPath
(
    MediaBufQueueExEvent_t  event,
    void                    *pUserContext,
    const MediaBuffer_t     *pMediaBuffer
)
{
    TRACE( MOM_CTRL_CB_INFO, "%s (enter %p %d)\n", __func__, pUserContext, event );

    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)pUserContext;

    if ( (pMomCtrlCtx != NULL) && (MomCtrlGetState( pMomCtrlCtx ) == eMomCtrlStateRunning) )
    {
        switch ( event )
        {
            case EMPTY_BUFFER_ADDED:
                {
                    MediaBuffer_t *pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolMainPath );
                    if ( pBuffer != NULL )
                    {
                        OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_MAINPATH-1], &pBuffer );
                        DCT_ASSERT( osStatus == OSLAYER_OK );
                    }

                    break;
                }

            default:
                {
                    break;
                }
        }
    }

    TRACE( MOM_CTRL_CB_INFO, "%s (exit)\n", __func__ );
}



/******************************************************************************
 * MomCtrlPictureBufferPoolNotifyCbSelfPath()
 *****************************************************************************/
void MomCtrlPictureBufferPoolNotifyCbSelfPath
(
    MediaBufQueueExEvent_t  event,
    void                    *pUserContext,
    const MediaBuffer_t     *pMediaBuffer
)
{
    TRACE( MOM_CTRL_CB_INFO, "%s (enter %p %d)\n", __func__, pUserContext, event );

    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)pUserContext;
    if ( (pMomCtrlCtx != NULL) && (MomCtrlGetState( pMomCtrlCtx ) == eMomCtrlStateRunning) )
    {
        switch ( event )
        {
            case EMPTY_BUFFER_ADDED:
                {
                    MediaBuffer_t *pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolSelfPath );
                    if ( pBuffer != NULL )
                    {
                        OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_SELFPATH-1], &pBuffer );
                        DCT_ASSERT( osStatus == OSLAYER_OK );
                    }
                    else
                    {
                    }

                    break;
                }

            default:
                {
                    break;
                }
        }
    }

    TRACE( MOM_CTRL_CB_INFO, "%s (exit)\n", __func__ );
}


/******************************************************************************
 * MomCtrlPictureBufferPoolNotifyCbSelfPath()
 *****************************************************************************/
void MomCtrlPictureBufferPoolNotifyCbSelfPath2
(
    MediaBufQueueExEvent_t  event,
    void                    *pUserContext,
    const MediaBuffer_t     *pMediaBuffer
)
{
    TRACE( MOM_CTRL_CB_INFO, "%s (enter %p %d)\n", __func__, pUserContext, event );

    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)pUserContext;
    if ( (pMomCtrlCtx != NULL) && (MomCtrlGetState( pMomCtrlCtx ) == eMomCtrlStateRunning) )
    {
        switch ( event )
        {
            case EMPTY_BUFFER_ADDED:
                {
                    MediaBuffer_t *pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolSelfPath2 );
                    if ( pBuffer != NULL )
                    {
                        OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_SELFPATH2-1], &pBuffer );
                        DCT_ASSERT( osStatus == OSLAYER_OK );
                    }
                    else
                    {
                    }

                    break;
                }

            default:
                {
                    break;
                }
        }
    }

    TRACE( MOM_CTRL_CB_INFO, "%s (exit)\n", __func__ );
}



/******************************************************************************
 * MomCtrlPictureBufferPoolNotifyCbRdiPath()
 *****************************************************************************/
void MomCtrlPictureBufferPoolNotifyCbRdiPath
(
    MediaBufQueueExEvent_t  event,
    void                    *pUserContext,
    const MediaBuffer_t     *pMediaBuffer
)
{
    TRACE( MOM_CTRL_CB_INFO, "%s (enter %p %d)\n", __func__, pUserContext, event );

    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)pUserContext;

    if ( (pMomCtrlCtx != NULL) && (MomCtrlGetState( pMomCtrlCtx ) == eMomCtrlStateRunning) )
    {
        switch ( event )
        {
            case EMPTY_BUFFER_ADDED:
                {
                    MediaBuffer_t *pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolRdiPath );
                    if ( pBuffer != NULL )
                    {
                        OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_RDIPATH-1], &pBuffer );
                        DCT_ASSERT( osStatus == OSLAYER_OK );
                    }

                    break;
                }

            default:
                {
                    break;
                }
        }
    }

    TRACE( MOM_CTRL_CB_INFO, "%s (exit)\n", __func__ );
}


/******************************************************************************
 * MomCtrlPictureBufferPoolNotifyCbMetaPath()
 *****************************************************************************/
void MomCtrlPictureBufferPoolNotifyCbMetaPath
(
    MediaBufQueueExEvent_t  event,
    void                    *pUserContext,
    const MediaBuffer_t     *pMediaBuffer
)
{
    TRACE( MOM_CTRL_CB_INFO, "%s (enter %p %d)\n", __func__, pUserContext, event );

    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)pUserContext;

    if ( (pMomCtrlCtx != NULL) && (MomCtrlGetState( pMomCtrlCtx ) == eMomCtrlStateRunning) )
    {
        switch ( event )
        {
            case EMPTY_BUFFER_ADDED:
                {
                    MediaBuffer_t *pBuffer = MediaBufPoolGetBuffer( pMomCtrlCtx->pPicBufPoolMetaPath );
                    if ( pBuffer != NULL )
                    {
                        OSLAYER_STATUS osStatus = osQueueWrite( &pMomCtrlCtx->EmptyBufQueue[MOM_CTRL_PATH_METAPATH-1], &pBuffer );
                        DCT_ASSERT( osStatus == OSLAYER_OK );
                    }

                    break;
                }

            default:
                {
                    break;
                }
        }
    }

    TRACE( MOM_CTRL_CB_INFO, "%s (exit)\n", __func__ );
}


/******************************************************************************
 * MomCtrlCamerIcDrvRequestCb()
 *****************************************************************************/
RESULT MomCtrlCamerIcDrvRequestCb
(
    const uint32_t request,
    void **param,
    void *pUserContext
)
{
    RESULT result = RET_FAILURE;

    TRACE( MOM_CTRL_CB_INFO, "%s (enter)\n", __func__ );
    if ( (pUserContext != NULL) && ( param != NULL) )
    {
        MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)pUserContext;
        if ( MomCtrlGetState( pMomCtrlCtx ) == eMomCtrlStateRunning )
        {
            switch ( request )
            {
                case CAMERIC_MI_REQUEST_GET_EMPTY_MP_BUFFER:
                case CAMERIC_MI_REQUEST_GET_EMPTY_SP_BUFFER:
                case CAMERIC_MI_REQUEST_GET_EMPTY_SP2_BP_BUFFER:
                case CAMERIC_MI_REQUEST_GET_EMPTY_RDI_BUFFER:
                case CAMERIC_MI_REQUEST_GET_EMPTY_META_BUFFER:
                    {
                        MediaBuffer_t *pBuffer = NULL;
                        OSLAYER_STATUS osStatus;
                        uint32_t path;
                        path = ( request == CAMERIC_MI_REQUEST_GET_EMPTY_MP_BUFFER ) ? MOM_CTRL_PATH_MAINPATH :
							    ( request == CAMERIC_MI_REQUEST_GET_EMPTY_SP_BUFFER ) ? MOM_CTRL_PATH_SELFPATH :
                                ( request == CAMERIC_MI_REQUEST_GET_EMPTY_SP2_BP_BUFFER ) ? MOM_CTRL_PATH_SELFPATH2 :
                                ( request == CAMERIC_MI_REQUEST_GET_EMPTY_RDI_BUFFER ) ? MOM_CTRL_PATH_RDIPATH :
                                MOM_CTRL_PATH_METAPATH;

                        osStatus = osQueueTimedRead( &pMomCtrlCtx->EmptyBufQueue[path-1], &pBuffer, CAMERIC_MI_REQUEST_GET_EMPTY_BUFFER_TIMEOUT );
                        // osStatus = osQueueRead( &pMomCtrlCtx->EmptyBufQueue[path-1], &pBuffer );
                        if ( (pBuffer != NULL) && (osStatus == OSLAYER_OK) )
                        {
                            TRACE( MOM_CTRL_CB_INFO, "%s (req buffer %p)\n", __func__, pBuffer);
                            *param = &(pBuffer->buf);
                            pBuffer->pMetaData = pBuffer->buf.p_meta_data;
                            result = RET_SUCCESS;
                        }
                        else
                        {
                            TRACE( MOM_CTRL_CB_INFO, "%s (req buffer timed out)\n", __func__);
                            *param = NULL;
                            result = RET_NOTAVAILABLE;
                        }

                        break;
                    }

                default:
                    {
                        break;
                    }
            }
        }
        else
        {
            result = RET_WRONG_STATE;
        }
    }

    TRACE( MOM_CTRL_CB_INFO, "%s (exit res=%d)\n", __func__, result );

    return ( result );
}



/******************************************************************************
 * MomCtrlCamerIcDrvEventCb()
 *****************************************************************************/
void MomCtrlCamerIcDrvEventCb
(
    const uint32_t  event,
    void            *param,
    void            *pUserContext
)
{
    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)pUserContext;

    TRACE( MOM_CTRL_CB_INFO, "%s (enter)\n", __func__ );

    if (pMomCtrlCtx != NULL)
    {

        if ( MomCtrlGetState( pMomCtrlCtx ) == eMomCtrlStateRunning )
        {
            switch ( event )
            {

                case CAMERIC_MI_EVENT_FULL_MP_BUFFER:
                    {
                    	DCT_ASSERT( NULL != param );

                        OSLAYER_STATUS osStatus;

                        ScmiBuffer      *scmiBuffer = (ScmiBuffer *)param;
                        MediaBuffer_t   *pBuffer = container_of(scmiBuffer, MediaBuffer_t, buf);
                        PicBufMetaData_t *pMetaData = (PicBufMetaData_t *)pBuffer->pMetaData;

                        osStatus = osTimeStampUs( &pMetaData->TimeStampUs );
                        DCT_ASSERT( osStatus == OSLAYER_OK );

                        osStatus = osQueueWrite( &pMomCtrlCtx->FullBufQueue[MOM_CTRL_PATH_MAINPATH-1], &pBuffer );
                        DCT_ASSERT( osStatus == OSLAYER_OK );

                        MomCtrlSendCommand( pMomCtrlCtx, MOM_CTRL_CMD_PROC_FULL_BUFFER_MAINPATH );

                        break;
                    }

                case CAMERIC_MI_EVENT_FLUSHED_MP_BUFFER:
                    {
                    	DCT_ASSERT( NULL != param );

                        /* mark as empty */
                        ScmiBuffer      *scmiBuffer = (ScmiBuffer *)param;
                        MediaBuffer_t   *pBuffer = container_of(scmiBuffer, MediaBuffer_t, buf);
                        MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolMainPath, pBuffer );
                        pBuffer->buf.size = pBuffer->baseSize;

                        break;
                    }

                case CAMERIC_MI_EVENT_DROPPED_MP_BUFFER:
                    {
                        TRACE( MOM_CTRL_CB_ERROR, "%s (MP buffer dropped)\n", __func__ );

                        break;
                    }

                case CAMERIC_MI_EVENT_FULL_SP_BUFFER:
                    {
                    	DCT_ASSERT( NULL != param );

                        OSLAYER_STATUS osStatus;

                        ScmiBuffer      *scmiBuffer = (ScmiBuffer *)param;
                        MediaBuffer_t   *pBuffer = container_of(scmiBuffer, MediaBuffer_t, buf);
                        PicBufMetaData_t *pMetaData = (PicBufMetaData_t *)pBuffer->pMetaData;

                        osStatus = osTimeStampUs( &pMetaData->TimeStampUs );
                        DCT_ASSERT( osStatus == OSLAYER_OK );

                        osStatus = osQueueWrite( &pMomCtrlCtx->FullBufQueue[MOM_CTRL_PATH_SELFPATH-1], &pBuffer );
                        DCT_ASSERT( osStatus == OSLAYER_OK );

                        MomCtrlSendCommand( pMomCtrlCtx, MOM_CTRL_CMD_PROC_FULL_BUFFER_SELFPATH );

                        break;
                    }
				case CAMERIC_MI_EVENT_FULL_SP2_BP_BUFFER:
                    {
                    	DCT_ASSERT( NULL != param );

                        OSLAYER_STATUS osStatus;
                        ScmiBuffer      *scmiBuffer = (ScmiBuffer *)param;
                        MediaBuffer_t   *pBuffer = container_of(scmiBuffer, MediaBuffer_t, buf);
                        PicBufMetaData_t *pMetaData = (PicBufMetaData_t *)pBuffer->pMetaData;

                        osStatus = osTimeStampUs( &pMetaData->TimeStampUs );
                        DCT_ASSERT( osStatus == OSLAYER_OK );

                        osStatus = osQueueWrite( &pMomCtrlCtx->FullBufQueue[MOM_CTRL_PATH_SELFPATH2-1], &pBuffer );
                        DCT_ASSERT( osStatus == OSLAYER_OK );

                        MomCtrlSendCommand( pMomCtrlCtx, MOM_CTRL_CMD_PROC_FULL_BUFFER_SELFPATH2 );

                        break;
                    }
                case CAMERIC_MI_EVENT_FLUSHED_SP_BUFFER:
                    {
                    	DCT_ASSERT( NULL != param );

                        /* mark as empty */
                        ScmiBuffer      *scmiBuffer = (ScmiBuffer *)param;
                        MediaBuffer_t   *pBuffer = container_of(scmiBuffer, MediaBuffer_t, buf);
                        MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolSelfPath, pBuffer );
                        pBuffer->buf.size = pBuffer->baseSize;

                        break;
                    }

                case CAMERIC_MI_EVENT_DROPPED_SP_BUFFER:
                    {
                        TRACE( MOM_CTRL_CB_ERROR, "%s (SP buffer dropped)\n", __func__ );
                        break;
                    }

		case CAMERIC_MI_EVENT_FLUSHED_SP2_BP_BUFFER:
                    {
                    	DCT_ASSERT( NULL != param );

                        /* mark as empty */
                        ScmiBuffer      *scmiBuffer = (ScmiBuffer *)param;
                        MediaBuffer_t   *pBuffer = container_of(scmiBuffer, MediaBuffer_t, buf);
                        MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolSelfPath2, pBuffer );
                        pBuffer->buf.size = pBuffer->baseSize;

                        break;
                    }

                case CAMERIC_MI_EVENT_DROPPED_SP2_BP_BUFFER:
                    {
                        TRACE( MOM_CTRL_CB_ERROR, "%s (SP buffer dropped)\n", __func__ );
                        break;
                    }

                case CAMERIC_MI_EVENT_FULL_RDI_BUFFER:
                    {
                    	DCT_ASSERT( NULL != param );

                        OSLAYER_STATUS osStatus;

                        ScmiBuffer      *scmiBuffer = (ScmiBuffer *)param;
                        MediaBuffer_t   *pBuffer = container_of(scmiBuffer, MediaBuffer_t, buf);
                        PicBufMetaData_t *pMetaData = (PicBufMetaData_t *)pBuffer->pMetaData;

                        osStatus = osTimeStampUs( &pMetaData->TimeStampUs );
                        DCT_ASSERT( osStatus == OSLAYER_OK );

                        osStatus = osQueueWrite( &pMomCtrlCtx->FullBufQueue[MOM_CTRL_PATH_RDIPATH-1], &pBuffer );
                        DCT_ASSERT( osStatus == OSLAYER_OK );

                        MomCtrlSendCommand( pMomCtrlCtx, MOM_CTRL_CMD_PROC_FULL_BUFFER_RDIPATH );

                        break;
                    }

                case CAMERIC_MI_EVENT_FLUSHED_RDI_BUFFER:
                    {
                    	DCT_ASSERT( NULL != param );

                        /* mark as empty */
                        ScmiBuffer      *scmiBuffer = (ScmiBuffer *)param;
                        MediaBuffer_t   *pBuffer = container_of(scmiBuffer, MediaBuffer_t, buf);
                        MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolRdiPath, pBuffer );
                        pBuffer->buf.size = pBuffer->baseSize;

                        break;
                    }

                case CAMERIC_MI_EVENT_DROPPED_RDI_BUFFER:
                    {
                        TRACE( MOM_CTRL_CB_ERROR, "%s (RDI buffer dropped)\n", __func__ );

                        break;
                    }

                case CAMERIC_MI_EVENT_FULL_META_BUFFER:
                    {
                    	DCT_ASSERT( NULL != param );

                        OSLAYER_STATUS osStatus;

                        ScmiBuffer      *scmiBuffer = (ScmiBuffer *)param;
                        MediaBuffer_t   *pBuffer = container_of(scmiBuffer, MediaBuffer_t, buf);
                        PicBufMetaData_t *pMetaData = (PicBufMetaData_t *)pBuffer->pMetaData;

                        osStatus = osTimeStampUs( &pMetaData->TimeStampUs );
                        DCT_ASSERT( osStatus == OSLAYER_OK );

                        osStatus = osQueueWrite( &pMomCtrlCtx->FullBufQueue[MOM_CTRL_PATH_METAPATH-1], &pBuffer );
                        DCT_ASSERT( osStatus == OSLAYER_OK );

                        MomCtrlSendCommand( pMomCtrlCtx, MOM_CTRL_CMD_PROC_FULL_BUFFER_METAPATH );

                        break;
                    }

                case CAMERIC_MI_EVENT_FLUSHED_META_BUFFER:
                    {
                    	DCT_ASSERT( NULL != param );

                        /* mark as empty */
                        ScmiBuffer      *scmiBuffer = (ScmiBuffer *)param;
                        MediaBuffer_t   *pBuffer = container_of(scmiBuffer, MediaBuffer_t, buf);
                        MediaBufPoolFreeBuffer( pMomCtrlCtx->pPicBufPoolMetaPath, pBuffer );
                        pBuffer->buf.size = pBuffer->baseSize;

                        break;
                    }

                case CAMERIC_MI_EVENT_DROPPED_META_BUFFER:
                    {
                        TRACE( MOM_CTRL_CB_ERROR, "%s (META buffer dropped)\n", __func__ );

                        break;
                    }

                default:
                    {
                        TRACE( MOM_CTRL_CB_ERROR, "%s unknown event\n", __func__ );
                        break;
                    }
            }
        }
        else
        {
            TRACE( MOM_CTRL_CB_ERROR, "%s (wrong state)\n", __func__ );
        }
    }

    TRACE( MOM_CTRL_CB_INFO, "%s (exit)\n", __func__ );
}

