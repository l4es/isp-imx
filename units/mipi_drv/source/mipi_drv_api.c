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
 * @file cameric_mipi.c
 *
 * @brief   Implementation of MIPI driver API.
 *
 *****************************************************************************/
#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include "mipi_drv_api.h"
#include "mipi_drv.h"


CREATE_TRACER( MIPI_DRV_API_INFO  , "MIPI_DRV_API: ", INFO    , 0 );
CREATE_TRACER( MIPI_DRV_API_WARN  , "MIPI_DRV_API: ", WARNING , 1 );
CREATE_TRACER( MIPI_DRV_API_ERROR , "MIPI_DRV_API: ", ERROR   , 1 );


/******************************************************************************
 * local macro definitions
 *****************************************************************************/


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
 * Implementation of Driver API Functions
 *****************************************************************************/

/******************************************************************************
 * MipiDrvInit()
 *****************************************************************************/
RESULT MipiDrvInit
(
    MipiDrvConfig_t  *pConfig
)
{
    RESULT result = RET_SUCCESS;

    TRACE( MIPI_DRV_API_INFO, "%s: (enter)\n", __func__ );

    // check API params
    if( (pConfig == NULL) || (pConfig->HalHandle == NULL) )
    {
        result = RET_NULL_POINTER;
        goto error_exit;
    }

#ifdef MIPI_USE_CAMERIC
    if(pConfig->CamerIcDrvHandle == NULL)
    {
        result = RET_NULL_POINTER;
        goto error_exit;
    }
#endif

    if (pConfig->InstanceNum > 1)
    {
        result = RET_INVALID_PARM;
        goto error_exit;
    }

    // allocate context
    MipiDrvContext_t *pMipiDrvCtx = malloc( sizeof(MipiDrvContext_t) );
    if (pMipiDrvCtx == NULL)
    {
        result = RET_OUTOFMEM;
        goto error_exit;
    }

    // pre initialize context
    memset( pMipiDrvCtx, 0, sizeof(*pMipiDrvCtx) );
    switch(pConfig->InstanceNum)
    {
        case 0: pMipiDrvCtx->pBase = (void*)HAL_BASEADDR_MARVIN;   break;
        case 1: pMipiDrvCtx->pBase = (void*)HAL_BASEADDR_MARVIN_2; break;
        default:
            result = RET_INVALID_PARM;
            goto cleanup_1;
    }
    pMipiDrvCtx->Config = *pConfig;
    pMipiDrvCtx->MipiConfig.NumLanes = 0;
    pMipiDrvCtx->State = MIPI_DRV_STATE_NOT_CONFIGURED;

    // initialize driver
    result = MipiDrvCreate( pMipiDrvCtx );
    if (RET_SUCCESS != result)
    {
        TRACE( MIPI_DRV_API_ERROR, "%s: MipiDrvCreate() failed (result=%d)\n", __func__, result );
        goto cleanup_1;
    }

    // success, so return handle
    pConfig->MipiDrvHandle = (MipiDrvHandle_t)pMipiDrvCtx;

    TRACE( MIPI_DRV_API_INFO, "%s: (exit)\n", __func__ );

    return result ;

cleanup_1: // free context
    free( pMipiDrvCtx );

error_exit: // just return with error
    TRACE( MIPI_DRV_API_INFO, "%s: (exit, result=%d)\n", __func__, result );

    return result;
}


/******************************************************************************
 * MipiDrvRelease()
 *****************************************************************************/
RESULT MipiDrvRelease
(
    MipiDrvHandle_t MipiDrvHandle
)
{
    RESULT result = RET_SUCCESS;

    TRACE( MIPI_DRV_API_INFO, "%s: (enter)\n", __func__ );

    // check API params
    if( MipiDrvHandle == NULL )
    {
        result = RET_NULL_POINTER;
        goto error_exit;
    }

    // get context
    MipiDrvContext_t *pMipiDrvCtx = (MipiDrvContext_t *)MipiDrvHandle;

    // check state
    if ( (pMipiDrvCtx->State != MIPI_DRV_STATE_STOPPED) && (pMipiDrvCtx->State != MIPI_DRV_STATE_NOT_CONFIGURED) )
    {
        TRACE( MIPI_DRV_API_ERROR, "%s: wrong state\n", __func__ );
        result = RET_WRONG_STATE;
        goto error_exit;
    }

    // destroy driver
    result = MipiDrvDestroy( pMipiDrvCtx );
    if (RET_SUCCESS != result)
    {
        TRACE( MIPI_DRV_API_ERROR, "%s: MipiDrvDestroy() failed (result=%d)\n", __func__, result );
    }

    // free context
    free( pMipiDrvCtx );

    if (RET_SUCCESS != result)
    {
        goto error_exit;
    }

    TRACE( MIPI_DRV_API_INFO, "%s: (exit)\n", __func__ );

    return result;

error_exit: // just return with error
    TRACE( MIPI_DRV_API_INFO, "%s: (exit, result=%d)\n", __func__, result );

    return result;
}


/******************************************************************************
 * MipiDrvConfig()
 *****************************************************************************/
RESULT MipiDrvConfig
(
    MipiDrvHandle_t MipiDrvHandle,
    MipiConfig_t    *pMipiConfig
)
{
    RESULT result = RET_SUCCESS;

    TRACE( MIPI_DRV_API_INFO, "%s: (enter)\n", __func__ );

    // check API params
    if( (MipiDrvHandle == NULL) || (pMipiConfig == NULL) )
    {
        result = RET_NULL_POINTER;
        goto error_exit;
    }

    // get context
    MipiDrvContext_t *pMipiDrvCtx = (MipiDrvContext_t *)MipiDrvHandle;

    // check state
    if ( (pMipiDrvCtx->State != MIPI_DRV_STATE_STOPPED) && (pMipiDrvCtx->State != MIPI_DRV_STATE_NOT_CONFIGURED) )
    {
        TRACE( MIPI_DRV_API_ERROR, "%s: wrong state\n", __func__ );
        result = RET_WRONG_STATE;
        goto error_exit;
    }

    // configure driver
    MipiDrvCmd_t cmd;
    cmd.ID = MIPI_DRV_CMD_CONFIG;
    cmd.params.config.pMipiConfig = pMipiConfig;
    result = MipiDrvCmd( pMipiDrvCtx, &cmd );
    if (RET_SUCCESS != result)
    {
        TRACE( MIPI_DRV_API_ERROR, "%s: MipiDrvCmd('MIPI_DRV_CMD_CONFIG') failed\n", __func__, result );

        // set new state
        pMipiDrvCtx->State = MIPI_DRV_STATE_NOT_CONFIGURED;

        goto error_exit;
    }

    // success, so set new state
    pMipiDrvCtx->State = MIPI_DRV_STATE_STOPPED;

    TRACE( MIPI_DRV_API_INFO, "%s: (exit)\n", __func__ );

    return result;

error_exit: // just return with error
    TRACE( MIPI_DRV_API_INFO, "%s: (exit, result=%d)\n", __func__, result );

    return result;
}


/******************************************************************************
 * MipiDrvStart()
 *****************************************************************************/
RESULT MipiDrvStart
(
    MipiDrvHandle_t MipiDrvHandle
)
{
    RESULT result = RET_SUCCESS;

    TRACE( MIPI_DRV_API_INFO, "%s: (enter)\n", __func__ );

    // check API params
    if( MipiDrvHandle == NULL )
    {
        result = RET_NULL_POINTER;
        goto error_exit;
    }

    // get context
    MipiDrvContext_t *pMipiDrvCtx = (MipiDrvContext_t *)MipiDrvHandle;

    // check state
    if ( pMipiDrvCtx->State != MIPI_DRV_STATE_STOPPED )
    {
        TRACE( MIPI_DRV_API_ERROR, "%s: wrong state\n", __func__ );
        result = RET_WRONG_STATE;
        goto error_exit;
    }

    // start driver
    MipiDrvCmd_t cmd;
    cmd.ID = MIPI_DRV_CMD_START;
    result = MipiDrvCmd( pMipiDrvCtx, &cmd );
    if (RET_SUCCESS != result)
    {
        TRACE( MIPI_DRV_API_ERROR, "%s: MipiDrvCmd('MIPI_DRV_CMD_START') failed (result=%d)\n", __func__, result );
        goto error_exit;
    }

    // success, so set new state
    pMipiDrvCtx->State = MIPI_DRV_STATE_RUNNING;

    TRACE( MIPI_DRV_API_INFO, "%s: (exit)\n", __func__ );

    return result;

error_exit: // just return with error
    TRACE( MIPI_DRV_API_INFO, "%s: (exit, result=%d)\n", __func__, result );

    return result;
}


/******************************************************************************
 * MipiDrvStop()
 *****************************************************************************/
RESULT MipiDrvStop
(
    MipiDrvHandle_t MipiDrvHandle
)
{
    RESULT result = RET_SUCCESS;

    TRACE( MIPI_DRV_API_INFO, "%s: (enter)\n", __func__ );

    // check API params
    if( MipiDrvHandle == NULL )
    {
        result = RET_NULL_POINTER;
        goto error_exit;
    }

    // get context
    MipiDrvContext_t *pMipiDrvCtx = (MipiDrvContext_t *)MipiDrvHandle;

    // check state
    if ( pMipiDrvCtx->State != MIPI_DRV_STATE_RUNNING )
    {
        // already stopped
        result = RET_SUCCESS;
        goto success_exit;
    }

    // stop driver
    MipiDrvCmd_t cmd;
    cmd.ID = MIPI_DRV_CMD_STOP;
    result = MipiDrvCmd( pMipiDrvCtx, &cmd );
    if (RET_SUCCESS != result)
    {
        TRACE( MIPI_DRV_API_ERROR, "%s: MipiDrvCmd(MIPI_DRV_CMD_STOP) failed (result=%d)\n", __func__, result );
        goto error_exit;
    }

    // success, so set new state
    pMipiDrvCtx->State = MIPI_DRV_STATE_STOPPED;

success_exit:
    TRACE( MIPI_DRV_API_INFO, "%s: (exit)\n", __func__ );

    return result;

error_exit: // just return with error
    TRACE( MIPI_DRV_API_INFO, "%s: (exit, result=%d)\n", __func__, result );

    return result;
}


#ifdef SUBDEV_V4L2
#include <linux/videodev2.h>
#include <linux/media.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <memory.h>
#include <inttypes.h>
#include "vvcsioc.h"

int CsiOpen(const char* name) {
    int fd = open(name, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		char szDeviceName[64] = { 0 };
	    struct v4l2_capability cap;
	    for (int index = 0; index < 20; index++) {
	        sprintf(szDeviceName, "/dev/v4l-subdev%d", index);
	        fd = open(szDeviceName, O_RDWR | O_NONBLOCK);
	        if (fd < 0) {
	            continue;
	        }
	        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) != 0) {
	            continue;
	        }

	        if (strcmp((char*)cap.driver, "csi_samsung_subdev") == 0) {
	            // is sensor 2775
	            break;
	        }
	        close(fd);
	        fd = -1;
	    }

	    if (fd < 0) {
	        printf("can't open csi subdev!!!");
	        exit(1);
	    }
	}
    return fd;
}

void CsiClose(int fd) {
    close(fd);
}

int CsiSetFormat(int fd, int pad, int width, int height, int format) {
    struct csi_sam_format sam_format;
    printf("#### CsiSetFormat %d %d %d\n", width, height, format);
    sam_format.format = format;
    sam_format.width = width;
    sam_format.height = height;
    return ioctl(fd, VVCSIOC_S_FMT, &sam_format);
}

int CsiSetHdr(int fd, int pad, bool enable) {
    return ioctl(fd, VVCSIOC_S_HDR, &enable);
}

int CsiSetPower(int fd, bool enable) {
    int ret = 0;
    if (enable)
        ret = ioctl(fd, VVCSIOC_POWERON, 0);
    else
        ret = ioctl(fd, VVCSIOC_POWEROFF, 0);
    return ret;
}

int CsiStreamon(int fd) {
    return ioctl(fd, VVCSIOC_STREAMON, 0);
}

int CsiStreamoff(int fd) {
    return ioctl(fd, VVCSIOC_STREAMOFF, 0);
}
#else
int CsiOpen(const char* name) {
    return -1;
}

void CsiClose(int fd) {

}

int CsiSetFormat(int fd, int pad, int width, int height, int format) {

    return 0;
}

int CsiSetHdr(int fd, int pad, bool enable) {
    return 0;
}

int CsiSetPower(int fd, bool enable) {
    return 0;
}

int CsiStreamon(int fd) {
    return 0;
}

int CsiStreamoff(int fd) {
    return 0;
}

#endif
