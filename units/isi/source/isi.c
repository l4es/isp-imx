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
 * @file isisup.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>
#include <sys/ioctl.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#ifdef SUBDEV_CHAR
#include "csi_ioctl.h"
#include "soc_ioctl.h"
#endif

/******************************************************************************
 * local macro definitions
 *****************************************************************************/
#ifdef SUBDEV_CHAR
CREATE_TRACER( ISI_INFO , "ISI: ", INFO,    1);
CREATE_TRACER( ISI_WARN , "ISI: ", WARNING, 1);
CREATE_TRACER( ISI_ERROR, "ISI: ", ERROR,   1);
#else
CREATE_TRACER( ISI_INFO , "ISI: ", INFO,    0);
CREATE_TRACER( ISI_WARN , "ISI: ", WARNING, 1);
CREATE_TRACER( ISI_ERROR, "ISI: ", ERROR,   1);
#endif
/*****************************************************************************/
/**
 *          IsiSetCsiConfig
 *
 * @brief   mipi config.
 *
 * @param   handle      Sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT IsiSetCsiConfig
(
    IsiSensorHandle_t   handle,
    uint32_t            clk
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }
#ifdef SUBDEV_CHAR
    IsiSensorCaps_t     Caps;
    result = IsiGetCapsIss(handle, &Caps );
    if (result != RET_SUCCESS)
    {
        TRACE( ISI_ERROR, "%s: IsiGetCapsIss Get MIPI LANE Error\n", __func__);
        return result;
    }

    HalContext_t *pHalCtx = pSensorCtx->HalHandle;
    if (pHalCtx == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    struct soc_control_context csi_power;
    csi_power.device_idx = pHalCtx->sensor_index;
    csi_power.control_value = BOOL_TRUE;
    result = ioctl(pHalCtx->soc_fd,VVSOC_IOC_S_POWER_CSI,&csi_power);
    if (result != 0)
    {
        TRACE( ISI_ERROR, "%s: Set csi power Error\n", __func__);
        return ( RET_FAILURE );
    }

    struct soc_control_context csi_clk;
    csi_clk.device_idx = pHalCtx->sensor_index;
    csi_clk.control_value = clk;
    result = ioctl(pHalCtx->soc_fd,VVSOC_IOC_S_CLOCK_CSI,&csi_clk);
    if (result != 0)
    {
        TRACE( ISI_ERROR, "%s: Set csi clk Error\n", __func__);
        return ( RET_FAILURE );
    }

    struct soc_control_context csi_reset;
    csi_reset.device_idx = pHalCtx->sensor_index;
    csi_reset.control_value = BOOL_FALSE;
    result = ioctl(pHalCtx->soc_fd,VVSOC_IOC_S_RESET_CSI,&csi_reset);
    sleep(1);
    csi_reset.device_idx = pHalCtx->sensor_index;
    csi_reset.control_value = BOOL_TRUE;
    result |= ioctl(pHalCtx->soc_fd,VVSOC_IOC_S_RESET_CSI,&csi_reset);
    if (result != 0)
    {
        TRACE( ISI_ERROR, "%s: Set csi reset Error\n", __func__);
        return ( RET_FAILURE );
    }

    struct vvcam_csi_lane_cfg csi_lane_cfg;
    csi_lane_cfg.mipi_lane_num = Caps.MipiLanes;
    result = ioctl(pHalCtx->csi_fd,VVCSI_IOC_S_LANE_CFG,&csi_lane_cfg);
    if (result != 0)
    {
        TRACE( ISI_ERROR, "%s: Set csi mipi lane Error\n", __func__);
        return ( RET_FAILURE );
    }

    uint32_t streaming_enable = 1;
    result = ioctl(pHalCtx->csi_fd,VVCSI_IOC_S_STREAM,&streaming_enable);
    if (result != 0)
    {
        TRACE( ISI_ERROR, "%s: Set csi mipi Starts Error\n", __func__);
        return ( RET_FAILURE );
    }
#endif
    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiCreateSensorIss
 *
 * @brief   Creates a new camera instance.
 *
 * @param   pConfig     configuration of the new sensor instance
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT IsiCreateSensorIss
(
    IsiSensorInstanceConfig_t   *pConfig
)
{
    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( (pConfig == NULL) || (pConfig->pSensor == NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    result = HalAddRef( pConfig->HalHandle );
    if ( result != RET_SUCCESS )
    {
        return ( result );
    }

    if ( pConfig->pSensor->pIsiCreateSensorIss == NULL )
    {
        (void)HalDelRef( pConfig->HalHandle );
        return ( RET_NOTSUPP );
    }

    result = pConfig->pSensor->pIsiCreateSensorIss( pConfig );
    if ( result != RET_SUCCESS )
    {
        (void)HalDelRef( pConfig->HalHandle );
    }

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

RESULT IsiInitSensorIss
(
    IsiSensorHandle_t   handle
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }


    if ( pSensorCtx->pSensor->pIsiInitSensorIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiInitSensorIss( pSensorCtx );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiGetSensorModeIss
 *
 * @brief   get cuurent sensor mode info.
 *
 * @param   handle      Sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 *
 *****************************************************************************/
RESULT IsiGetSensorModeIss
(
    IsiSensorHandle_t   handle,
    void *pmode
)
{
IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }


    if ( pSensorCtx->pSensor->pIsiGetSensorModeIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetSensorModeIss( pSensorCtx, pmode);

    struct vvcam_mode_info *psensor_mode = pmode;
    TRACE( ISI_INFO, "******************************\n");
    TRACE( ISI_INFO, "Current Sensor Mode:\n");
    TRACE( ISI_INFO, "Mode Index: %d \n",psensor_mode->index);
    TRACE( ISI_INFO, "Resolution: %d * %d\n",psensor_mode->width,psensor_mode->height);
    TRACE( ISI_INFO, "fps: %d \n",psensor_mode->fps);
    TRACE( ISI_INFO, "hdr_mode: %d \n",psensor_mode->hdr_mode);
    TRACE( ISI_INFO, "stitching_mode: %d \n",psensor_mode->stitching_mode);
    TRACE( ISI_INFO, "bit_width: %d \n",psensor_mode->bit_width);
    TRACE( ISI_INFO, "bayer_pattern: %d \n",psensor_mode->bayer_pattern);
    TRACE( ISI_INFO, "******************************\n");
    (void)psensor_mode; //avoid compiler's complaint

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );

}

/*****************************************************************************/
/**
 *          IsiReleaseSensorIss
 *
 * @brief   Destroys a camera instance.
 *
 * @param   handle      Sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 *
 *****************************************************************************/
RESULT IsiReleaseSensorIss
(
    IsiSensorHandle_t   handle
)
{
    uint32_t defMode;
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    result = HalGetSensorDefaultMode(pSensorCtx->HalHandle, &defMode);
    if(result != RET_SUCCESS)
    {
        TRACE( ISI_ERROR, "%s: Get the default mode index Error\n", __func__);
        return ( RET_FAILURE );
    }
    result = HalSetSensorMode(pSensorCtx->HalHandle, defMode);
    if(result != RET_SUCCESS)
    {
        TRACE( ISI_ERROR, "%s:  set the current mode index Error\n", __func__);
        return ( RET_FAILURE );
    }

    (void)HalDelRef( pSensorCtx->HalHandle );

    if ( pSensorCtx->pSensor->pIsiReleaseSensorIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiReleaseSensorIss( pSensorCtx );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiGetCapsIss
 *
 * @brief   fills in the correct pointers for the sensor description struct
 *
 * @param   handle      Sensor instance handle
 * @param   pCaps
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetCapsIss
(
    IsiSensorHandle_t   handle,
    IsiSensorCaps_t     *pCaps
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pCaps == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    result = pSensorCtx->pSensor->pIsiGetCapsIss( pSensorCtx, pCaps );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );

}



/*****************************************************************************/
/**
 *          IsiSetupSensorIss
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      Sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiSetupSensorIss
(
    IsiSensorHandle_t   handle,
    IsiSensorConfig_t   *pConfig
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pConfig == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiSetupSensorIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiSetupSensorIss( pSensorCtx, pConfig );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiChangeSensorResolutionIss
 *
 * @brief   Change image sensor resolution while keeping all other static settings.
 *          Dynamic settings like current exposure, gain & integration time are
 *          kept as close as possible.
 *
 * @note    Re-read current & min/max values as they will probably have changed!
 *
 * @param   handle                  Sensor instance handle
 * @param   width                   
 * @param   height   
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_WRONG_STATE
 * @retval  RET_OUTOFRANGE
 *
 *****************************************************************************/
RESULT IsiChangeSensorResolutionIss
(
    IsiSensorHandle_t   handle,
    uint16_t            width,
    uint16_t            height
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiChangeSensorResolutionIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiChangeSensorResolutionIss( pSensorCtx, width, height );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiSensorSetStreamingIss
 *
 * @brief   Enables/disables streaming of sensor data, if possible.
 *
 * @param   handle      Sensor instance handle
 * @param   on          new streaming state (BOOL_TRUE=on, BOOL_FALSE=off)
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NOTSUPP
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
RESULT IsiSensorSetStreamingIss
(
    IsiSensorHandle_t   handle,
    bool_t              on
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( (pSensorCtx == NULL) || (pSensorCtx->pSensor == NULL) )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiSensorSetStreamingIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiSensorSetStreamingIss( pSensorCtx, on );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiSensorSetPowerIss
 *
 * @brief   Performs the power-up/power-down sequence of the camera, if possible.
 *
 * @param   handle      Sensor instance handle
 * @param   on          new power state (BOOL_TRUE=on, BOOL_FALSE=off)
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NOTSUPP
 *
 *****************************************************************************/
RESULT IsiSensorSetPowerIss
(
    IsiSensorHandle_t   handle,
    bool_t              on
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( (pSensorCtx == NULL) || (pSensorCtx->pSensor == NULL) )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiSensorSetPowerIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiSensorSetPowerIss( pSensorCtx, on );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiCheckSensorConnectionIss
 *
 * @brief   Performs the power-up sequence of the camera, if possible.
 *
 * @param   handle      Sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiCheckSensorConnectionIss
(
    IsiSensorHandle_t   handle
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiCheckSensorConnectionIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiCheckSensorConnectionIss( pSensorCtx );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiGetSensorRevisionIss
 *
 * @brief   reads the sensor revision register and returns this value
 *
 * @param   Handle      pointer to sensor description struct
 * @param   p_value     pointer to storage value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetSensorRevisionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *p_value
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( (pSensorCtx == NULL) || (pSensorCtx->pSensor == NULL) )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( p_value == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetSensorRevisionIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result =  pSensorCtx->pSensor->pIsiGetSensorRevisionIss( handle, p_value );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiGetGainLimitsIss
 *
 * @brief   Returns the exposure minimal and maximal values of a sensor
 *          instance
 *
 * @param   handle       sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetGainLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinGain,
    float               *pMaxGain
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinGain == NULL) || (pMaxGain == NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetGainLimitsIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetGainLimitsIss( pSensorCtx, pMinGain, pMaxGain );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiGetIntegrationTimeLimitsIss
 *
 * @brief   Returns the exposure minimal and maximal values of a sensor
 *          instance
 *
 * @param   handle       sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetIntegrationTimeLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinIntegrationTime,
    float               *pMaxIntegrationTime
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinIntegrationTime == NULL) || (pMaxIntegrationTime == NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetIntegrationTimeLimitsIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetIntegrationTimeLimitsIss( pSensorCtx, pMinIntegrationTime, pMaxIntegrationTime );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiExposureControlIss
 *
 * @brief   Sets the exposure minimal and maximal values of a sensor
 *          instance
 *
 * @param   handle       sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiExposureControlIss
(
    IsiSensorHandle_t   handle,
    float               NewGain,
    float               NewIntegrationTime,
    uint8_t             *pNumberOfFramesToSkip,
    float               *pSetGain,
    float               *pSetIntegrationTime,
    float               *hdr_ratio
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( (pNumberOfFramesToSkip == NULL)
            || (pSetGain == NULL)
            || (pSetIntegrationTime == NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiExposureControlIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiExposureControlIss( pSensorCtx, NewGain, NewIntegrationTime, pNumberOfFramesToSkip, pSetGain, pSetIntegrationTime, hdr_ratio);

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiGetCurrentExposureIss
 *
 * @brief   Returns the currently adjusted AE values
 *
 * @param   handle       sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetCurrentExposureIss
(
    IsiSensorHandle_t   handle,
    float               *pCurGain,
    float               *pCurIntegrationTime
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( (pCurGain == NULL) || (pCurIntegrationTime == NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetCurrentExposureIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetCurrentExposureIss( pSensorCtx, pCurGain, pCurIntegrationTime );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiGetGainIss
 *
 *****************************************************************************/
RESULT IsiGetGainIss
(
    IsiSensorHandle_t   handle,
    float               *pGain
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pGain == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetGainIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetGainIss( pSensorCtx, pGain );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

RESULT IsiGetVSGainIss
(
    IsiSensorHandle_t   handle,
    float               *pGain
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;
    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pGain == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetVSGainIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetVSGainIss( pSensorCtx, pGain );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

RESULT IsiGetLongGainIss
(
    IsiSensorHandle_t   handle,
    float               *pGain
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;
    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pGain == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetLongGainIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetLongGainIss( pSensorCtx, pGain );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}


/*****************************************************************************/
/**
 *          IsiGetGainIncrementIss
 *
 *****************************************************************************/
RESULT IsiGetGainIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIncr == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetGainIncrementIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetGainIncrementIss( pSensorCtx, pIncr );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

/*****************************************************************************/
/**
 *          IsiSetGainIss
 *
 *****************************************************************************/
RESULT IsiSetGainIss
(
    IsiSensorHandle_t   handle,
    float               NewGain,
    float               *pSetGain,
    float               *hdr_ratio
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetGain == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiSetGainIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiSetGainIss( pSensorCtx, NewGain, pSetGain, hdr_ratio);

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiGetIntegrationTimeIss
 *
 *****************************************************************************/
RESULT IsiGetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               *pIntegrationTime
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIntegrationTime == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetIntegrationTimeIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetIntegrationTimeIss( pSensorCtx, pIntegrationTime );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

RESULT IsiGetVSIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               *pIntegrationTime
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIntegrationTime == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetVSIntegrationTimeIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetVSIntegrationTimeIss( pSensorCtx, pIntegrationTime );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

RESULT IsiGetLongIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               *pIntegrationTime
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIntegrationTime == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetLongIntegrationTimeIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetLongIntegrationTimeIss( pSensorCtx, pIntegrationTime );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}




/*****************************************************************************/
/**
 *          IsiGetIntegrationTimeIncrementIss
 *
 *****************************************************************************/
RESULT IsiGetIntegrationTimeIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIncr == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetIntegrationTimeIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetIntegrationTimeIncrementIss( pSensorCtx, pIncr );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiSetIntegrationTimeIss
 *
 *****************************************************************************/
RESULT IsiSetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               NewIntegrationTime,
    float               *pSetIntegrationTime,
    float               *hdr_ratio
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    uint8_t NumberOfFramesToSkip = 0U;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetIntegrationTime == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiSetIntegrationTimeIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiSetIntegrationTimeIss( pSensorCtx, NewIntegrationTime, pSetIntegrationTime, &NumberOfFramesToSkip, hdr_ratio );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiGetSensorFpsIss
 *
 * @brief   Get Sensor Fps Config.
 *
 * @param   handle                  sensor instance handle
 * @param   pFps                    current fps
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetSensorFpsIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *pFps
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pFps == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetSensorFpsIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiGetSensorFpsIss( pSensorCtx, pFps );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiSetSensorFpsIss
 *
 * @brief   set Sensor Fps Config.
 *
 * @param   handle                  sensor instance handle
 * @param   Fps                     Setfps
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiSetSensorFpsIss
(
    IsiSensorHandle_t   handle,
    uint32_t            Fps
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiSetSensorFpsIss == NULL )
    {
        TRACE( ISI_INFO, "%s: pIsiSetSensorFpsIss is NULL\n", __func__);
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiSetSensorFpsIss( pSensorCtx, Fps );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiGetResolutionIss
 *
 * @brief   Reads integration time values from the image sensor module.
 *
 * @param   handle                  sensor instance handle
 * @param   pwidth    
 * @param   pheight      
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetResolutionIss
(
    IsiSensorHandle_t   handle,
    uint16_t *pwidth,
    uint16_t *pheight
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    result = pSensorCtx->pSensor->pIsiGetResolutionIss( pSensorCtx, pwidth, pheight );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiQuerySensorIss
 *
 * @brief   query sensor info arry.
 *
 * @param   handle                  sensor instance handle
 * @param   pSensorInfo             sensor query arry
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiQuerySensorIss
(
    IsiSensorHandle_t   handle,
    vvcam_mode_info_array_t *pSensorInfo
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorInfo == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiQuerySensorIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiQuerySensorIss( pSensorCtx, pSensorInfo );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

/*****************************************************************************/
/**
 *          IsiMdiInitMotoDrive
 *
 * @brief   General initialisation tasks like I/O initialisation.
 *
 * @param   handle          sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiMdiInitMotoDrive
(
    IsiSensorHandle_t   handle
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( (pSensorCtx == NULL) || (pSensorCtx->pSensor == NULL) )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiMdiInitMotoDriveMds == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiMdiInitMotoDriveMds( pSensorCtx );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiMdiSetupMotoDrive
 *
 * @brief   Setup of the MotoDrive and return possible max step.
 *
 * @param   handle          sensor instance handle
 *          pMaxStep        pointer to variable to receive the maximum
 *                          possible focus step
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiMdiSetupMotoDrive
(
    IsiSensorHandle_t   handle,
    uint32_t            *pMaxStep
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( (pSensorCtx == NULL) || (pSensorCtx->pSensor == NULL) )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pMaxStep == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiMdiSetupMotoDrive == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiMdiSetupMotoDrive( pSensorCtx, pMaxStep );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiMdiFocusSet
 *
 * @brief   Drives the lens system to a certain focus point.
 *
 * @param   handle          sensor instance handle
 *          AbsStep         absolute focus point to apply
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiMdiFocusSet
(
    IsiSensorHandle_t   handle,
    const uint32_t      AbsStep
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiMdiFocusSet == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiMdiFocusSet( pSensorCtx, AbsStep );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiMdiFocusGet
 *
 * @brief   Retrieves the currently applied focus point.
 *
 * @param   handle          sensor instance handle
 *          pAbsStep        pointer to a variable to receive the current
 *                          focus point
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiMdiFocusGet
(
    IsiSensorHandle_t   handle,
    uint32_t            *pAbsStep
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pAbsStep == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiMdiFocusGet == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiMdiFocusGet( pSensorCtx, pAbsStep );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiMdiFocusCalibrate
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiMdiFocusCalibrate
(
    IsiSensorHandle_t   handle
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiMdiFocusCalibrate == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiMdiFocusCalibrate( pSensorCtx );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiActivateTestPattern
 *
 * @brief   Activates or deactivates sensor's test-pattern (normally a defined
 *          colorbar )
 *
 * @param   handle          sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiActivateTestPattern
(
    IsiSensorHandle_t   handle,
    const bool_t        enable
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiActivateTestPattern == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiActivateTestPattern( pSensorCtx, enable );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}


RESULT IsiEnableHdr
(
    IsiSensorHandle_t   handle,
    const bool_t        enable
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiEnableHdr == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiEnableHdr( pSensorCtx, enable );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

RESULT IsiResetSensor
(
    IsiSensorHandle_t   handle
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiResetSensorIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiResetSensorIss( pSensorCtx);

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

RESULT IsiSetBayerPattern
(
    IsiSensorHandle_t handle,
    uint8_t pattern
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiSetBayerPattern == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiSetBayerPattern( pSensorCtx, pattern );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

/*****************************************************************************/
/**
 *          IsiDumpAllRegisters
 *
 * @brief   Activates or deactivates sensor's test-pattern (normally a defined
 *          colorbar )
 *
 * @param   handle          sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiDumpAllRegisters
(
    IsiSensorHandle_t   handle,
    const uint8_t       *filename
)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pRegisterTable == NULL )
    {
        return ( RET_NOTSUPP );
    }

    const IsiRegDescription_t* ptReg = pSensorCtx->pSensor->pRegisterTable;

    FILE* pFile;
    pFile = fopen ( (const char *)filename, "w" );

    fprintf(pFile,"*************************************************************\n");
    fprintf(pFile,"* IMAGE SENSOR REGISTERS                                    *\n");
    fprintf(pFile,"*************************************************************\n");

    while (ptReg->Flags != eTableEnd)
    {
        if (ptReg->Flags & (eReadWrite))
        {
            fprintf(pFile, " %-30s @ 0x%04X", ptReg->pName, ptReg->Addr);

            if (ptReg->Flags & eReadable)
            {
                uint32_t value = 0U;
                RESULT Res = RET_SUCCESS;
                Res = IsiReadRegister( handle, ptReg->Addr, &value );
                if (Res == RET_SUCCESS)
                {
                    fprintf(pFile, " = 0x%08X", value);
                    if (ptReg->Flags & eNoDefault)
                    {
                        fprintf(pFile, "\n");
                    }
                    else if (value == ptReg->DefaultValue)
                    {
                        fprintf(pFile, " (= default value)\n");
                    }
                    else
                    {
                        fprintf(pFile, " (default was 0x%08X)\n", ptReg->DefaultValue);
                    }
                }
                else
                {
                    fprintf(pFile, " <read failure %d>\n", Res);
                }
            }
            else if (ptReg->Flags & eWritable)
            {
                fprintf(pFile, " <is only writable>\n");
            }
        }
        ++ptReg;
    }

    fclose (pFile);

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

/*****************************************************************************/
/**
 *          IsiTryToSetConfigFromPreferredCaps
 *
 * @brief   Tries to set the referenced sensor config parameter to the first of the
 *          given preferred capabilities that is included in the given capability
 *          mask. If none of the preferred capabilities is supported, the config
 *          parameter value remains unchanged.
 *
 * @note    Use this function for example to modify the retrieved default sensor
 *          config parameter for parameter according to some external preferences
 *          while taking the retrieved sensor capabilities for that config parameter
 *          into account.
 *
 * @param   pConfigParam    reference to parameter of sensor config structure
 * @param   prefList        reference to 0 (zero) terminated array of preferred
 *                          capability values in descending order
 * @param   capsmask        bitmask of supported capabilites for that parameter
 *
 * @return  Return the result of the function call.
 * @retval  BOOL_TRUE       preferred capability set in referenced config parameter
 * @retval  BOOL_FALSE      preferred capability not supported
 *
 *****************************************************************************/
bool_t IsiTryToSetConfigFromPreferredCaps
(
    uint32_t    *pConfigParam,
    uint32_t    *prefList,
    uint32_t    capsmask
)
{
    uint32_t i;

    for(i=0; prefList[i]!=0 ; i++)
    {
        if( (capsmask & prefList[i]) != 0 )
        {
            break;
        }
    }

    if (prefList[i] != 0)
    {
        *pConfigParam = prefList[i];
        return BOOL_TRUE;
    }

    return BOOL_FALSE;
}

/*****************************************************************************/
/**
 *          IsiTryToSetConfigFromPreferredCap
 *
 * @brief   Tries to set referenced sensor config parameter to the given preferred
 *          capability while checking that capability against the given capability
 *          mask. If that capability isn't supported, the config parameter value
 *          remains unchanged.
 *
 * @note    Use this function for example to modify the retrieved default sensor
 *          config parameter for parameter according to some external preferences
 *          while taking the retrieved sensor capabilities for that config parameter
 *          into account.
 *
 * @param   pConfigParam    reference to parameter of sensor config structure
 * @param   prefcap         preferred capability value
 * @param   capsmask        bitmask of supported capabilites for that parameter
 *
 * @return  Return the result of the function call.
 * @retval  BOOL_TRUE       preferred capability set in referenced config parameter
 * @retval  BOOL_FALSE      preferred capability not supported
 *
 *****************************************************************************/
bool_t IsiTryToSetConfigFromPreferredCap
(
    uint32_t        *pConfigParam,
    const uint32_t  prefcap,
    const uint32_t  capsmask
)
{
    if( (capsmask & prefcap) != 0 )
    {
        *pConfigParam = prefcap;
        return BOOL_TRUE;
    }

    return BOOL_FALSE;
}



/*****************************************************************************/
/**
 *          IsiGetSensorAWBMode
 *
 * @brief   get sensor awb mode ( workaround )
 *
 * Used to modify the AWB control loop handling in case of sensors that require
 * on-sensor whitebalance gain
 *
 * @param        handle          sensor instance handle
 * @param[out]   pawb_mode       awb mode the sensor is operating in currently
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT IsiGetSensorAWBMode(IsiSensorHandle_t handle, IsiSensorAwbMode_t *pawb_mode){
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pawb_mode == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    if ( pSensorCtx->pSensor->pIsiGetSensorAWBModeIss == NULL ){
    
        *pawb_mode = ISI_SENSOR_AWB_MODE_NORMAL;
        return ( RET_SUCCESS );
    }

    result = pSensorCtx->pSensor->pIsiGetSensorAWBModeIss( pSensorCtx, pawb_mode );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiSensorSetBlc
 *
 * @brief   set sensor linear mode black level 
 *          
 *
 * @param   handle          sensor instance handle
 * @param   pblc            blc params point
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT IsiSensorSetBlc(IsiSensorHandle_t handle, sensor_blc_t * pblc)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiSensorSetBlcIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiSensorSetBlcIss( pSensorCtx, pblc );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiSensorSetWB
 *
 * @brief   set sensor linear mode white balance 
 *          or hdr mode normal exp frame white balance
 *
 * @param   handle          sensor instance handle
 * @param   pwb             wb params point
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT IsiSensorSetWB(IsiSensorHandle_t handle, sensor_white_balance_t *pwb)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiSensorSetWBIss == NULL )
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiSensorSetWBIss( pSensorCtx, pwb );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}



/*****************************************************************************/
/**
 *          IsiSensorGetExpandCurve
 *
 * @brief   get sensor expand curve 
 *
 * @param   handle          sensor instance handle
 * @param   pexpand_curve   expand cure point
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT IsiSensorGetExpandCurve(IsiSensorHandle_t handle, sensor_expand_curve_t * pexpand_curve)
{
    IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( ISI_INFO, "%s: (enter)\n", __func__);

    if ( pSensorCtx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pSensorCtx->pSensor->pIsiSensorGetExpandCurveIss == NULL || pexpand_curve == NULL)
    {
        return ( RET_NOTSUPP );
    }

    result = pSensorCtx->pSensor->pIsiSensorGetExpandCurveIss( pSensorCtx, pexpand_curve );

    TRACE( ISI_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

