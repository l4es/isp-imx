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
 * @dom_ctrl.c
 *
 * @brief
 *   Implementation of dom ctrl.
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
 */

#ifdef DOM_FPS
#include <sys/time.h>
#endif // DOM_FPS

#include "vlog.h"
#include "dom_ctrl.h"
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <assert.h>
#include <memory.h> 

#ifdef SUBDEV_V4L2
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#endif


#define LOGTAG "APPDOM"

static inline domCtrlState_t domCtrlGetState
(
    domCtrlContext_t    *pdomContext
);

/******************************************************************************
 * domCtrlSetState()
 *****************************************************************************/
static inline void domCtrlSetState
(
    domCtrlContext_t    *pdomContext,
    domCtrlState_t      newState
);


/******************************************************************************
 * domCtrlCompleteCommand()
 *****************************************************************************/
static void domCtrlCompleteCommand
(
    domCtrlContext_t        *pdomContext,
    domCtrlCmdId_t          CmdID,
    RESULT                  result
);

/******************************************************************************
 * domCtrlBufferReleaseCb()
 *****************************************************************************/
static void domCtrlBufferReleaseCb
(
    void            *pUserContext,  //!< Opaque user data pointer that was passed in on creation.
    BufIdentity   *pBuffer,       //!< Pointer to buffer that is to be released.
    RESULT          DelayedResult   //!< Result of delayed buffer processing.
);

/******************************************************************************
 * domCtrlThreadHandler()
 *****************************************************************************/
static int32_t domCtrlThreadHandler
(
    void *p_arg
);

/******************************************************************************
 * domCtrlDisplayBuffer()
 *****************************************************************************/
static RESULT domCtrlDisplayBuffer
(
    domCtrlContext_t    *pdomContext,
    BufIdentity       *pBuffer
);

/******************************************************************************
 * domCtrlDisplayBufferYUV422Semi()
 *****************************************************************************/
static RESULT domCtrlDisplayBufferYUV422Semi
(
    domCtrlContext_t    *pdomContext,
    BufIdentity       *pBuffer
);

/******************************************************************************
 * domCtrlOpenMemFd()
 *****************************************************************************/
static RESULT domCtrlOpenMemFd
(
    domCtrlContext_t    *pdomContext
);

RESULT domCtrlCreate
(
    domCtrlContext_t    *pdomContext
)
{
    RESULT result;

    LOGI("%s (enter)\n", __func__);

    assert( pdomContext != NULL );

    // finalize init of context
    pdomContext->pCurDisplayBuffer = NULL;
#ifdef DOM_FPS
    pdomContext->FpsStartTimeMs = 0;
    pdomContext->FpsDisplayed   = 0;
    pdomContext->FpsSkipped     = 0;
#endif // DOM_FPS
    pdomContext->domId          = pdomContext->domId;

    result = domCtrlOpenMemFd(pdomContext);
    if (result != RET_SUCCESS)
    {
        LOGE("%s (open mem fd failed)\n", __func__ );
        return result;
    }

    // create graphic subsystem dependent video player
    domCtrlVidplayConfig_t VidplayConfig;
    memset( &VidplayConfig, 0, sizeof(VidplayConfig) );
    VidplayConfig.hParent = pdomContext->Config.hParent;
    VidplayConfig.posX    = pdomContext->Config.posX;
    VidplayConfig.posY    = pdomContext->Config.posY;
    VidplayConfig.width   = pdomContext->Config.width;
    VidplayConfig.height  = pdomContext->Config.height;
    result = domCtrlVidplayInit( &VidplayConfig );
    if (result != RET_SUCCESS)
    {
        LOGE("%s (creating video player failed)\n", __func__ );
        return result;
    }
    pdomContext->hDomCtrlVidplay = VidplayConfig.domCtrlVidplayHandle;

    // create command queue
    if ( OSLAYER_OK != osQueueInit( &pdomContext->CommandQueue, pdomContext->Config.MaxPendingCommands, sizeof(domCtrlCmdId_t) ) )
    {
        LOGE("%s (creating command queue (depth: %d) failed)\n", __func__, pdomContext->Config.MaxPendingCommands);
        domCtrlVidplayShutDown( pdomContext->hDomCtrlVidplay );
        return RET_FAILURE;
    }

    // create full buffer queue
    if ( OSLAYER_OK != osQueueInit( &pdomContext->FullBufQueue, pdomContext->Config.MaxBuffers, sizeof(BufIdentity *) ) )
    {
        LOGE("%s (creating buffer queue (depth: %d) failed)\n", __func__, pdomContext->Config.MaxBuffers);
        osQueueDestroy( &pdomContext->CommandQueue );
        domCtrlVidplayShutDown( pdomContext->hDomCtrlVidplay );
        return RET_FAILURE;
    }

    // initialize draw mutex
    if ( OSLAYER_OK != osMutexInit( &pdomContext->drawMutex ) )
    {
        LOGE("%s (connecting to media buffer queue ex failed)\n", __func__);
        osQueueDestroy( &pdomContext->FullBufQueue );
        osQueueDestroy( &pdomContext->CommandQueue );
        domCtrlVidplayShutDown( pdomContext->hDomCtrlVidplay );
        return RET_FAILURE;
    }

    // allocate memory for draw context list
    pdomContext->pDrawContextList = malloc( sizeof(List) );
    if ( pdomContext->pDrawContextList == NULL )
    {
        LOGE("%s (allocating memory for draw context list failed)\n", __func__ );
        osMutexDestroy( &pdomContext->drawMutex );
        osQueueDestroy( &pdomContext->FullBufQueue );
        osQueueDestroy( &pdomContext->CommandQueue );
        domCtrlVidplayShutDown( pdomContext->hDomCtrlVidplay );
        return ( RET_OUTOFMEM );
    }
    //MEMSET( pdomContext->pDrawContextList, 0, sizeof( List ) );
    ListInit( pdomContext->pDrawContextList );

    // 'connect' to input queue
    pdomContext->InputQueueHighWM = BOOL_FALSE;

    // create handler thread
    if ( OSLAYER_OK != osThreadCreate( &pdomContext->Thread, domCtrlThreadHandler, pdomContext ) )
    {
        LOGE("%s (creating handler thread failed)\n", __func__);
        free( pdomContext->pDrawContextList );
        osMutexDestroy( &pdomContext->drawMutex );
        osQueueDestroy( &pdomContext->FullBufQueue );
        osQueueDestroy( &pdomContext->CommandQueue );
        domCtrlVidplayShutDown( pdomContext->hDomCtrlVidplay );
        return RET_FAILURE;
    }

    LOGI("%s (exit)\n", __func__ );

    return RET_SUCCESS;
}

RESULT domCtrlDestroy
(
    domCtrlContext_t *pdomContext
)
{
    RESULT result = RET_SUCCESS;
    RESULT lres;
    OSLAYER_STATUS osStatus;

    LOGI("%s (enter)\n", __func__);

    assert( pdomContext != NULL );

    // send handler thread a shutdown command
    lres = domCtrlSendCommand( pdomContext, DOM_CTRL_CMD_SHUTDOWN );
    if (lres != RET_SUCCESS)
    {
        LOGE("%s (send command failed -> RESULT=%d)\n", __func__, result);
        UPDATE_RESULT( result, lres);
    }

    // wait for handler thread to have stopped due to the shutdown command given above
    if ( OSLAYER_OK != osThreadWait( &pdomContext->Thread ) )
    {
        LOGE("%s (waiting for handler thread failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // destroy handler thread
    if ( OSLAYER_OK != osThreadClose( &pdomContext->Thread ) )
    {
        LOGE("%s (closing handler thread failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // cancel any further commands waiting in command queue
    do
    {
        // get next command from queue
        domCtrlCmdId_t CmdID;
        osStatus = osQueueTryRead( &pdomContext->CommandQueue, &CmdID );

        switch (osStatus)
        {
            case OSLAYER_OK:        // got a command, so cancel it
                domCtrlCompleteCommand( pdomContext, CmdID, RET_CANCELED );
                break;
            case OSLAYER_TIMEOUT:   // queue is empty
                break;
            default:                // something is broken...
                UPDATE_RESULT( result, RET_FAILURE);
                break;
        }
    } while (osStatus == OSLAYER_OK);

    // destroy full buffer queue
    if ( OSLAYER_OK != osQueueDestroy( &pdomContext->FullBufQueue ) )
    {
        LOGE("%s (destroying full buffer queue failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // destroy command queue
    if ( OSLAYER_OK != osQueueDestroy( &pdomContext->CommandQueue ) )
    {
        LOGE("%s (destroying command queue failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // destroy graphic subsystem dependent video player
    lres = domCtrlVidplayShutDown( pdomContext->hDomCtrlVidplay );
    if (lres != RET_SUCCESS)
    {
        LOGE("%s (destroying video player failed)\n", __func__ );
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // destroy draw mutex
    if ( OSLAYER_OK != osMutexDestroy( &pdomContext->drawMutex ) )
    {
        LOGE("%s (destroying draw mutex failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // deallocate memory of list and all remaining elements of list
    domCtrlDrawContext_t *pDrawCtx = (domCtrlDrawContext_t *)ListRemoveHead( pdomContext->pDrawContextList );
    while ( pDrawCtx )
    {
        if ( NULL != pDrawCtx->pIbdDrawCmds )
        {
            // Release memory of idb commands
            for ( uint32_t i = 0; i < pDrawCtx->NumDrawCmds; ++i )
            {
                if ( IBD_DRAW_TEXT == pDrawCtx->pIbdDrawCmds[i].cmdId )
                {
                    if ( NULL != pDrawCtx->pIbdDrawCmds[i].params.text.pcText )
                    {
                        free( pDrawCtx->pIbdDrawCmds[i].params.text.pcText );
                    }
                }
            }
            free( pDrawCtx->pIbdDrawCmds );
        }
        free( pDrawCtx );

        pDrawCtx = (domCtrlDrawContext_t *)ListRemoveHead( pdomContext->pDrawContextList );
    }
    free( pdomContext->pDrawContextList );

    // release display buffer?
    if (pdomContext->pCurDisplayBuffer != NULL)
    {
        free( pdomContext->pCurDisplayBuffer );
        pdomContext->pCurDisplayBuffer = NULL;
    }

    close(pdomContext->memFd);

    LOGI("%s (exit)\n", __func__ );

    return result;
}

RESULT domCtrlSendCommand
(
    domCtrlContext_t    *pdomContext,
    domCtrlCmdId_t      CmdID
)
{
    if (pdomContext == NULL)
    {
        return RET_NULL_POINTER;
    }

    // are we shutting down?
    if ( domCtrlGetState( pdomContext ) == edomCtrlStateInvalid )
    {
        return RET_CANCELED;
    }

    // send command
    OSLAYER_STATUS osStatus = osQueueWrite( &pdomContext->CommandQueue, &CmdID);

    return (osStatus == OSLAYER_OK) ? RET_SUCCESS : RET_FAILURE;
}

static inline domCtrlState_t domCtrlGetState
(
    domCtrlContext_t    *pdomContext
)
{
    assert( pdomContext != NULL );
    return pdomContext->State;
}

static inline void domCtrlSetState
(
    domCtrlContext_t    *pdomContext,
    domCtrlState_t      newState
)
{
    assert( pdomContext != NULL );
    pdomContext->State = newState;
}

static void domCtrlCompleteCommand
(
    domCtrlContext_t        *pdomContext,
    domCtrlCmdId_t          CmdID,
    RESULT                  result
)
{
    if ( !pdomContext)
        return;
    // do callback
    if (pdomContext->Config.domCbCompletion)
    	pdomContext->Config.domCbCompletion( CmdID, result, pdomContext->Config.pUserContext );
}

static void domCtrlBufferReleaseCb
(
    void            *pUserContext,
    BufIdentity   *pBuffer,
    RESULT          DelayedResult
)
{
    assert(pUserContext != NULL);
    assert(pBuffer != NULL);

    // get context
    domCtrlContext_t *pdomContext = (domCtrlContext_t *)(pUserContext);

    // complete command
    domCtrlCompleteCommand( pdomContext, DOM_CTRL_CMD_PROCESS_BUFFER, DelayedResult );
}

static int32_t domCtrlThreadHandler
(
    void *p_arg
)
{
    if ( p_arg == NULL )
    {
        LOGE("%s (arg pointer is NULL)\n", __func__);
    }
    else
    {
        domCtrlContext_t *pdomContext = (domCtrlContext_t *)p_arg;

        bool_t bExit = BOOL_FALSE;

        // processing loop
        do
        {
            // set default result
            RESULT result = RET_WRONG_STATE;

            // wait for next command
            domCtrlCmdId_t CmdID;
            OSLAYER_STATUS osStatus = osQueueRead(&pdomContext->CommandQueue, &CmdID);
            if (OSLAYER_OK != osStatus)
            {
                LOGE("%s (receiving command failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus);
                continue; // for now we simply try again
            }

            // process command
            switch ( CmdID )
            {
                case DOM_CTRL_CMD_START:
                {
                    LOGI("%s (begin DOM_CTRL_CMD_START)\n", __func__);

                    switch ( domCtrlGetState( pdomContext ) )
                    {
                        case edomCtrlStateIdle:
                        {
                            // clear display
                            result = domCtrlVidplayClear( pdomContext->hDomCtrlVidplay );
                            assert(result == RET_SUCCESS);

                            // show display
                            result = domCtrlVidplayShow( pdomContext->hDomCtrlVidplay, BOOL_TRUE );
                            assert(result == RET_SUCCESS);

                            // release display buffer?
                            /*if (pdomContext->pCurDisplayBuffer != NULL)
                            {
                                free( pdomContext->pCurDisplayBuffer );
                                pdomContext->pCurDisplayBuffer = NULL;
                            }*/

                            // we're up and running
                            domCtrlSetState( pdomContext, edomCtrlStateRunning );
                            result = RET_SUCCESS;

                            break;
                        }
                        default:
                        {
                            LOGE("%s (wrong state %d)\n", __func__, domCtrlGetState( pdomContext ));
                        }
                    }

                    LOGI("%s (end DOM_CTRL_CMD_START)\n", __func__);

                    break;
                }

                case DOM_CTRL_CMD_STOP:
                {
                    LOGI("%s (begin DOM_CTRL_CMD_STOP)\n", __func__);

                    switch ( domCtrlGetState( pdomContext ) )
                    {
                        case edomCtrlStateRunning:
                        {
                            // hide display
                            result = domCtrlVidplayShow( pdomContext->hDomCtrlVidplay, BOOL_FALSE );
                            assert(result == RET_SUCCESS);

                            // back to idle state
                            domCtrlSetState( pdomContext, edomCtrlStateIdle );

                            result = RET_SUCCESS;
                            break;
                        }
                        default:
                        {
                            LOGE("%s (wrong state %d)\n", __func__, domCtrlGetState( pdomContext ));
                        }
                    }

                    LOGI("%s (end DOM_CTRL_CMD_STOP)\n", __func__);

                    break;
                }

                case DOM_CTRL_CMD_SHUTDOWN:
                {
                    LOGI("%s (begin DOM_CTRL_CMD_SHUTDOWN)\n", __func__);

                    switch ( domCtrlGetState( pdomContext ) )
                    {
                        case edomCtrlStateIdle:
                        {
                            // release display buffer?
                            /*if (pdomContext->pCurDisplayBuffer != NULL)
                            {
                                free( pdomContext->pCurDisplayBuffer );
                                pdomContext->pCurDisplayBuffer = NULL;
                            }*/

                            domCtrlSetState( pdomContext, edomCtrlStateInvalid ); // stop further commands from being send to command queue
                            bExit = BOOL_TRUE;
                            result = RET_PENDING; // avoid completion below
                            break;
                        }
                        default:
                        {
                            LOGE("%s (wrong state %d)\n", __func__, domCtrlGetState( pdomContext ));
                        }
                    }

                    LOGI("%s (end DOM_CTRL_CMD_SHUTDOWN)\n", __func__);

                    break;
                }

                case DOM_CTRL_CMD_PROCESS_BUFFER:
                {

                    BufIdentity *pBuffer = NULL;

                    switch ( domCtrlGetState( pdomContext ) )
                    {
                        case edomCtrlStateIdle:
                        {
                            osStatus = osQueueTryRead( &pdomContext->FullBufQueue, &pBuffer );
                            if ( ( osStatus != OSLAYER_OK ) || ( pBuffer == NULL ) )
                            {
                                LOGE("%s (receiving full buffer failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus);
                                break;
                            }

                            result = RET_SUCCESS;
                            break;
                        }

                        case edomCtrlStateRunning:
                        {
                            osStatus = osQueueTryRead( &pdomContext->FullBufQueue, &pBuffer );
                            if ( ( osStatus != OSLAYER_OK ) || ( pBuffer == NULL ) )
                            {
                                LOGE("%s (receiving full buffer failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus);
                                break;
                            }

                            if ( pBuffer )
                            {
                                // very simple & crude way to avoid system stall due to input queue overflow:
                                // just skip displaying the buffer if high watermark is exceeded so that we
                                // quickly drain the queue; expected to need improvement, though
                                if (!pdomContext->InputQueueHighWM)
                                {
                                #ifdef DOM_FPS
                                    pdomContext->FpsDisplayed++;
                                #endif // DOM_FPS
                                    // display buffer
                                    result = domCtrlDisplayBuffer( pdomContext, pBuffer );
                                }
                           
                            }
                            else
                            {
                                result = RET_FAILURE;
                            }

                            break;
                        }

                        default:
                        {
                            LOGE("%s (wrong state %d)\n", __func__, domCtrlGetState( pdomContext ));
                        }
                    }


                    break;
                }

                default:
                {
                    LOGE("%s (illegal command %d)\n", __func__, CmdID);
                    result = RET_NOTSUPP;
                }
            }

            // complete command?
            if (result != RET_PENDING)
            {
                domCtrlCompleteCommand( pdomContext, CmdID, result );
            }
        }
        while ( bExit == BOOL_FALSE );  /* !bExit */
    }

    LOGI("%s (exit)\n", __func__);

    return ( 0 );
}

void convertYUV2RGBA(int Y, int U, int V, unsigned char* pRGBA) {

    U -= 128;
    V -= 128;
    int R = (int)( Y + 1.403f * V);
	int G = (int)( Y - 0.344f * U - 0.714f * V);
	int B = (int)( Y + 1.770f * U);

    R = MAX(0, MIN(255, R));
    G = MAX(0, MIN(255, G));
    B = MAX(0, MIN(255, B));
    *pRGBA++ = B;
    *pRGBA++ = G;
    *pRGBA++ = R;
    *pRGBA++ = 0xFF;
}

static RESULT domCtrlDisplayBufferYUV422Semi
(
    domCtrlContext_t    *pdomContext,
    BufIdentity       *pBuffer
)
{
    RESULT result = RET_SUCCESS;
    RESULT lres;

    if (!pdomContext || !pBuffer) {
        return RET_NULL_POINTER;
    }

    LOGD("%s in", __func__);

    uint8_t *pYBase, *pCbCrBase;
    pYBase = (unsigned char*)mmap(NULL, 2*pBuffer->width*pBuffer->height, PROT_READ | PROT_WRITE, MAP_SHARED, pdomContext->memFd, pBuffer->address_usr);
    LOGD("phy address:0x%lx, user maped addr: %p", pBuffer->address_usr, pYBase);
    if(pYBase == (uint8_t *)-1)
    {
        LOGE("ERROR Ybase address for mmap");
        return RET_WRONG_HANDLE; 
    }

    pCbCrBase = (uint8_t *)(pYBase + pBuffer->width*pBuffer->height);
    uint8_t *pRGBA = malloc( 4*pBuffer->width*pBuffer->height );
    uint8_t *pRGBATmp = pRGBA;
    uint32_t x,y;
    int posY = 0;
    int posUV = 0;
    for (y = 0; y < pBuffer->height; y++) {
        for ( x= 0; x < pBuffer->width; x+=2) {
            uint8_t Cb = pCbCrBase[posUV++];
            uint8_t Cr = pCbCrBase[posUV++];
            uint8_t Y1 = pYBase[posY++];
            uint8_t Y2 = pYBase[posY++];
            convertYUV2RGBA(Y1, Cb, Cr, pRGBATmp);
            pRGBATmp+=4;
            convertYUV2RGBA(Y2, Cb, Cr, pRGBATmp);
            pRGBATmp+=4;
        }
        // update line starts
    }
    // prepare a set of picture buffer meta data describing this buffer
    PicBufMetaData_t PicBuf;
    PicBuf.Type                             = PIC_BUF_TYPE_RGB32;
    PicBuf.Layout                           = PIC_BUF_LAYOUT_COMBINED;
    PicBuf.Data.RGB.combined.pData          = pRGBA;
    PicBuf.Data.RGB.combined.PicWidthPixel  = pBuffer->width;
    PicBuf.Data.RGB.combined.PicWidthBytes  = 4 * pBuffer->width;
    PicBuf.Data.RGB.combined.PicHeightPixel = pBuffer->height;
    // finally display buffer
    lres = domCtrlVidplayDisplay( pdomContext->hDomCtrlVidplay, &PicBuf );
    UPDATE_RESULT( result, lres );
    if (lres != RET_SUCCESS) {
        LOGE("%s (wrong state %d)\n", __func__, domCtrlGetState( pdomContext ));
    }
    // release previous display buffer?
    if (pdomContext->pCurDisplayBuffer != NULL) {
        free( pdomContext->pCurDisplayBuffer );
    }
    // update current display buffer
    pdomContext->pCurDisplayBuffer = pRGBA;

    munmap(pYBase,2*pBuffer->width*pBuffer->height);
//    close(memFd);
    
    return result;
}

static RESULT domCtrlDisplayBuffer
(
    domCtrlContext_t    *pdomContext,
    BufIdentity       *pBuffer
)
{
    RESULT result = RET_SUCCESS;
    LOGD("%s In", __func__);

    if ( (pdomContext == NULL) || (pBuffer == NULL) ) {
        return RET_NULL_POINTER;
    }

    switch (pBuffer->format) {
    case CAMERA_PIX_FMT_YUV422SP:
        result = domCtrlDisplayBufferYUV422Semi( pdomContext, pBuffer );
        break;
    }

    return result;
}

static RESULT domCtrlOpenMemFd
(
    domCtrlContext_t    *pdomContext
)
{
    RESULT result = RET_SUCCESS;
    LOGD("%s In", __func__);

    if ( pdomContext == NULL) {
        return RET_NULL_POINTER;
    }
    
#ifdef SUBDEV_CHAR
        char szDeviceName[64] = { 0 };
        sprintf(szDeviceName, "/dev/vivisp%d", pdomContext->domId);
        pdomContext->memFd = open(szDeviceName,  O_RDWR | O_NONBLOCK);
        if (pdomContext->memFd < 0)
        {
            LOGE("ERROR open memFd for mmap");
            return RET_WRONG_HANDLE; 
        }
#endif
    
#ifdef SUBDEV_V4L2
    //    int memFd = HalGetFdHandle((HalHandle_t)pdomContext->Config.HalHandle, 2 /*HAL_MODULE_ISP*/);
        pdomContext->memFd = -1;
        {
            char szDeviceName[64] = { 0 };
            char videoID[32];
            sprintf(videoID, "platform:viv%d", pdomContext->domId);
            
            struct v4l2_capability cap;
            for (int index = 0; index < 20; index++) {
                sprintf(szDeviceName, "/dev/video%d", index);
                pdomContext->memFd = open(szDeviceName, O_RDWR | O_NONBLOCK);
                if (pdomContext->memFd < 0) {
                    continue;
                }
                if (ioctl(pdomContext->memFd, VIDIOC_QUERYCAP, &cap) != 0) {
                    continue;
                }
            
                if (!strcmp((char*)cap.driver, "viv_v4l2_device") &&
                     !strcmp((char*)cap.bus_info, videoID)) {
                    LOGD("open dev node %s",szDeviceName);
                    break;
                }
                close(pdomContext->memFd);
                pdomContext->memFd = -1;
            }
            
            if (pdomContext->memFd < 0)
            {
                LOGE("ERROR open memFd for mmap");
                return RET_WRONG_HANDLE; 
            }
            
        }
#endif

    return result;
}

