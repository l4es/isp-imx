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
 * @file mom_ctrl_api.c
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

#include <bufferpool/media_buffer.h>
#include <bufferpool/media_buffer_pool.h>
#include <bufferpool/media_buffer_queue_ex.h>

#include "mom_ctrl.h"
#include "mom_ctrl_api.h"



/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER( MOM_CTRL_API_INFO , "MOM-CTRL-API: ", INFO,    0 );
CREATE_TRACER( MOM_CTRL_API_WAR,   "MOM-CTRL-API: ", WARNING, 1 );
CREATE_TRACER( MOM_CTRL_API_ERROR, "MOM-CTRL-API: ", ERROR,   1 );



/******************************************************************************
 * MomCtrlInit()
 *****************************************************************************/
RESULT MomCtrlInit
(
    MomCtrlConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    MomCtrlContext_t *pMomCtrlCtx;

    TRACE( MOM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if ( (pConfig == NULL)
            || ( (pConfig->pPicBufPoolMainPath == NULL) && (pConfig->pPicBufPoolSelfPath == NULL)
				&& (pConfig->pPicBufPoolSelfPath2 == NULL))
            || (pConfig->momCbCompletion == NULL) )  //rdi and meta can be null
    {
        return ( RET_INVALID_PARM );
    }

    if ( pConfig->hCamerIc == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pConfig->MaxPendingCommands == 0 )
    {
        return ( RET_OUTOFRANGE );
    }

    /* allocate control context */
    pMomCtrlCtx = malloc( sizeof(MomCtrlContext_t) );
    if ( pMomCtrlCtx == NULL )
    {
        TRACE( MOM_CTRL_API_ERROR, "%s (allocating control context failed)\n", __func__ );
        return ( RET_OUTOFMEM );
    }
    memset( pMomCtrlCtx, 0, sizeof(MomCtrlContext_t) );

    /* pre initialize control context */
    pMomCtrlCtx->State                  = eMomCtrlStateInvalid;
    pMomCtrlCtx->MaxCommands            = pConfig->MaxPendingCommands;
    pMomCtrlCtx->NumBuffersMainPath     = ( pConfig->NumBuffersMainPath == 0 ) ? 10 : pConfig->NumBuffersMainPath;
    pMomCtrlCtx->NumBuffersSelfPath     = ( pConfig->NumBuffersSelfPath == 0 ) ? 10 : pConfig->NumBuffersSelfPath;
	pMomCtrlCtx->NumBuffersSelfPath2    = ( pConfig->NumBuffersSelfPath2 == 0 ) ? 10 : pConfig->NumBuffersSelfPath2;
    pMomCtrlCtx->NumBuffersRdiPath      = ( pConfig->NumBuffersRdiPath == 0 ) ? 10 : pConfig->NumBuffersRdiPath;
    pMomCtrlCtx->NumBuffersMetaPath     = ( pConfig->NumBuffersMetaPath == 0 ) ? 10 : pConfig->NumBuffersMetaPath;

    pMomCtrlCtx->pPicBufPoolMainPath    = pConfig->pPicBufPoolMainPath;
    pMomCtrlCtx->pPicBufPoolSelfPath    = pConfig->pPicBufPoolSelfPath;
	pMomCtrlCtx->pPicBufPoolSelfPath2   = pConfig->pPicBufPoolSelfPath2;
    pMomCtrlCtx->pPicBufPoolRdiPath    = pConfig->pPicBufPoolRdiPath;
	pMomCtrlCtx->pPicBufPoolMetaPath   = pConfig->pPicBufPoolMetaPath;

    pMomCtrlCtx->momCbCompletion        = pConfig->momCbCompletion;
    pMomCtrlCtx->pUserContext           = pConfig->pUserContext;
    pMomCtrlCtx->HalHandle              = pConfig->HalHandle;
    pMomCtrlCtx->hCamerIc               = pConfig->hCamerIc;

    /* create control process */
    result = MomCtrlCreate( pMomCtrlCtx );
    if ( result != RET_SUCCESS )
    {
        TRACE( MOM_CTRL_API_ERROR, "%s (creating control process failed)\n", __func__ );
        free( pMomCtrlCtx );
    }
    else
    {
        /* control context is initilized, we're ready and in idle state */
        pMomCtrlCtx->State = eMomCtrlStateInitialize;

        /* success, so let's return control context */
        pConfig->hMomContext = (MomCtrlContextHandle_t)pMomCtrlCtx;
    }

    TRACE( MOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( result );
}



/******************************************************************************
 * MomCtrlStart()
 *****************************************************************************/
RESULT MomCtrlStart
(
    MomCtrlContextHandle_t hMomContext
)
{
    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)hMomContext;

    TRACE( MOM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if( pMomCtrlCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateInitialize )
            && ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateStopped ) )
    {
        return ( RET_WRONG_STATE );
    }

    MomCtrlSendCommand( pMomCtrlCtx, MOM_CTRL_CMD_START );

    TRACE( MOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( RET_PENDING );
}



/******************************************************************************
 * MomCtrlStop()
 *****************************************************************************/
RESULT MomCtrlStop
(
    MomCtrlContextHandle_t hMomContext
)
{
    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)hMomContext;

    TRACE( MOM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if( pMomCtrlCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateRunning )
            && ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateStopped ) )
    {
        return ( RET_WRONG_STATE );
    }

    MomCtrlSendCommand( pMomCtrlCtx, MOM_CTRL_CMD_STOP );

    TRACE( MOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( RET_PENDING );
}



/******************************************************************************
 * MomCtrlShutDown()
 *****************************************************************************/
RESULT MomCtrlShutDown
(
    MomCtrlContextHandle_t hMomContext
)
{
    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)hMomContext;

    RESULT result = RET_SUCCESS;

    TRACE( MOM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if( pMomCtrlCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateInitialize )
            && ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateStopped ) )
    {
        return ( RET_WRONG_STATE );
    }

    result = MomCtrlDestroy( pMomCtrlCtx );
    if ( result != RET_SUCCESS )
    {
        TRACE( MOM_CTRL_API_ERROR, "%s (destroying control process failed -> RESULT=%d)\n", __func__, result);
    }

    /* release context memory */
    free( pMomCtrlCtx );

    TRACE( MOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( result );
}



/******************************************************************************
 * MomCtrlAttachQueueToPath()
 *****************************************************************************/
RESULT MomCtrlAttachQueueToPath
(
    MomCtrlContextHandle_t  hMomContext,
    const uint32_t          path,
    osQueue                 *pQueue
)
{
    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)hMomContext;

    TRACE( MOM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if( pMomCtrlCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateInitialize )
            && ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateRunning )
            && ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateStopped ) )
    {
        return ( RET_WRONG_STATE );
    }

    if ( (pQueue != NULL)
            && (path > MOM_CTRL_PATH_INVALID) && (path < MOM_CTRL_PATH_MAX) )
    {
        osMutex *pMutex = &(pMomCtrlCtx->PathLock[(path-1)]);
        List *pList = &(pMomCtrlCtx->PathQueues[(path-1)]);

        osMutexLock( pMutex );
        ListAddTail( pList, ((void *)pQueue) );
        osMutexUnlock( pMutex );
    }
    else
    {
        return ( RET_INVALID_PARM );
    }

    TRACE( MOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( RET_SUCCESS );
}


/******************************************************************************
 * MomCtrlDetachQueueToPath()
 *****************************************************************************/
static int FindQueue( List *pList, void *key )
{
    return ( (pList == key) ? 1 : 0 );
}

RESULT MomCtrlDetachQueueToPath
(
    MomCtrlContextHandle_t  hMomContext,
    const uint32_t          path,
    osQueue                 *pQueue
)
{
    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)hMomContext;

    TRACE( MOM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if( pMomCtrlCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateInitialize )
            && ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateRunning )
            && ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateStopped ) )
    {
        return ( RET_WRONG_STATE );
    }

    if ( (pQueue != NULL)
            && (path > MOM_CTRL_PATH_INVALID) && (path < MOM_CTRL_PATH_MAX) )
    {
        osMutex *pMutex = &(pMomCtrlCtx->PathLock[(path-1)]);
        List *pList = &(pMomCtrlCtx->PathQueues[(path-1)]);

        osMutexLock( pMutex );
        ListRemoveItem( pList, FindQueue, ((void *)pQueue) );
        osMutexUnlock( pMutex );
    }
    else
    {
        return ( RET_INVALID_PARM );
    }

    TRACE( MOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( RET_SUCCESS );
}



/******************************************************************************
 * MomCtrlRegisterBufferCb()
 *****************************************************************************/
RESULT  MomCtrlRegisterBufferCb
(
    MomCtrlContextHandle_t  hMomContext,
    const uint32_t          path,
    MomCtrlBufferCb_t       fpCallback,
    void                    *pBufferCbCtx
)
{
    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)hMomContext;

    TRACE( MOM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if( pMomCtrlCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if( fpCallback == NULL )
    {
        return RET_NULL_POINTER;
    }

    if ( ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateInitialize )
        && ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateRunning    )
        && ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateStopped    ) )
    {
        return ( RET_WRONG_STATE );
    }

    if ( (path <= MOM_CTRL_PATH_INVALID) || (path >= MOM_CTRL_PATH_MAX) )
    {
        return ( RET_INVALID_PARM );
    }

    osMutexLock( &pMomCtrlCtx->BufferLock );

    if ( MOM_CTRL_PATH_MAINPATH == path ) {
        pMomCtrlCtx->BufferCbMainPath.fpCallback   = fpCallback;
        pMomCtrlCtx->BufferCbMainPath.pBufferCbCtx = pBufferCbCtx;
    } else if ( MOM_CTRL_PATH_SELFPATH == path ) {
        pMomCtrlCtx->BufferCbSelfPath.fpCallback   = fpCallback;
        pMomCtrlCtx->BufferCbSelfPath.pBufferCbCtx = pBufferCbCtx;
    } else if(MOM_CTRL_PATH_SELFPATH2 == path) {
        pMomCtrlCtx->BufferCbSelfPath2.fpCallback   = fpCallback;
        pMomCtrlCtx->BufferCbSelfPath2.pBufferCbCtx = pBufferCbCtx;
    } else if(MOM_CTRL_PATH_RDIPATH == path) {
        pMomCtrlCtx->BufferCbRdiPath.fpCallback   = fpCallback;
        pMomCtrlCtx->BufferCbRdiPath.pBufferCbCtx = pBufferCbCtx;
    } else if(MOM_CTRL_PATH_METAPATH == path) {
        pMomCtrlCtx->BufferCbMetaPath.fpCallback   = fpCallback;
        pMomCtrlCtx->BufferCbMetaPath.pBufferCbCtx = pBufferCbCtx;
    } else {
        TRACE( MOM_CTRL_API_ERROR, "%s Wrong Path %d Config, exit\n", __func__, path );
        return ( RET_WRONG_CONFIG );
    }

    osMutexUnlock( &pMomCtrlCtx->BufferLock );

    TRACE( MOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( RET_SUCCESS );
}


/******************************************************************************
 * MomCtrlDeRegisterBufferCb()
 *****************************************************************************/
RESULT  MomCtrlDeRegisterBufferCb
(
    MomCtrlContextHandle_t  hMomContext,
    const uint32_t          path
)
{
    MomCtrlContext_t *pMomCtrlCtx = (MomCtrlContext_t *)hMomContext;

    TRACE( MOM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if( pMomCtrlCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateInitialize )
      && ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateRunning    )
      && ( MomCtrlGetState( pMomCtrlCtx ) != eMomCtrlStateStopped    ) )
    {
        return ( RET_WRONG_STATE );
    }

    if ( (path <= MOM_CTRL_PATH_INVALID) || (path >= MOM_CTRL_PATH_MAX) )
    {
        return ( RET_INVALID_PARM );
    }

    osMutexLock( &pMomCtrlCtx->BufferLock );

    if ( MOM_CTRL_PATH_MAINPATH == path ) {
        pMomCtrlCtx->BufferCbMainPath.fpCallback   = NULL;
        pMomCtrlCtx->BufferCbMainPath.pBufferCbCtx = NULL;
    } else if ( MOM_CTRL_PATH_SELFPATH == path ) {
        pMomCtrlCtx->BufferCbSelfPath.fpCallback   = NULL;
        pMomCtrlCtx->BufferCbSelfPath.pBufferCbCtx = NULL;
    } else if(MOM_CTRL_PATH_SELFPATH2 == path){
		pMomCtrlCtx->BufferCbSelfPath2.fpCallback   = NULL;
        pMomCtrlCtx->BufferCbSelfPath2.pBufferCbCtx = NULL;
	} else if(MOM_CTRL_PATH_RDIPATH == path){
		pMomCtrlCtx->BufferCbRdiPath.fpCallback   = NULL;
        pMomCtrlCtx->BufferCbRdiPath.pBufferCbCtx = NULL;
	} else if(MOM_CTRL_PATH_METAPATH == path){
		pMomCtrlCtx->BufferCbMetaPath.fpCallback   = NULL;
        pMomCtrlCtx->BufferCbMetaPath.pBufferCbCtx = NULL;
	} else{
        TRACE( MOM_CTRL_API_ERROR, "%s Wrong Path %d Config, exit\n", __func__, path );
        return ( RET_WRONG_CONFIG );
    }

    osMutexUnlock( &pMomCtrlCtx->BufferLock );

    TRACE( MOM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( RET_SUCCESS );
}


