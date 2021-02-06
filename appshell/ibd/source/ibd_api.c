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
 * @file ibd_api.c
 *
 * @brief
 *   Implementation of ibd API.
 *
 *****************************************************************************/
/**
 * @page ibd_page IBD
 * The In-Buffer Display module allows to perform some simple graphics stuff on image buffers.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref ibd_api
 * - @ref ibd_common
 * - @ref ibd
 *
 */


#include "ibd.h"
#include "ibd_api.h"
#include <log.h>

#define LOGTAG "IBD"


extern RESULT PicBufIsConfigValid( PicBufMetaData_t *pPicBufMetaData);

/***** local functions ***********************************************/

/***** API implementation ***********************************************/

/******************************************************************************
 * ibdOpenMapped()
 *****************************************************************************/
ibdHandle_t ibdOpenMapped
(
    HalHandle_t     halHandle,
    MediaBuffer_t   *pBuffer
)
{
    RESULT result = RET_FAILURE;
    ibdContext_t *pibdContext = NULL;

    ALOGI("IBD-API:%s (enter)\n", __func__ );

    if ( (halHandle == NULL) || (pBuffer == NULL) )
    {
       ALOGE("IBD-API:%s RET_NULL_POINTER\n", __func__ );
        return NULL;
    }

    // get meta data
    PicBufMetaData_t *pPicBufMetaData = (PicBufMetaData_t *)(pBuffer->pMetaData);
    if ( pPicBufMetaData == NULL )
    {
       ALOGE("IBD-API:%s RET_INVALID_PARM\n", __func__ );
        return NULL;
    }

    // pre-check buffer meta data
    result = PicBufIsConfigValid( pPicBufMetaData );
    if (RET_SUCCESS != result)
    {
        ALOGE("IBD-API:%s PicBufIsConfigValid() failed (RESULT=%d)\n", __func__, result);
        return NULL;
    }

    // create context
    pibdContext = ibdCreateContext( halHandle, pPicBufMetaData );
    if (pibdContext == NULL)
    {
        ALOGE("IBD-API:%s ibdCreateContext() failed\n", __func__);
        return NULL;
    }

    ALOGI("IBD-API:%s (exit)\n", __func__ );

    return (ibdHandle_t)pibdContext;
}


/******************************************************************************
 * ibdOpenDirect()
 *****************************************************************************/
ibdHandle_t ibdOpenDirect
(
    PicBufMetaData_t    *pPicBufMetaData
)
{
    RESULT result = RET_FAILURE;
    ibdContext_t *pibdContext = NULL;

    ALOGI("IBD-API:%s (enter)\n", __func__ );

    if (pPicBufMetaData == NULL)
    {
       ALOGE("IBD-API:%s RET_NULL_POINTER\n", __func__ );
        return NULL;
    }

    // pre-check buffer meta data
    result = PicBufIsConfigValid( pPicBufMetaData );
    if (RET_SUCCESS != result)
    {
        ALOGE("IBD-API:%s PicBufIsConfigValid() failed (RESULT=%d)\n", __func__, result);
        return NULL;
    }

    // create context
    pibdContext = ibdCreateContext( NULL, pPicBufMetaData );
    if (pibdContext == NULL)
    {
        ALOGE("IBD-API:%s ibdCreateContext() failed\n", __func__);
        return NULL;
    }

    ALOGI("IBD-API:%s (exit)\n", __func__ );

    return (ibdHandle_t)pibdContext;
}


/******************************************************************************
 * ibdClose()
 *****************************************************************************/
RESULT ibdClose
(
    ibdHandle_t     ibdHandle
)
{
    RESULT result = RET_FAILURE;

    ALOGI("IBD-API:%s (enter)\n", __func__ );

    if (ibdHandle == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    // get context from handle
    ibdContext_t *pibdContext = (ibdContext_t*)ibdHandle;

    // destroy context
    result = ibdDestroyContext( pibdContext );
    if (RET_SUCCESS != result)
    {
        ALOGE("IBD-API:%s ibdDestroyContext() failed (RESULT=%d)\n", __func__, result);
    }

    ALOGI("IBD-API:%s (exit)\n", __func__ );

    return ( result );
}


/******************************************************************************
 * ibdDraw()
 *****************************************************************************/
RESULT ibdDraw
(
    ibdHandle_t     ibdHandle,
    uint32_t        numCmds,
    ibdCmd_t        *pIbdCmds,
    bool_t          scaledCoords
)
{
    RESULT result = RET_FAILURE;

    ALOGI("IBD-API:%s (enter)\n", __func__ );

    if ( (ibdHandle == NULL) || (pIbdCmds == NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    if ( numCmds == 0 )
    {
        return ( RET_INVALID_PARM );
    }

    // get context from handle
    ibdContext_t *pibdContext = (ibdContext_t*)ibdHandle;

    result = ibdDrawCmds( pibdContext, numCmds, pIbdCmds, scaledCoords );
    if (RET_SUCCESS != result)
    {
        ALOGE("IBD-API:%s ibdDrawCmds() failed (RESULT=%d)\n", __func__, result);
    }

    ALOGI("IBD-API:%s (exit)\n", __func__ );

    return ( result );
}
