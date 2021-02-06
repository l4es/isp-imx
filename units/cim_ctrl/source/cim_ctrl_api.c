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
 * @file cimm_ctrl_api.c
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
#include "cim_ctrl_api.h"


/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER( CIM_CTRL_API_INFO , "CIM-CTRL-API: ", INFO,    1 );
CREATE_TRACER( CIM_CTRL_API_WAR,   "CIM-CTRL-API: ", WARNING, 1 );
CREATE_TRACER( CIM_CTRL_API_ERROR, "CIM-CTRL-API: ", ERROR,   1 );



/******************************************************************************
 * CimCtrlInit()
 *****************************************************************************/
RESULT CimCtrlInit
(
    CimCtrlConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    CimCtrlContext_t *pCimCtrlCtx;

    TRACE( CIM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if ( pConfig == NULL )
    {
        return ( RET_INVALID_PARM );
    }

    if ( pConfig->MaxPendingCommands == 0 )
    {
        return ( RET_OUTOFRANGE );
    }

    /* allocate control context */
    pCimCtrlCtx = malloc( sizeof(CimCtrlContext_t) );
    if ( pCimCtrlCtx == NULL )
    {
        TRACE( CIM_CTRL_API_ERROR, "%s (allocating control context failed)\n", __func__ );
        return ( RET_OUTOFMEM );
    }
    memset( pCimCtrlCtx, 0, sizeof(CimCtrlContext_t) );

    /* pre initialize control context */
    pCimCtrlCtx->State              = eCimCtrlStateInvalid;
    pCimCtrlCtx->MaxCommands        = pConfig->MaxPendingCommands;
    pCimCtrlCtx->pUserContext       = pConfig->pUserContext;
    pCimCtrlCtx->cimCbCompletion    = pConfig->cimCbCompletion;
    pCimCtrlCtx->HalHandle          = pConfig->HalHandle;

    /* create control process */
    result = CimCtrlCreate( pCimCtrlCtx );
    if ( result != RET_SUCCESS )
    {
        TRACE( CIM_CTRL_API_ERROR, "%s (creating control process failed)\n", __func__ );
        free( pCimCtrlCtx );
    }
    else
    {
        /* control context is initialized, we're ready and in idle state */
        pCimCtrlCtx->State = eCimCtrlStateInitialize;

        /* success, so let's return control context */
        pConfig->hCimContext = (CimCtrlContextHandle_t)pCimCtrlCtx;
    }


    TRACE( CIM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( result );
}



/******************************************************************************
 * CimCtrlStart()
 *****************************************************************************/
RESULT CimCtrlStart
(
    CimCtrlContextHandle_t handle
)
{
    CimCtrlContext_t *pCimCtrlCtx = (CimCtrlContext_t *)handle;

    TRACE( CIM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if ( pCimCtrlCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ( CimCtrlGetState( pCimCtrlCtx ) != eCimCtrlStateInitialize )
            && ( CimCtrlGetState( pCimCtrlCtx) != eCimCtrlStateStopped ) )
    {
        return ( RET_WRONG_STATE );
    }

    CimCtrlSendCommand( pCimCtrlCtx, CIM_CTRL_CMD_START );

    TRACE( CIM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( RET_PENDING );
}



/******************************************************************************
 * CimCtrlStop()
 *****************************************************************************/
RESULT CimCtrlStop
(
    CimCtrlContextHandle_t handle
)
{
    CimCtrlContext_t *pCimCtrlCtx = (CimCtrlContext_t *)handle;

    TRACE( CIM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if ( pCimCtrlCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ( CimCtrlGetState( pCimCtrlCtx ) != eCimCtrlStateRunning )
            && ( CimCtrlGetState( pCimCtrlCtx ) != eCimCtrlStateStopped ) )
    {
        return ( RET_WRONG_STATE );
    }

    CimCtrlSendCommand( pCimCtrlCtx, CIM_CTRL_CMD_STOP );

    TRACE( CIM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( RET_PENDING );
}



/******************************************************************************
 * CimCtrlShutDown()
 *****************************************************************************/
RESULT CimCtrlShutDown
(
    CimCtrlContextHandle_t handle
)
{
    RESULT result = RET_SUCCESS;

    CimCtrlContext_t *pCimCtrlCtx = (CimCtrlContext_t *)handle;

    TRACE( CIM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if ( pCimCtrlCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ( CimCtrlGetState( pCimCtrlCtx ) != eCimCtrlStateInitialize )
            && ( CimCtrlGetState( pCimCtrlCtx) != eCimCtrlStateStopped ) )
    {
        return ( RET_WRONG_STATE );
    }

    result = CimCtrlDestroy( pCimCtrlCtx );
    if ( result != RET_SUCCESS )
    {
        TRACE( CIM_CTRL_API_ERROR, "%s (destroying control process failed -> RESULT=%d)\n", __func__, result);
    }

    /* release context memory */
    free( pCimCtrlCtx );

    TRACE( CIM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( result );
}



/******************************************************************************
 * CimCtrlRegisterSensorDrv()
 *****************************************************************************/
RESULT CimCtrlRegisterSensorDrv
(
    CimCtrlContextHandle_t  handle,
    IsiCamDrvConfig_t       *pIsiCamDrvConfig,
    const uint32_t          HalDevID,
    const uint8_t           I2cBusNum
)
{
    RESULT result = RET_SUCCESS;

    CimCtrlContext_t *pCimCtrlCtx = (CimCtrlContext_t *)handle;

    IsiSensorInstanceConfig_t   Config;
    IsiSensorHandle_t hSensor = NULL;

    TRACE( CIM_CTRL_API_INFO, "%s (enter)\n", __func__ );

    if ( pCimCtrlCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ( CimCtrlGetState( pCimCtrlCtx ) != eCimCtrlStateInitialize )
            && ( CimCtrlGetState( pCimCtrlCtx) != eCimCtrlStateStopped ) )
    {
        return ( RET_WRONG_STATE );
    }

    if ( ( pIsiCamDrvConfig == NULL )
            || ( pIsiCamDrvConfig->pfIsiGetSensorIss == NULL ) )
    {
        return ( RET_INVALID_PARM );
    }

    result = pIsiCamDrvConfig->pfIsiGetSensorIss( &(pIsiCamDrvConfig->IsiSensor) );
    if ( result != RET_SUCCESS )
    {
        return ( result );
    }

    Config.HalHandle = pCimCtrlCtx->HalHandle;
    Config.HalDevID  = HalDevID;
    Config.I2cBusNum = I2cBusNum;
    result = IsiCreateSensorIss( &Config );
    if ( result != RET_SUCCESS )
    {
        return ( result );
    }
    if ( Config.hSensor == NULL )
    {
        return ( RET_FAILURE );
    }

    hSensor =  Config.hSensor;

    /* enable sensor power */
    result = IsiSensorSetPowerIss( hSensor, BOOL_TRUE );
    if ( result != RET_SUCCESS )
    {
        TRACE( CIM_CTRL_API_ERROR, "Enabling sensor-power failed (%d).\n", result );
        return ( result );
    }

    /* check sensor connection */
    result = IsiCheckSensorConnectionIss( hSensor );
    if ( result != RET_SUCCESS )
    {
        TRACE( CIM_CTRL_API_ERROR, "Connecting to sensor failed (%d).\n", result );
        return ( result );
    }

    TRACE( CIM_CTRL_API_INFO, "%s (exit)\n", __func__ );

    return ( RET_SUCCESS );
}
