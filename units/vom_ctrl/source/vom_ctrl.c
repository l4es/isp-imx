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
 * @vom_ctrl.c
 *
 * @brief
 *   Implementation of vom ctrl.
 *
 *****************************************************************************/
/**
 * @page vom_ctrl_page VOM Ctrl
 * The Video Output Module displays image buffers handed in via QuadMVDU_FX on
 * a connected HDMI display device.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref vom_ctrl_api
 * - @ref vom_ctrl_common
 * - @ref vom_ctrl
 * - @ref vom_ctrl_mvdu
 *
 */

#include <ebase/trace.h>

#include <common/return_codes.h>
#include <common/picture_buffer.h>

#include <oslayer/oslayer.h>

#include <bufferpool/media_buffer_queue_ex.h>

#include "vom_ctrl.h"

//sunny add for test video 2 direct fb

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
//#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <linux/fb.h>

static volatile unsigned int *fbbuf = NULL;
static int fd=0;
static struct fb_var_screeninfo vinfo;
static unsigned long screen_size=0;
static int bpp;

extern uint8_t *extern_mem_virtual_base;

//

/******************************************************************************
 * local macro definitions
 *****************************************************************************/

CREATE_TRACER(VOM_CTRL_INFO , "VOM-CTRL: ", INFO, 0);
CREATE_TRACER(VOM_CTRL_ERROR, "VOM-CTRL: ", ERROR, 1);


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
 * vomCtrlGetState()
 *****************************************************************************/
static inline vomCtrlState_t vomCtrlGetState
(
    vomCtrlContext_t    *pVomContext
);

/******************************************************************************
 * vomCtrlSetState()
 *****************************************************************************/
static inline void vomCtrlSetState
(
    vomCtrlContext_t    *pVomContext,
    vomCtrlState_t      newState
);

/******************************************************************************
 * vomCtrlMediaBufQueueExNotifyCbInput()
 *****************************************************************************/
static void vomCtrlMediaBufQueueExNotifyCbInput
(
    MediaBufQueueExEvent_t  event,
    void                    *pUserContext,
    const MediaBuffer_t     *pMediaBuffer
);

/******************************************************************************
 * vomCtrlCompleteCommand()
 *****************************************************************************/
static void vomCtrlCompleteCommand
(
    vomCtrlContext_t    *pVomContext,
    vomCtrlCmdId_t      Command,
    RESULT              result
);

#if 0
/******************************************************************************
 * vomCtrlBufferReleaseCb()
 *****************************************************************************/
static void vomCtrlBufferReleaseCb
(
    void            *pUserContext,  //!< Opaque user data pointer that was passed in on creation.
    MediaBuffer_t   *pBuffer,       //!< Pointer to buffer that is to be released.
    RESULT          DelayedResult   //!< Result of delayed buffer processing.
);
#endif

/******************************************************************************
 * vomCtrlThreadHandler()
 *****************************************************************************/
static int32_t vomCtrlThreadHandler
(
    void *p_arg
);

void display_to_directfb(HalHandle_t        HalHandle, uint32_t p_buffer)
{
	HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
	if (!pHalCtx || pHalCtx->refCount == 0)
	{
		return ;
	}

	if(fd == 0)
	{
		fd = open("/dev/fb0", O_RDWR);
		if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
			return ;
		bpp = vinfo.bits_per_pixel;
		screen_size = vinfo.xres * vinfo.yres * bpp / 8;
		printf("------------------------------------------------------------------------------srceen info:%dX%d %d bit\n",vinfo.xres,vinfo.yres,bpp);
		fbbuf = mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	}
	//else
	{
		if (fbbuf)
		//	munmap((void*) fbbuf, screen_size);
		//memcpy((char *)fbbuf,p_buffer,screen_size);
		memcpy((char *)fbbuf, pHalCtx->extern_mem_virtual_base + (p_buffer-0x25800000),1920*1080);
	}

}


/******************************************************************************
 * API functions; see header file for detailed comment.
 *****************************************************************************/

/******************************************************************************
 * vomCtrlCreate()
 *****************************************************************************/
RESULT vomCtrlCreate
(
    vomCtrlContext_t    *pVomContext
)
{
    TRACE(VOM_CTRL_INFO, "%s (enter)\n", __func__);

    DCT_ASSERT( pVomContext != NULL );
#if 0
    // connect to MVDU
    RESULT result = vomCtrlMvduInit(&pVomContext->MvduHandle, pVomContext->pVideoFormat, pVomContext->Enable3D, pVomContext->VideoFormat3D, vomCtrlBufferReleaseCb, pVomContext, pVomContext->HalHandle);
    if (RET_SUCCESS != result)
    {
        TRACE(VOM_CTRL_ERROR, "%s (initializing MVDU failed)\n", __func__);
        return result;
    }
#endif
	RESULT result;

    // add HAL reference
    result = HalAddRef( pVomContext->HalHandle );
    if (result != RET_SUCCESS)
    {
        TRACE( VOM_CTRL_ERROR, "%s (adding HAL reference failed)\n", __func__ );
        HalDelRef( pVomContext->HalHandle );
        return result;
    }

    // create command queue
    if ( OSLAYER_OK != osQueueInit( &pVomContext->CommandQueue, pVomContext->MaxCommands, sizeof(vomCtrlCmdId_t) ) )
    {
        TRACE(VOM_CTRL_ERROR, "%s (creating command queue (depth: %d) failed)\n", __func__, pVomContext->MaxCommands);
        vomCtrlMvduDestroy(pVomContext->MvduHandle);
        HalDelRef( pVomContext->HalHandle );
        return ( RET_FAILURE );
    }

    // create full buffer queue
    if ( OSLAYER_OK != osQueueInit( &pVomContext->FullBufQueue, pVomContext->MaxBuffers, sizeof(MediaBuffer_t *) ) )
    {
        TRACE(VOM_CTRL_ERROR, "%s (creating buffer queue (depth: %d) failed)\n", __func__, pVomContext->MaxBuffers);
        osQueueDestroy( &pVomContext->CommandQueue );
        vomCtrlMvduDestroy(pVomContext->MvduHandle);
        HalDelRef( pVomContext->HalHandle );
        return RET_FAILURE;
    }

    // 'connect' to input queue
    pVomContext->InputQueueHighWM = BOOL_FALSE;
    if ( NULL != pVomContext->pInputQueue )
    {
		if ( OSLAYER_OK != MediaBufQueueExRegisterCb( pVomContext->pInputQueue, vomCtrlMediaBufQueueExNotifyCbInput, pVomContext ) )
		{
			TRACE( VOM_CTRL_ERROR, "%s (connecting to media buffer queue ex failed)\n", __func__ );
			osQueueDestroy( &pVomContext->FullBufQueue );
			osQueueDestroy( &pVomContext->CommandQueue );
			vomCtrlMvduDestroy(pVomContext->MvduHandle);
			HalDelRef( pVomContext->HalHandle );
			return ( RET_FAILURE );
		}
    }

    // create handler thread
    if ( OSLAYER_OK != osThreadCreate( &pVomContext->Thread, vomCtrlThreadHandler, pVomContext ) )
    {
        TRACE(VOM_CTRL_ERROR, "%s (creating handler thread failed)\n", __func__);
        if ( NULL != pVomContext->pInputQueue )
        {
        	MediaBufQueueExDeregisterCb( pVomContext->pInputQueue, vomCtrlMediaBufQueueExNotifyCbInput );
        }
        osQueueDestroy( &pVomContext->FullBufQueue );
        osQueueDestroy( &pVomContext->CommandQueue );
        vomCtrlMvduDestroy(pVomContext->MvduHandle);
        HalDelRef( pVomContext->HalHandle );
        return ( RET_FAILURE );
    }

    TRACE(VOM_CTRL_INFO, "%s (exit)\n", __func__ );

    return ( RET_SUCCESS );
}


/******************************************************************************
 * vomCtrlDestroy()
 *****************************************************************************/
RESULT vomCtrlDestroy
(
    vomCtrlContext_t *pVomContext
)
{
    RESULT result = RET_SUCCESS;
    RESULT lres;
    OSLAYER_STATUS osStatus;

    TRACE(VOM_CTRL_INFO, "%s (enter)\n", __func__);

    DCT_ASSERT( pVomContext != NULL );

    // send handler thread a shutdown command
    lres = vomCtrlSendCommand( pVomContext, VOM_CTRL_CMD_SHUTDOWN );
    if (lres != RET_SUCCESS)
    {
        TRACE(VOM_CTRL_ERROR, "%s (send command failed -> RESULT=%d)\n", __func__, result);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // wait for handler thread to have stopped due to the shutdown command given above
    if ( OSLAYER_OK != osThreadWait( &pVomContext->Thread ) )
    {
        TRACE(VOM_CTRL_ERROR, "%s (waiting for handler thread failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // destroy handler thread
    if ( OSLAYER_OK != osThreadClose( &pVomContext->Thread ) )
    {
        TRACE(VOM_CTRL_ERROR, "%s (closing handler thread failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // disconnect from input queue
    if ( NULL != pVomContext->pInputQueue )
    {
		if ( OSLAYER_OK != MediaBufQueueExDeregisterCb( pVomContext->pInputQueue, vomCtrlMediaBufQueueExNotifyCbInput ) )
		{
			TRACE( VOM_CTRL_ERROR, "%s (disconnecting from media buffer queue ex failed)\n", __func__ );
			UPDATE_RESULT( result, RET_FAILURE);
		}
    }

    // cancel any further commands waiting in command queue
    do
    {
        // get next command from queue
        vomCtrlCmdId_t Command;
        osStatus = osQueueTryRead( &pVomContext->CommandQueue, &Command );

        switch (osStatus)
        {
            case OSLAYER_OK:        // got a command, so cancel it
                vomCtrlCompleteCommand( pVomContext, Command, RET_CANCELED );
                break;
            case OSLAYER_TIMEOUT:   // queue is empty
                break;
            default:                // something is broken...
                UPDATE_RESULT( result, RET_FAILURE);
                break;
        }
    } while (osStatus == OSLAYER_OK);

    // destroy full buffer queue
    if ( OSLAYER_OK != osQueueDestroy( &pVomContext->FullBufQueue ) )
    {
        TRACE(VOM_CTRL_ERROR, "%s (destroying full buffer queue failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // destroy command queue
    if ( OSLAYER_OK != osQueueDestroy( &pVomContext->CommandQueue ) )
    {
        TRACE(VOM_CTRL_ERROR, "%s (destroying command queue failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // disconnect from MVDU
    if ( RET_SUCCESS != vomCtrlMvduDestroy(pVomContext->MvduHandle) )
    {
        TRACE(VOM_CTRL_ERROR, "%s (shutting down MVDU failed)\n", __func__);
        UPDATE_RESULT( result, RET_FAILURE);
    }

    // remove HAL reference
    lres = HalDelRef( pVomContext->HalHandle );
    if (lres != RET_SUCCESS)
    {
        TRACE( VOM_CTRL_ERROR, "%s (removing HAL reference failed)\n", __func__ );
        UPDATE_RESULT( result, RET_FAILURE);
    }

    TRACE(VOM_CTRL_INFO, "%s (exit)\n", __func__ );

    return result;
}


/******************************************************************************
 * vomCtrlSendCommand()
 *****************************************************************************/
RESULT vomCtrlSendCommand
(
    vomCtrlContext_t    *pVomContext,
    vomCtrlCmdId_t      Command
)
{
    TRACE(VOM_CTRL_INFO, "%s (enter)\n", __func__);

    if( pVomContext == NULL )
    {
        return RET_NULL_POINTER;
    }

    // are we shutting down?
    if ( vomCtrlGetState( pVomContext ) == eVomCtrlStateInvalid )
    {
        return RET_CANCELED;
    }

    // send command
    OSLAYER_STATUS osStatus = osQueueWrite( &pVomContext->CommandQueue, &Command);
    if (osStatus != OSLAYER_OK)
    {
        TRACE(VOM_CTRL_ERROR, "%s (sending command to queue failed -> OSLAYER_STATUS=%d)\n", __func__, vomCtrlGetState( pVomContext ), osStatus);
    }

    TRACE(VOM_CTRL_INFO, "%s (exit)\n", __func__);

    return ( (osStatus == OSLAYER_OK) ? RET_SUCCESS : RET_FAILURE);
}


/******************************************************************************
 * Local functions
 *****************************************************************************/

/******************************************************************************
 * vomCtrlGetState()
 *****************************************************************************/
static inline vomCtrlState_t vomCtrlGetState
(
    vomCtrlContext_t    *pVomContext
)
{
    DCT_ASSERT( pVomContext != NULL );
    return ( pVomContext->State );
}


/******************************************************************************
 * vomCtrlSetState()
 *****************************************************************************/
static inline void vomCtrlSetState
(
    vomCtrlContext_t    *pVomContext,
    vomCtrlState_t      newState
)
{
    DCT_ASSERT( pVomContext != NULL );
    pVomContext->State = newState;
}


/******************************************************************************
 * vomCtrlMediaBufQueueExNotifyCbInput()
 *****************************************************************************/
static void vomCtrlMediaBufQueueExNotifyCbInput
(
    MediaBufQueueExEvent_t event,
    void* pUserContext,
    const MediaBuffer_t *pMediaBuffer
)
{
    TRACE(VOM_CTRL_INFO, "%s (enter)\n", __func__);

    DCT_ASSERT( pUserContext != NULL );

    vomCtrlContext_t *pVomContext = (vomCtrlContext_t *)pUserContext;

    switch(event)
    {
        case FULL_BUFFER_ADDED:
        {
            RESULT result = vomCtrlSendCommand( pVomContext, VOM_CTRL_CMD_PROCESS_BUFFER );
            if (result != RET_SUCCESS)
            {
                 TRACE(VOM_CTRL_ERROR, "%s (send command failed -> RESULT=%d)\n", __func__, result);
            }
            break;
        }

        case HIGH_WATERMARK_CRITICAL_REGION_ENTERED:
        {
            pVomContext->InputQueueHighWM = BOOL_TRUE;
            break;
        }

        case HIGH_WATERMARK_CRITICAL_REGION_LEFT:
        {
            pVomContext->InputQueueHighWM = BOOL_FALSE;
            break;
        }
    }

    TRACE(VOM_CTRL_INFO, "%s (exit)\n", __func__);
}


/******************************************************************************
 * vomCtrlCompleteCommand()
 *****************************************************************************/
static void vomCtrlCompleteCommand
(
    vomCtrlContext_t    *pVomContext,
    vomCtrlCmdId_t      Command,
    RESULT              result
)
{
    TRACE(VOM_CTRL_INFO, "%s (enter)\n", __func__);

    DCT_ASSERT( pVomContext != NULL );
    pVomContext->vomCbCompletion( Command, result, pVomContext->pUserContext );

    TRACE(VOM_CTRL_INFO, "%s (exit)\n", __func__);
}

#if 0

/******************************************************************************
 * vomCtrlBufferReleaseCb()
 *****************************************************************************/
static void vomCtrlBufferReleaseCb
(
    void            *pUserContext,
    MediaBuffer_t   *pBuffer,
    RESULT          DelayedResult
)
{
    RESULT result;

    DCT_ASSERT(pUserContext != NULL);
    DCT_ASSERT(pBuffer != NULL);

    // get context
    vomCtrlContext_t *pVomContext = (vomCtrlContext_t *)(pUserContext);

    if ( NULL == pVomContext->pInputQueue )
    {
        if ( pBuffer->pNext != NULL )
        {
            MediaBufUnlockBuffer( pBuffer->pNext );
        }
        MediaBufUnlockBuffer( pBuffer );
    }
    else
    {
		// release buffer
		result = MediaBufQueueExReleaseBuffer( pVomContext->pInputQueue, pBuffer );
		DCT_ASSERT(result == RET_SUCCESS);
    }

    // complete command
    vomCtrlCompleteCommand( pVomContext, VOM_CTRL_CMD_PROCESS_BUFFER, DelayedResult );
}

#endif
/******************************************************************************
 * vomCtrlThreadHandler()
 *****************************************************************************/
static int32_t vomCtrlThreadHandler
(
    void *p_arg
)
{
    TRACE(VOM_CTRL_INFO, "%s (enter)\n", __func__);

    if ( p_arg == NULL )
    {
        TRACE(VOM_CTRL_ERROR, "%s (arg pointer is NULL)\n", __func__);
    }
    else
    {
        vomCtrlContext_t *pVomContext = (vomCtrlContext_t *)p_arg;

        bool_t bExit = BOOL_FALSE;

        // processing loop
        do
        {
            // set default result
            RESULT result = RET_WRONG_STATE;

            // wait for next command
            vomCtrlCmdId_t Command;
            OSLAYER_STATUS osStatus = osQueueRead(&pVomContext->CommandQueue, &Command);
            if (OSLAYER_OK != osStatus)
            {
                TRACE(VOM_CTRL_ERROR, "%s (receiving command failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus);
                continue; // for now we simply try again
            }

            // process command
            switch ( Command )
            {
                case VOM_CTRL_CMD_START:
                {
                    TRACE(VOM_CTRL_INFO, "%s (begin VOM_CTRL_CMD_START)\n", __func__);

                    switch ( vomCtrlGetState( pVomContext ) )
                    {
                        case eVomCtrlStateIdle:
                        {
                            vomCtrlSetState( pVomContext, eVomCtrlStateRunning );
                            result = RET_SUCCESS;
                            break;
                        }
                        default:
                        {
                            TRACE(VOM_CTRL_ERROR, "%s (wrong state %d)\n", __func__, vomCtrlGetState( pVomContext ));
                        }
                    }

                    TRACE(VOM_CTRL_INFO, "%s (end VOM_CTRL_CMD_START)\n", __func__);

                    break;
                }

                case VOM_CTRL_CMD_STOP:
                {
                    TRACE(VOM_CTRL_INFO, "%s (begin VOM_CTRL_CMD_STOP)\n", __func__);

                    switch ( vomCtrlGetState( pVomContext ) )
                    {
                        case eVomCtrlStateRunning:
                        {
                            vomCtrlSetState( pVomContext, eVomCtrlStateIdle );
                            result = RET_SUCCESS;
                            break;
                        }
                        default:
                        {
                            TRACE(VOM_CTRL_ERROR, "%s (wrong state %d)\n", __func__, vomCtrlGetState( pVomContext ));
                        }
                    }

                    TRACE(VOM_CTRL_INFO, "%s (end VOM_CTRL_CMD_STOP)\n", __func__);

                    break;
                }

                case VOM_CTRL_CMD_SHUTDOWN:
                {
                    TRACE(VOM_CTRL_INFO, "%s (begin VOM_CTRL_CMD_SHUTDOWN)\n", __func__);

                    switch ( vomCtrlGetState( pVomContext ) )
                    {
                        case eVomCtrlStateIdle:
                        {
                            vomCtrlSetState( pVomContext, eVomCtrlStateInvalid ); // stop further commands from being send to command queue
                            bExit = BOOL_TRUE;
                            result = RET_PENDING; // avoid completion below
                            break;
                        }
                        default:
                        {
                            TRACE(VOM_CTRL_ERROR, "%s (wrong state %d)\n", __func__, vomCtrlGetState( pVomContext ));
                        }
                    }

                    TRACE(VOM_CTRL_INFO, "%s (end VOM_CTRL_CMD_SHUTDOWN)\n", __func__);

                    break;
                }

                case VOM_CTRL_CMD_PROCESS_BUFFER:
                {
                    TRACE(VOM_CTRL_INFO, "%s (begin VOM_CTRL_CMD_PROCESS_BUFFER)\n", __func__);

                    MediaBuffer_t *pBuffer = NULL;

                    switch ( vomCtrlGetState( pVomContext ) )
                    {
                        case eVomCtrlStateIdle:
                        {
                            if ( NULL == pVomContext->pInputQueue )
                            {
								osStatus = osQueueTryRead( &pVomContext->FullBufQueue, &pBuffer );
								if ( ( osStatus != OSLAYER_OK ) || ( pBuffer == NULL ) )
								{
									TRACE(VOM_CTRL_ERROR, "%s (receiving full buffer failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus);
									break;
								}

	                            // just discard the buffer
                                if ( pBuffer->pNext != NULL )
                                {
                                    MediaBufUnlockBuffer( pBuffer->pNext );
                                }
	                            MediaBufUnlockBuffer( pBuffer );
                            }
                            else
                            {
								// just discard buffer
                            	pBuffer = MediaBufQueueExGetFullBuffer( pVomContext->pInputQueue );
								if ( pBuffer )
								{
									result = MediaBufQueueExReleaseBuffer( pVomContext->pInputQueue, pBuffer );
									DCT_ASSERT( result == RET_SUCCESS );
								}
                            }

                            result = RET_SUCCESS;
                            break;
                        }

                        case eVomCtrlStateRunning:
                        {
                            if ( NULL == pVomContext->pInputQueue )
                            {
								osStatus = osQueueTryRead( &pVomContext->FullBufQueue, &pBuffer );
								if ( ( osStatus != OSLAYER_OK ) || ( pBuffer == NULL ) )
								{
									TRACE(VOM_CTRL_ERROR, "%s (receiving full buffer failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus);
									break;
								}

								// very simple & crude way to avoid system stall due to input queue overflow:
								// just skip displaying the buffer if high watermark is exceeded so that we
								// quickly drain the queue; expected to need improvement, though
								if (!pVomContext->InputQueueHighWM)
								{
									// send buffer to MVDU_FX
									result = vomCtrlMvduDisplay( pVomContext->MvduHandle, pBuffer );
									TRACE(VOM_CTRL_INFO, "---------------------------%s (before vomCtrlMvduDisplay  1)\n", __func__);
								}

								// immediately release buffer if it's not pending
								if (result != RET_PENDING)
								{
                                    if ( pBuffer->pNext != NULL )
                                    {
                                        MediaBufUnlockBuffer( pBuffer->pNext );
                                    }
		                            MediaBufUnlockBuffer( pBuffer );
								}
                            }
                            else
                            {
                            	pBuffer = MediaBufQueueExGetFullBuffer( pVomContext->pInputQueue );
								if ( pBuffer )
								{
									// very simple & crude way to avoid system stall due to input queue overflow:
									// just skip displaying the buffer if high watermark is exceeded so that we
									// quickly drain the queue; expected to need improvement, though
#if 0
sunny
									if (!pVomContext->InputQueueHighWM)
									{
										// send buffer to MVDU_FX
										result = vomCtrlMvduDisplay( pVomContext->MvduHandle, pBuffer );
									}
#endif
									TRACE(VOM_CTRL_INFO, "---------------------------%s (before vomCtrlMvduDisplay  2)\n", __func__);
									//PicBufMetaData_t *pPicBufMetaData = (PicBufMetaData_t *)(pBuffer->pMetaData);
									//display_to_directfb(pPicBufMetaData);

///sunny add


									//PicBufMetaData_t            *pPicBufMetaData;
									PicBufMetaData_t *pPicBufMetaData = (PicBufMetaData_t *)(pBuffer->pMetaData);
									display_to_directfb(pVomContext->HalHandle, (uint32_t)(pPicBufMetaData->Data.YCbCr.semiplanar.Y.BaseAddress));


///sunny add

									// immediately release buffer if it's not pending
									if (result != RET_PENDING)
									{
										RESULT lresult = MediaBufQueueExReleaseBuffer( pVomContext->pInputQueue, pBuffer );
										DCT_ASSERT(lresult == RET_SUCCESS);
									}


								}
								else
								{
									result = RET_FAILURE;
								}
                            }

                            break;
                        }

                        default:
                        {
                            TRACE(VOM_CTRL_ERROR, "%s (wrong state %d)\n", __func__, vomCtrlGetState( pVomContext ));
                        }
                    }

                    TRACE(VOM_CTRL_INFO, "%s (end VOM_CTRL_CMD_PROCESS_BUFFER)\n", __func__);

                    break;
                }

                default:
                {
                    TRACE(VOM_CTRL_ERROR, "%s (illegal command %d)\n", __func__, Command);
                    result = RET_NOTSUPP;
                }
            }

            // complete command?
            if (result != RET_PENDING)
            {
                vomCtrlCompleteCommand( pVomContext, Command, result );
            }
        }
        while ( bExit == BOOL_FALSE );  /* !bExit */
    }

    TRACE(VOM_CTRL_INFO, "%s (exit)\n", __func__);

    return ( 0 );
}
