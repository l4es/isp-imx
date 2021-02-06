/******************************************************************************\
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <common/return_codes.h>
#include <common/misc.h>
#include <fcntl.h>
#include <math.h>
#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "vvsensor.h"
#include "IMX290_priv.h"

#ifdef SENSOR_V4L2_DEV
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#endif

CREATE_TRACER( IMX290_INFO , "IMX290: ", INFO,    1);
CREATE_TRACER( IMX290_WARN , "IMX290: ", WARNING, 1);
CREATE_TRACER( IMX290_ERROR, "IMX290: ", ERROR,   1);
CREATE_TRACER( IMX290_DEBUG,     "IMX290: ", INFO, 1);
CREATE_TRACER( IMX290_REG_INFO , "IMX290: ", INFO, 1);
CREATE_TRACER( IMX290_REG_DEBUG, "IMX290: ", INFO, 1);

#define IMX290_SLAVE_ADDR       0x36            /**< i2c slave address of the IMX290 camera sensor */
#define IMX290_MIN_GAIN_STEP    ( 1.0f/16.0f )  /**< min gain step size used by GUI (hardware min = 1/16; 1/16..32/16 depending on actual gain ) */
#define IMX290_MAX_GAIN_AEC     ( 32.0f )       /**< max. gain used by the AEC (arbitrarily chosen, hardware limit = 62.0, driver limit = 32.0 ) */
#define IMX290_VS_MAX_INTEGRATION_TIME (0.0018)
#define IMX290_VTS_NUM          0x4705
#define IMX290_HTS_NUM          0x1130
#define IMX290_PIX_CLOCK        (37.125f)


#ifdef SENSOR_V4L2_DEV
int fd;
#endif
extern const IsiRegDescription_t IMX290_g_aRegDescription[];
const IsiSensorCaps_t IMX290_g_IsiSensorDefaultConfig;

/*****************************************************************************
 *Sensor Info
*****************************************************************************/
static const uint32_t SensorClkIn = 24000000;
static const char SensorName[16] = "IMX290";
static const uint32_t SensorRevChipID = 0x290;
static const IsiSccbInfo_t SensorSccbInfo = {
    .slave_addr = (0x34>>1),
    .addr_byte  = 2,
    .data_byte  = 1,
};
static const uint32_t SensorDrvSupportResoluton[] = {
    ISI_RES_TV720P,
    ISI_RES_TV1080P
};

static RESULT IMX290_IsiSensorSetPowerIss
(
    IsiSensorHandle_t   handle,
    bool_t              on
)
{
    RESULT result = RET_SUCCESS;

    int32_t ret = 0;

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    int32_t enable = on;
 	ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_POWER, &enable);
    if (ret != 0)
    {
        TRACE( IMX290_ERROR, "%s: sensor set power error!\n", __func__);
        return ( RET_FAILURE );
    }

    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}

static RESULT IMX290_IsiResetSensorIss
(
 IsiSensorHandle_t handle
)
{
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;

    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    ret = ioctl(pHalCtx->sensor_fd,VVSENSORIOC_RESET, NULL);
    if (ret != 0)
    {
        TRACE( IMX290_ERROR, "%s: sensor reset error!\n", __func__);
        return ( RET_FAILURE );
    }

    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}


static RESULT IMX290_IsiSensorSetClkIss
(
    IsiSensorHandle_t   handle,
    uint32_t              clk
)
{
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    ret = ioctl(pHalCtx->sensor_fd,VVSENSORIOC_S_CLK, &clk);
	if (ret != 0)
    {
        TRACE( IMX290_ERROR, "%s: sensor set clk error!\n", __func__);
        return ( RET_FAILURE );
    }

    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}

static RESULT IMX290_IsiSensorGetClkIss
(
    IsiSensorHandle_t   handle,
    uint32_t              *pclk
)
{
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_CLK, pclk);
	if (ret != 0)
    {
        TRACE( IMX290_ERROR, "%s: sensor get clk error!\n", __func__);
        return ( RET_FAILURE );
    }

    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}


static RESULT IMX290_IsiConfigSensorSCCBIss(IsiSensorHandle_t   handle)
{
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    struct vvcam_sccb_cfg_s  sensor_sccb_config;
    sensor_sccb_config.slave_addr = SensorSccbInfo.slave_addr;
    sensor_sccb_config.addr_byte = SensorSccbInfo.addr_byte;
    sensor_sccb_config.data_byte = SensorSccbInfo.data_byte;

    ret = ioctl(pHalCtx->sensor_fd,VVSENSORIOC_SENSOR_SCCB_CFG, &sensor_sccb_config);
	if (ret != 0)
    {
        TRACE( IMX290_ERROR, "%s: sensor config sccb info error!\n", __func__);
        return ( RET_FAILURE );
    }


    TRACE( IMX290_INFO, "%s (exit) result = %d\n", __func__,result);
    return ( result );
}

static RESULT IMX290_IsiRegisterReadIss
(
    IsiSensorHandle_t   handle,
    const uint32_t      address,
    uint32_t            *p_value
)
{
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    struct vvcam_sccb_data sccb_data;
    sccb_data.addr = address;
    sccb_data.data = 0;
    ret = ioctl(pHalCtx->sensor_fd,VVSENSORIOC_READ_REG, &sccb_data);
	if (ret != 0)
    {
        TRACE( IMX290_ERROR, "%s: read sensor register error!\n", __func__);
        return ( RET_FAILURE );
    }

    *p_value = sccb_data.data;

    TRACE( IMX290_INFO, "%s (exit) result = %d\n", __func__,result);
    return ( result );
}

static RESULT IMX290_IsiRegisterWriteIss
(
    IsiSensorHandle_t   handle,
    const uint32_t      address,
    const uint32_t      value
)
{
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    struct vvcam_sccb_data sccb_data;
    sccb_data.addr = address;
    sccb_data.data = value;

    ret = ioctl(pHalCtx->sensor_fd,VVSENSORIOC_WRITE_REG, &sccb_data);
	if (ret != 0)
    {
        TRACE( IMX290_ERROR, "%s: write sensor register error!\n", __func__);
        return ( RET_FAILURE );
    }

    TRACE( IMX290_INFO, "%s (exit) result = %d\n", __func__,result);
    return ( result );
}

static RESULT IMX290_IsiCreateSensorIss
(
    IsiSensorInstanceConfig_t   *pConfig
)
{
    RESULT result = RET_SUCCESS;
    IMX290_Context_t *pIMX290Ctx;

    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    if (!pConfig || !pConfig->pSensor)
        return ( RET_NULL_POINTER );

    pIMX290Ctx = ( IMX290_Context_t * )malloc ( sizeof (IMX290_Context_t) );
    if (!pIMX290Ctx) {
        TRACE( IMX290_ERROR,  "%s: Can't allocate ov2775 context\n",  __func__ );
        return ( RET_OUTOFMEM );
    }

    MEMSET( pIMX290Ctx, 0, sizeof( IMX290_Context_t ) );

    result = HalAddRef( pConfig->HalHandle );
    if ( result != RET_SUCCESS ) {
        free ( pIMX290Ctx );
        return ( result );
    }

    pIMX290Ctx->IsiCtx.HalHandle           = pConfig->HalHandle;
    pIMX290Ctx->IsiCtx.pSensor             = pConfig->pSensor;
    pIMX290Ctx->GroupHold           = BOOL_FALSE;
    pIMX290Ctx->OldGain             = 0;
    pIMX290Ctx->OldIntegrationTime  = 0;
    pIMX290Ctx->Configured          = BOOL_FALSE;
    pIMX290Ctx->Streaming           = BOOL_FALSE;
    pIMX290Ctx->TestPattern         = BOOL_FALSE;
    pIMX290Ctx->isAfpsRun           = BOOL_FALSE;

    pIMX290Ctx->HdrMode             = pConfig->HdrMode;
    pIMX290Ctx->Resolution          = pConfig->Resolution;
    pIMX290Ctx->MaxFps              = pConfig->MaxFps;
    pIMX290Ctx->MinFps              = pConfig->MinFps;

    pConfig->hSensor = ( IsiSensorHandle_t )pIMX290Ctx;


    if (pIMX290Ctx->HdrMode == SENSOR_MODE_LINEAR &&
        pIMX290Ctx->Resolution == ISI_RES_TV1080P &&
        pIMX290Ctx->MaxFps == 30)
    {
        memcpy(pIMX290Ctx->SensorRegCfgFile, "IMX290_mipi4lane_1080p_init.txt",strlen("IMX290_mipi4lane_1080p_init.txt"));
    }else
    {
        memcpy(pIMX290Ctx->SensorRegCfgFile, "IMX290_mipi4lane_1080p_init.txt",strlen("IMX290_mipi4lane_1080p_init.txt"));
    }


    if (access(pIMX290Ctx->SensorRegCfgFile,F_OK) == 0)
    {
        pIMX290Ctx->KernelDriverFlag = 0;
    }else
    {
        pIMX290Ctx->KernelDriverFlag = 1;
    }


    result = IMX290_IsiSensorSetPowerIss( pIMX290Ctx, BOOL_TRUE );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    if (pIMX290Ctx->KernelDriverFlag )
    {
         result = IMX290_IsiSensorGetClkIss( pIMX290Ctx, &SensorClkIn );
         RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
    }

    result = IMX290_IsiSensorSetClkIss( pIMX290Ctx, SensorClkIn );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    result = IMX290_IsiResetSensorIss(pIMX290Ctx);
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    if (!pIMX290Ctx->KernelDriverFlag)
    {
        result = IMX290_IsiConfigSensorSCCBIss(pIMX290Ctx);
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
    }

    pIMX290Ctx->pattern = ISI_BPAT_GRGRBGBG;
#ifdef SENSOR_V4L2_DEV
    pIMX290Ctx->subdev = -1;

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

        if (strcmp((char*)cap.driver, "ov2775") == 0) {
            // is sensor 2775
            break;
        }
        fd = -1;
    }

    if (fd < 0) {
        printf("can't open ov2775 subdev!!!");
        exit(1);
    }

    pIMX290Ctx->subdev = fd;
#endif

    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}

static RESULT IMX290_IsiGetRegCfgIss(const char *registerFileName, struct vvcam_sccb_array * arry)
{
    RESULT result = RET_SUCCESS;

    if (NULL == registerFileName)
    {
        TRACE( IMX290_ERROR, "%s:registerFileName is NULL\n", __func__);
        return (RET_NULL_POINTER);
    }

    FILE *fp = NULL;
    fp = fopen(registerFileName, "rb");
    if (!fp)
    {
        TRACE( IMX290_ERROR, "%s:load register file  %s error!\n", __func__,registerFileName);
        return (RET_FAILURE);
    }

    unsigned char LineBuf[512];
    unsigned int FileTotalLine = 0;
    while (!feof(fp))
    {
        fgets(LineBuf, 512, fp);
        FileTotalLine++;
    }

    arry->sccb_data = malloc(FileTotalLine * sizeof(struct vvcam_sccb_data));
    if (arry->sccb_data ==  NULL)
    {
        TRACE( IMX290_ERROR, "%s:malloc failed NULL Point!\n", __func__,registerFileName);
        return (RET_FAILURE);
    }
    rewind(fp);

    arry->count = 0;
    while (!feof(fp))
    {
        memset(LineBuf,0,sizeof(LineBuf));
        fgets(LineBuf, 512, fp);

        result = sscanf(LineBuf,"0x%x 0x%x", &(arry->sccb_data[arry->count].addr),
                                             &(arry->sccb_data[arry->count].data));
        if (result != 2) continue;
        arry->count++;

    }

    return 0;
}

static RESULT IMX290_IsiGetPicSize(uint32_t Resolution, uint32_t *pwidth, uint32_t *pheight)
{
    switch(Resolution)
    {
        case ISI_RES_TV1080P:
        {
            *pwidth = 1920;
            *pheight = 1080;
            break;
        }
        case ISI_RES_TV720P:
        {
            *pwidth = 1280;
            *pheight = 720;
            break;
        }
        default:
        {
            return ( RET_NOTSUPP );
        }
    }
    return ( RET_SUCCESS );
}

static RESULT IMX290_IsiInitSensorIss
(
    IsiSensorHandle_t   handle
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;

    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;


    TRACE( IMX290_INFO, "%s (enter)\n", __func__);
    if ( pIMX290Ctx == NULL )
	{
	    return ( RET_WRONG_HANDLE );
	}
    if (pIMX290Ctx->KernelDriverFlag)
    {
        struct sensor_mode_s  sensor_mode;
        IMX290_IsiGetPicSize(pIMX290Ctx->Resolution,&sensor_mode.width, &sensor_mode.height);
        sensor_mode.max_fps = pIMX290Ctx->MaxFps;
        sensor_mode.hdr_mode = pIMX290Ctx->HdrMode;

        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_INIT,&sensor_mode);
        if (ret != 0)
        {
            TRACE( IMX290_ERROR, "%s:sensor init error!\n", __func__);
            return ( RET_FAILURE );
        }
        struct vvcam_ae_info_s ae_info;
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_AE_INFO,&ae_info);
        if (ret != 0)
        {
            TRACE( IMX290_ERROR, "%s:sensor get ae info error!\n", __func__);
            return ( RET_FAILURE );
        }
        pIMX290Ctx->one_line_exp_time    = (float)ae_info.one_line_exp_time_ns / 1000000000;
        pIMX290Ctx->MaxIntegrationLine   = ae_info.max_interrgation_time;
        pIMX290Ctx->MinIntegrationLine   = ae_info.min_interrgation_time;
        pIMX290Ctx->gain_accuracy        = ae_info.gain_accuracy;
        pIMX290Ctx->AecMinGain           = (float)(ae_info.min_gain)/ae_info.gain_accuracy;
        pIMX290Ctx->AecMaxGain           = (float)(ae_info.max_gain)/ae_info.gain_accuracy;

    }else
    {
        struct vvcam_sccb_array arry;
        result = IMX290_IsiGetRegCfgIss(pIMX290Ctx->SensorRegCfgFile,&arry);
        if (result != 0)
        {
            TRACE( IMX290_ERROR, "%s:IMX290_IsiGetRegCfgIss error!\n", __func__);
            return ( RET_FAILURE );
        }

        ret = ioctl(pHalCtx->sensor_fd,VVSENSORIOC_WRITE_ARRAY,&arry);
        if (ret != 0)
        {
            TRACE( IMX290_ERROR, "%s:Senso Write Reg arry error!\n", __func__);
            return ( RET_FAILURE );
        }
        if (pIMX290Ctx->HdrMode == SENSOR_MODE_LINEAR &&
        pIMX290Ctx->Resolution == ISI_RES_TV1080P &&
        pIMX290Ctx->MaxFps == 30)
        {
            pIMX290Ctx->one_line_exp_time = 0.000069607;
            pIMX290Ctx->FrameLengthLines = 0x438;
            pIMX290Ctx->CurFrameLengthLines = pIMX290Ctx->FrameLengthLines;
            pIMX290Ctx->MaxIntegrationLine =  pIMX290Ctx->CurFrameLengthLines - 3;
            pIMX290Ctx->MinIntegrationLine = 1;
            pIMX290Ctx->AecMaxGain = 24;
            pIMX290Ctx->AecMinGain = 1;
            pIMX290Ctx->CurrFps = pIMX290Ctx->MaxFps;

        }else
        {
            pIMX290Ctx->one_line_exp_time = 0.000069607;
            pIMX290Ctx->FrameLengthLines = 0x438;
            pIMX290Ctx->CurFrameLengthLines = pIMX290Ctx->FrameLengthLines;
            pIMX290Ctx->MaxIntegrationLine =  pIMX290Ctx->CurFrameLengthLines - 3;
            pIMX290Ctx->MinIntegrationLine = 1;
            pIMX290Ctx->AecMaxGain = 24;
            pIMX290Ctx->AecMinGain = 1;
            pIMX290Ctx->CurrFps = pIMX290Ctx->MaxFps;
        }
    }

    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}


static RESULT IMX290_IsiReleaseSensorIss
(
    IsiSensorHandle_t   handle
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    if ( pIMX290Ctx == NULL )
        return ( RET_WRONG_HANDLE );

    (void)IMX290_IsiSensorSetStreamingIss( pIMX290Ctx, BOOL_FALSE );
    (void)IMX290_IsiSensorSetPowerIss( pIMX290Ctx, BOOL_FALSE );
    (void)HalDelRef( pIMX290Ctx->IsiCtx.HalHandle );

#ifdef SENSOR_V4L2_DEV
    if (pIMX290Ctx->subdev >= 0)
        close(pIMX290Ctx->subdev);
#endif

    MEMSET( pIMX290Ctx, 0, sizeof( IMX290_Context_t ) );
    free ( pIMX290Ctx );
#ifdef SENSOR_V4L2_DEV
    close(fd);
#endif
    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}

static RESULT IMX290_IsiGetCapsIss
(
    IsiSensorHandle_t   handle,
    IsiSensorCaps_t     *pIsiSensorCaps
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    if ( pIMX290Ctx == NULL )
        return ( RET_WRONG_HANDLE );

    if ( pIsiSensorCaps == NULL ) {
        return ( RET_NULL_POINTER );
    } else {
        pIsiSensorCaps->BusWidth        = ISI_BUSWIDTH_12BIT;
        pIsiSensorCaps->Mode            = ISI_MODE_BAYER;
        pIsiSensorCaps->FieldSelection  = ISI_FIELDSEL_BOTH;
        pIsiSensorCaps->YCSequence      = ISI_YCSEQ_YCBYCR;//  ISI_YCSEQ_YCRYCB;//ISI_YCSEQ_YCBYCR;           /**< only Bayer supported, will not be evaluated */
        pIsiSensorCaps->Conv422         = ISI_CONV422_NOCOSITED;//ISI_CONV422_INTER;//ISI_CONV422_NOCOSITED;
        pIsiSensorCaps->BPat            = pIMX290Ctx->pattern;
        pIsiSensorCaps->HPol            = ISI_HPOL_REFPOS;
        pIsiSensorCaps->VPol            = ISI_VPOL_NEG;//ISI_VPOL_POS;//ISI_VPOL_NEG;
        pIsiSensorCaps->Edge            = ISI_EDGE_RISING;//ISI_EDGE_FALLING;//ISI_EDGE_RISING;
        pIsiSensorCaps->Bls             = ISI_BLS_OFF;
        pIsiSensorCaps->Gamma           = ISI_GAMMA_OFF;
        pIsiSensorCaps->CConv           = ISI_CCONV_OFF;

        pIsiSensorCaps->Resolution      = pIMX290Ctx->Resolution;//( ISI_RES_TV1080P24   | ISI_RES_TV1080P20 | ISI_RES_TV1080P15 | ISI_RES_TV1080P6

        pIsiSensorCaps->BLC             = ( ISI_BLC_AUTO | ISI_BLC_OFF );
        pIsiSensorCaps->AGC             =  ISI_AGC_OFF;//  ( ISI_AGC_AUTO | ISI_AGC_OFF );
        pIsiSensorCaps->AWB             = ISI_AWB_OFF;// ( ISI_AWB_AUTO | ISI_AWB_OFF );
        pIsiSensorCaps->AEC             = (ISI_AEC_OFF);//  ISI_AEC_AUTO | ISI_AEC_OFF );
        pIsiSensorCaps->DPCC            = ( ISI_DPCC_AUTO | ISI_DPCC_OFF );

        pIsiSensorCaps->DwnSz           = ISI_DWNSZ_SUBSMPL;
        pIsiSensorCaps->CieProfile      = ( ISI_CIEPROF_A
                                          | ISI_CIEPROF_D50
                                          | ISI_CIEPROF_D65
                                          | ISI_CIEPROF_D75
                                          | ISI_CIEPROF_F2
                                          | ISI_CIEPROF_F11 );
        pIsiSensorCaps->SmiaMode        = ISI_SMIA_OFF;
        pIsiSensorCaps->MipiMode        = ISI_MIPI_OFF;//= ISI_MIPI_MODE_RAW_10;
	    pIsiSensorCaps->MipiLanes       = ISI_MIPI_4LANES;
        pIsiSensorCaps->AfpsResolutions = ISI_AFPS_NOTSUPP;//(ISI_RES_TV1080P15); //ISI_AFPS_NOTSUPP; //( ISI_RES_TV1080P24 | ISI_RES_TV1080P20 | ISI_RES_TV1080P15  | ISI_RES_TV1080P6);
        pIsiSensorCaps->enableHdr       = pIMX290Ctx->enableHdr;
    }

    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}


const IsiSensorCaps_t IMX290_g_IsiSensorDefaultConfig = {
    ISI_BUSWIDTH_12BIT,      // BusWidth
    ISI_MODE_BAYER,             // Mode
    ISI_FIELDSEL_BOTH,          // FieldSel
    ISI_YCSEQ_YCBYCR,//ISI_YCSEQ_YCRYCB,  //ISI_YCSEQ_YCBYCR,           // YCSeq
    ISI_CONV422_NOCOSITED,//ISI_CONV422_INTER, //ISI_CONV422_NOCOSITED,      // Conv422
    ISI_BPAT_GRGRBGBG,          // BPat
    ISI_HPOL_REFPOS,            // HPol
    ISI_VPOL_NEG,//ISI_VPOL_POS,      //ISI_VPOL_NEG,               // VPol
    ISI_EDGE_RISING,//ISI_EDGE_FALLING,  //ISI_EDGE_RISING,            // Edge
    ISI_BLS_OFF,                // Bls
    ISI_GAMMA_OFF,              // Gamma
    ISI_CCONV_OFF,              // CConv
    ISI_RES_TV1080P,          //ISI_RES_TV1080P24,          // Res
    ISI_DWNSZ_SUBSMPL,          // DwnSz
    ISI_BLC_AUTO,               // BLC
    ISI_AGC_OFF,      //ISI_AGC_OFF,                // AGC
    ISI_AWB_OFF,   //ISI_AWB_OFF,                // AWB
    ISI_AEC_OFF,      //ISI_AEC_OFF,                // AEC
    ISI_DPCC_OFF,               // DPCC
    ISI_CIEPROF_F11,            // CieProfile, this is also used as start profile for AWB (if not altered by menu settings)
    ISI_SMIA_OFF,               // SmiaMode
    ISI_MIPI_OFF,      //ISI_MIPI_MODE_RAW_10,       // MipiMode
    ISI_MIPI_4LANES,
    ISI_AFPS_NOTSUPP,  //ISI_RES_TV1080P15, //   ISI_AFPS_NOTSUPP(ISI_AFPS_NOTSUPP | ISI_RES_TV1080P6)
    //( ISI_AFPS_NOTSUPP | ISI_RES_TV1080P24 | ISI_RES_TV1080P20 | ISI_RES_TV1080P15 ) // AfpsResolutions
    0,
};

static RESULT IMX290_AecSetModeParameters
(
    IMX290_Context_t        *pIMX290Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;
    TRACE( IMX290_INFO, "%s%s: (enter)\n", __func__, pIMX290Ctx->isAfpsRun?"(AFPS)":"" );


    pIMX290Ctx->AecIntegrationTimeIncrement = pIMX290Ctx->one_line_exp_time;
    pIMX290Ctx->AecMinIntegrationTime = pIMX290Ctx->one_line_exp_time * pIMX290Ctx->MinIntegrationLine;
    pIMX290Ctx->AecMaxIntegrationTime = pIMX290Ctx->one_line_exp_time * pIMX290Ctx->MaxIntegrationLine;

    TRACE( IMX290_DEBUG, "%s%s: AecMaxIntegrationTime = %f \n", __func__, pIMX290Ctx->isAfpsRun?"(AFPS)":"", pIMX290Ctx->AecMaxIntegrationTime );

    pIMX290Ctx->AecGainIncrement = IMX290_MIN_GAIN_STEP;

    //reflects the state of the sensor registers, must equal default settings
    pIMX290Ctx->AecCurGain            = pIMX290Ctx->AecMinGain;
    pIMX290Ctx->AecCurIntegrationTime = 0.0f;
    pIMX290Ctx->OldGain               = 0;
    pIMX290Ctx->OldIntegrationTime    = 0;

    TRACE( IMX290_INFO, "%s%s: (exit)\n", __func__, pIMX290Ctx->isAfpsRun?"(AFPS)":"" );

    return ( result );
}

static RESULT IMX290_IsiSetupSensorIss
(
    IsiSensorHandle_t       handle,
    const IsiSensorConfig_t *pConfig
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    uint32_t RegValue = 0U;

    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    if (!pIMX290Ctx) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if (!pConfig) {
        TRACE( IMX290_ERROR, "%s: Invalid configuration (NULL pointer detected)\n", __func__ );
        return ( RET_NULL_POINTER );
    }

    if ( pIMX290Ctx->Streaming != BOOL_FALSE ) {
        return RET_WRONG_STATE;
    }

    memcpy( &pIMX290Ctx->Config, pConfig, sizeof( IsiSensorConfig_t ) );

    /* 1.) SW reset of image sensor (via I2C register interface)  be careful, bits 6..0 are reserved, reset bit is not sticky */
    TRACE( IMX290_DEBUG, "%s: IMX290 System-Reset executed\n", __func__);
    osSleep( 100 );

    result = IMX290_AecSetModeParameters( pIMX290Ctx, pConfig );
    if ( result != RET_SUCCESS ) {
        TRACE( IMX290_ERROR, "%s: SetupOutputWindow failed.\n", __func__);
        return ( result );
    }

    pIMX290Ctx->Configured = BOOL_TRUE;
    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}

static RESULT IMX290_IsiGetSupportResolutionIss
(
    IsiSensorHandle_t   handle,
    IsiResolutionArry_t *pResolutinArry
)
{
    RESULT result = RET_SUCCESS;

    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    if (handle == NULL)
    {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    pResolutinArry->count = sizeof(SensorDrvSupportResoluton)/sizeof(uint32_t);
    memcpy(pResolutinArry->ResolutionArry, SensorDrvSupportResoluton, sizeof(SensorDrvSupportResoluton));
    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

static RESULT IMX290_IsiChangeSensorResolutionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            Resolution,
    uint8_t             *pNumberOfFramesToSkip

)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    float hdr_ratio[2]  = {1.0f, 1.0f};

    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    if ( !pIMX290Ctx )
        return ( RET_WRONG_HANDLE );

    if (!pNumberOfFramesToSkip)
        return ( RET_NULL_POINTER );

    if ( (pIMX290Ctx->Configured != BOOL_TRUE) || (pIMX290Ctx->Streaming != BOOL_FALSE) )
        return RET_WRONG_STATE;

    IsiSensorCaps_t Caps;
    result = IMX290_IsiGetCapsIss( handle, &Caps);
    if (RET_SUCCESS != result)
        return result;

    if ( (Resolution & Caps.Resolution) == 0 )
        return RET_OUTOFRANGE;

    if ( Resolution == pIMX290Ctx->Config.Resolution ) {
        // well, no need to worry
        *pNumberOfFramesToSkip = 0;
    } else {
        // change resolution
        char *szResName = NULL;
        result = IsiGetResolutionName( Resolution, &szResName );
        TRACE( IMX290_DEBUG, "%s: NewRes=0x%08x (%s)\n", __func__, Resolution, szResName);

        // update resolution in copy of config in context
        pIMX290Ctx->Config.Resolution = Resolution;

        // tell sensor about that

        // remember old exposure values
        float OldGain = pIMX290Ctx->AecCurGain;
        float OldIntegrationTime = pIMX290Ctx->AecCurIntegrationTime;

        // update limits & stuff (reset current & old settings)
        result = IMX290_AecSetModeParameters( pIMX290Ctx, &pIMX290Ctx->Config );
        if ( result != RET_SUCCESS ) {
            TRACE( IMX290_ERROR, "%s: AecSetModeParameters failed.\n", __func__);
            return ( result );
        }

        // restore old exposure values (at least within new exposure values' limits)
        uint8_t NumberOfFramesToSkip;
        float DummySetGain;
        float DummySetIntegrationTime;
        result = IMX290_IsiExposureControlIss( handle, OldGain, OldIntegrationTime, &NumberOfFramesToSkip, &DummySetGain, &DummySetIntegrationTime, hdr_ratio );
        if ( result != RET_SUCCESS ) {
            TRACE( IMX290_ERROR, "%s: IMX290_IsiExposureControlIss failed.\n", __func__);
            return ( result );
        }

        // return number of frames that aren't exposed correctly
        *pNumberOfFramesToSkip = NumberOfFramesToSkip + 1;
    }

    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}

static RESULT IMX290_IsiSensorSetStreamingIss
(
    IsiSensorHandle_t   handle,
    bool_t              on
)
{
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    int32_t set_val =1;
    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;


    if ( (pIMX290Ctx->Configured != BOOL_TRUE) || (pIMX290Ctx->Streaming == on) )
        return RET_WRONG_STATE;

    int32_t enable = (uint32_t)on;
    if (pIMX290Ctx->KernelDriverFlag)
    {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_STREAM, &enable);

    }else
    {
        if(on){
	  set_val = 0x00;
	}else{
	  set_val = 0x01;
	}
        ret = IMX290_IsiRegisterWriteIss(handle, 0x3000, set_val);
        ret = IMX290_IsiRegisterWriteIss(handle, 0x3002, set_val);
    }

    if (ret != 0)
    {
        return ( RET_FAILURE );
    }

    pIMX290Ctx->Streaming = on;

    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}

static RESULT IMX290_IsiCheckSensorConnectionIss
(
    IsiSensorHandle_t   handle
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    uint32_t sensor_id;

    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

     if (pIMX290Ctx->KernelDriverFlag)
     {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_RESERVE_ID, &SensorRevChipID);
        if (ret != 0)
        {
            TRACE( IMX290_ERROR, "%s: Read Sensor reserve ID Error! \n", __func__);
            return ( RET_FAILURE );
        }
     }

    result = IMX290_IsiGetSensorRevisionIss(handle, &sensor_id);
    if (result != RET_SUCCESS)
    {
        TRACE( IMX290_ERROR, "%s: Read Sensor ID Error! \n", __func__);
	    return ( RET_FAILURE );
    }


    if (SensorRevChipID != sensor_id)
    {
        TRACE( IMX290_ERROR, "%s:ChipID =0x%x sensor_id=%x error! \n", __func__, SensorRevChipID, sensor_id);
	    return ( RET_FAILURE );
    }

    TRACE( IMX290_INFO, "%s ChipID = 0x%08x, sensor_id = 0x%08x, success! \n", __func__, SensorRevChipID, sensor_id);
    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}

static RESULT IMX290_IsiGetSensorRevisionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *p_value
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;
    uint32_t reg_val;
    uint32_t sensor_id;

    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    if ( !p_value )
        return ( RET_NULL_POINTER );

    if (pIMX290Ctx->KernelDriverFlag)
    {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_CHIP_ID, &sensor_id);
        if (ret != 0)
        {
            TRACE( IMX290_ERROR, "%s: Read Sensor ID Error! \n", __func__);
            return ( RET_FAILURE );
        }
    }
    else
    {
        reg_val = 0;
        result |= IMX290_IsiRegisterReadIss(handle, 0x31DC, &reg_val);
        if((reg_val & 0xf8)== reg_val)
        sensor_id = 0x290;

    }

    *p_value = sensor_id;
    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}

static RESULT IMX290_IsiGetGainLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinGain,
    float               *pMaxGain
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    uint32_t RegValue = 0U;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( pIMX290Ctx == NULL ) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinGain == NULL) || (pMaxGain == NULL) ) {
        TRACE( IMX290_ERROR, "%s: NULL pointer received!!\n" );
        return ( RET_NULL_POINTER );
    }

    *pMinGain = pIMX290Ctx->AecMinGain;
    *pMaxGain = pIMX290Ctx->AecMaxGain;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);
    return ( result );
}

static RESULT IMX290_IsiGetIntegrationTimeLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinIntegrationTime,
    float               *pMaxIntegrationTime
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    uint32_t RegValue = 0U;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);
    if ( pIMX290Ctx == NULL ) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinIntegrationTime == NULL) || (pMaxIntegrationTime == NULL) ) {
        TRACE( IMX290_ERROR, "%s: NULL pointer received!!\n" );
        return ( RET_NULL_POINTER );
    }

    *pMinIntegrationTime = pIMX290Ctx->AecMinIntegrationTime;
    *pMaxIntegrationTime = pIMX290Ctx->AecMaxIntegrationTime;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);
    return ( result );
}

RESULT IMX290_IsiGetGainIss
(
    IsiSensorHandle_t   handle,
    float               *pSetGain
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( pIMX290Ctx == NULL ) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetGain == NULL) {
        return ( RET_NULL_POINTER );
    }

    *pSetGain = pIMX290Ctx->AecCurGain;

    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}


RESULT IMX290_IsiGetVSGainIss
(
    IsiSensorHandle_t   handle,
    float               *pSetGain
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( pIMX290Ctx == NULL ) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetGain == NULL) {
        return ( RET_NULL_POINTER );
    }

    *pSetGain = pIMX290Ctx->AecCurVSGain;

    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

RESULT IMX290_IsiGetGainIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( pIMX290Ctx == NULL ) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pIncr == NULL)
        return ( RET_NULL_POINTER );

    *pIncr = pIMX290Ctx->AecGainIncrement;

    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

RESULT IMX290_IsiSetGainIss
(
    IsiSensorHandle_t   handle,
    float               NewGain,
    float               *pSetGain,
    float               *hdr_ratio
)
{

    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    float  gain = 0.0;
    int32_t set_val = 0.0;

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    if( NewGain < pIMX290Ctx->AecMinGain ) NewGain = pIMX290Ctx->AecMinGain;
    if( NewGain > pIMX290Ctx->AecMaxGain ) NewGain = pIMX290Ctx->AecMaxGain;
    gain = 20*log10(NewGain);
    set_val =(int32_t) ((gain/3)*10);
    if (pIMX290Ctx->KernelDriverFlag)
    {
        uint32_t SensorGain = 0;
        SensorGain = NewGain * pIMX290Ctx->gain_accuracy;
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_GAIN, &set_val);
    }
    else
    {
        result = IMX290_IsiRegisterWriteIss(handle, 0x3014, (set_val&0xFF));

    }

    pIMX290Ctx->AecCurGain = ((float)(NewGain));

    *pSetGain = pIMX290Ctx->AecCurGain;
    TRACE( IMX290_DEBUG, "%s: g=%f\n", __func__, *pSetGain );
    return ( result );
}

RESULT IMX290_IsiSetVSGainIss
(
    IsiSensorHandle_t   handle,
    float NewIntegrationTime,
    float NewGain,
    float *pSetGain,
    float *hdr_ratio
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;
    RESULT result = RET_SUCCESS;

    float Gain = 0.0f;
    float ratio = 0.25f;
    uint32_t ucGain = 0U;
    uint32_t again = 0U;
    uint32_t data = 0;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if (!pIMX290Ctx) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if (!pSetGain || !hdr_ratio)
        return ( RET_NULL_POINTER );

    NewGain = MIN(pIMX290Ctx->AecMaxGain, MAX(pIMX290Ctx->AecMinGain, NewGain));

    if ((NewIntegrationTime) > IMX290_VS_MAX_INTEGRATION_TIME*hdr_ratio[0]) {
        NewGain = (NewIntegrationTime * NewGain) / (IMX290_VS_MAX_INTEGRATION_TIME*hdr_ratio[0]);
    }

	pIMX290Ctx->OldVsGain = ucGain;

    pIMX290Ctx->AecCurVSGain = NewGain;
    *pSetGain = pIMX290Ctx->AecCurGain;
    TRACE( IMX290_DEBUG, "%s: g=%f\n", __func__, *pSetGain );
    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}

RESULT IMX290_IsiSetBayerPattern(IsiSensorHandle_t handle, uint8_t pattern)
{

    RESULT result = RET_SUCCESS;
    uint8_t h_shift = 0,v_shift = 0;
    uint8_t val_h = 0,val_l = 0;
    uint16_t val = 0;
    uint8_t Start_p = 0;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    // pattern 0:B 1:GB 2:GR 3:R
    result = IMX290_IsiSensorSetStreamingIss(handle, 0);
    switch(pattern)
    {
        case BAYER_BGGR:
             Start_p = 0;
            break;
        case BAYER_GBRG:
             Start_p = 1;
            break;
        case BAYER_GRBG:
             Start_p = 2;
            break;
        case BAYER_RGGB:
             Start_p = 3;
            break;
    }


        h_shift = Start_p % 2;
        v_shift = Start_p / 2;

        IMX290_IsiRegisterReadIss(handle, 0x3040, &val_l);
        IMX290_IsiRegisterReadIss(handle, 0x3041, &val_h);

	val = ((val_l & 0x0006) | ((val_h<<8) & 0xff00)) + h_shift;
        val_h = (val >> 8) & 0x06;
        val_l = val & 0xff;
        IMX290_IsiRegisterWriteIss(handle, 0x3040, val_l);
        IMX290_IsiRegisterWriteIss(handle, 0x3041, val_h);

        IMX290_IsiRegisterReadIss(handle, 0x303c, &val_l);
        IMX290_IsiRegisterReadIss(handle, 0x303d, &val_h);

	val = (((val_h << 8) & 0xff00) | (val_l & 0x0006)) + v_shift;
        val_h = (val >> 8) & 0x06;
        val_l = val & 0xff;
        IMX290_IsiRegisterWriteIss(handle, 0x303c, val_l);
        IMX290_IsiRegisterWriteIss(handle, 0x303d, val_h);

        IMX290_IsiRegisterWriteIss(handle, 0x3042, 0x80);
        IMX290_IsiRegisterWriteIss(handle, 0x3043, 0x07);

        IMX290_IsiRegisterWriteIss(handle, 0x303E, 0x38);
        IMX290_IsiRegisterWriteIss(handle, 0x303F, 0x04);


    pIMX290Ctx->pattern = pattern;
    result = IMX290_IsiSensorSetStreamingIss(handle, pIMX290Ctx->Streaming);

	return ( result );
}

RESULT IMX290_IsiGetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               *pSetIntegrationTime
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if (!pIMX290Ctx) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if (!pSetIntegrationTime)
        return ( RET_NULL_POINTER );

    *pSetIntegrationTime = pIMX290Ctx->AecCurIntegrationTime;
    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}


RESULT IMX290_IsiGetVSIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               *pSetIntegrationTime
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if (!pIMX290Ctx) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }
    if (!pSetIntegrationTime)
        return ( RET_NULL_POINTER );

    *pSetIntegrationTime = pIMX290Ctx->AecCurVSIntegrationTime;
    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}

RESULT IMX290_IsiGetIntegrationTimeIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if (!pIMX290Ctx) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if (!pIncr)
        return ( RET_NULL_POINTER );

    //_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
    *pIncr = pIMX290Ctx->AecIntegrationTimeIncrement;
    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}

RESULT IMX290_IsiSetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               NewIntegrationTime,
    float               *pSetIntegrationTime,
    uint8_t             *pNumberOfFramesToSkip,
    float               *hdr_ratio
)
{
    RESULT result = RET_SUCCESS;
    int32_t ret;
    int32_t set_val = 0;

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    uint32_t exp_line = 0;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( !pIMX290Ctx)
    {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( !pSetIntegrationTime || !pNumberOfFramesToSkip)
    {
        TRACE( IMX290_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __func__ );
        return ( RET_NULL_POINTER );
    }

    exp_line = NewIntegrationTime/pIMX290Ctx->one_line_exp_time;
    exp_line = MIN(pIMX290Ctx->MaxIntegrationLine, MAX(pIMX290Ctx->MinIntegrationLine, exp_line));

    TRACE( IMX290_DEBUG, "%s: set AEC_PK_EXPO=0x%05x\n", __func__, exp_line );

    if ( exp_line != pIMX290Ctx->OldIntegrationTime )
    {
      if (pIMX290Ctx->KernelDriverFlag)
       {
           ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_EXP, &exp_line);
       }else
       {
	  set_val = IMX290_VTS_NUM - exp_line -1;
          ret = IMX290_IsiRegisterWriteIss(handle, 0x3020, (set_val&0x000FF));
          ret = IMX290_IsiRegisterWriteIss(handle, 0x3021, ((set_val>>8)&0x000FF));


	}

        pIMX290Ctx->OldIntegrationTime = exp_line; // remember current integration time
        pIMX290Ctx->AecCurIntegrationTime = exp_line * pIMX290Ctx->one_line_exp_time;

        *pNumberOfFramesToSkip = 1U; //skip 1 frame
    } else
    {
        *pNumberOfFramesToSkip = 0U; //no frame skip
    }


    *pSetIntegrationTime = pIMX290Ctx->AecCurIntegrationTime;

    TRACE( IMX290_DEBUG, "%s: Ti=%f\n", __func__, *pSetIntegrationTime );
    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}

RESULT IMX290_IsiSetVSIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float NewIntegrationTime,
    float *pSetVSIntegrationTime,
    uint8_t *pNumberOfFramesToSkip,
    float *hdr_ratio
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;
    RESULT result = RET_SUCCESS;
    uint32_t exp_line = 0;
    int32_t ret;


    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( !pIMX290Ctx) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if (!pSetVSIntegrationTime || !pNumberOfFramesToSkip) {
        TRACE( IMX290_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __func__ );
        return ( RET_NULL_POINTER );
    }

    NewIntegrationTime = MIN(pIMX290Ctx->AecMaxIntegrationTime, MAX(pIMX290Ctx->AecMinIntegrationTime, NewIntegrationTime));
    TRACE( IMX290_INFO, "%s:  maxIntegrationTime-=%f minIntegrationTime = %f\n", __func__, pIMX290Ctx->AecMaxIntegrationTime,pIMX290Ctx->AecMinIntegrationTime);

    if (NewIntegrationTime > IMX290_VS_MAX_INTEGRATION_TIME*hdr_ratio[0]) {
        NewIntegrationTime = IMX290_VS_MAX_INTEGRATION_TIME;
    } else {
        NewIntegrationTime = NewIntegrationTime/hdr_ratio[0];
        if(NewIntegrationTime < 0.0002)  NewIntegrationTime = 0.0002;
     }

    exp_line = NewIntegrationTime/pIMX290Ctx->one_line_exp_time;
    exp_line = MIN(pIMX290Ctx->MaxIntegrationLine, MAX(pIMX290Ctx->MinIntegrationLine, exp_line));

    if (exp_line != pIMX290Ctx->OldVsIntegrationTime)
     {
         if (pIMX290Ctx->KernelDriverFlag)
         {
            ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_VSEXP, &exp_line);
         }else
         {
            result = IMX290_IsiRegisterWriteIss(handle, 0x3467, 0x00);
            result = IMX290_IsiRegisterWriteIss(handle, 0x3464, 0x04);

            result = IMX290_IsiRegisterWriteIss(handle, 0x30b8, (exp_line >> 8) & 0xff);
            result = IMX290_IsiRegisterWriteIss(handle, 0x30b9, exp_line & 0xff);

            result = IMX290_IsiRegisterWriteIss(handle, 0x3467, 0x14);
            result = IMX290_IsiRegisterWriteIss(handle, 0x3464, 0x01);
         }

        pIMX290Ctx->OldVsIntegrationTime = exp_line;
        pIMX290Ctx->AecCurVSIntegrationTime = exp_line * pIMX290Ctx->one_line_exp_time ;   //remember current integration time
        *pNumberOfFramesToSkip = 1U; //skip 1 frame
    } else {
        *pNumberOfFramesToSkip = 0U; //no frame skip
    }

    *pSetVSIntegrationTime = pIMX290Ctx->AecCurVSIntegrationTime;

    TRACE( IMX290_DEBUG, "%s: NewIntegrationTime=%f\n", __func__, NewIntegrationTime );
    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}

RESULT IMX290_IsiExposureControlIss
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
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    float IntegrationTime;
    RESULT result = RET_SUCCESS;
    const float Gain = NewGain;
    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( pIMX290Ctx == NULL ) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pNumberOfFramesToSkip == NULL) || (pSetGain == NULL) || (pSetIntegrationTime == NULL) ) {
        TRACE( IMX290_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __func__ );
        return ( RET_NULL_POINTER );
    }

    TRACE( IMX290_DEBUG, "%s: g=%f, Ti=%f\n", __func__, NewGain, NewIntegrationTime );
  if (pIMX290Ctx->enableHdr == true)
   {
    result = IMX290_IsiSetVSIntegrationTimeIss( handle, NewIntegrationTime, pSetIntegrationTime, pNumberOfFramesToSkip, hdr_ratio );
    result = IMX290_IsiSetVSGainIss( handle, NewIntegrationTime,NewGain, pSetGain, hdr_ratio);
   }
    result = IMX290_IsiSetIntegrationTimeIss( handle, NewIntegrationTime, pSetIntegrationTime, pNumberOfFramesToSkip, hdr_ratio );
    TRACE( IMX290_DEBUG, "%s: set: NewGain=%f, hcgTi=%f, hcgskip=%d\n", __func__, Gain,NewIntegrationTime, *pNumberOfFramesToSkip );
    result = IMX290_IsiSetGainIss( handle, Gain, pSetGain, hdr_ratio);
    TRACE( IMX290_DEBUG, "%s: set: NewGain=%f, hcgTi=%f, hcgskip=%d\n", __func__, NewGain,NewIntegrationTime, *pNumberOfFramesToSkip );
    TRACE( IMX290_DEBUG, "%s: set: vsg=%f, vsTi=%f, vsskip=%d\n", __func__, NewGain,NewIntegrationTime, *pNumberOfFramesToSkip );
    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);

    return ( 0 );
}

RESULT IMX290_IsiGetCurrentExposureIss
(
    IsiSensorHandle_t   handle,
    float               *pSetGain,
    float               *pSetIntegrationTime
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    uint32_t RegValue = 0U;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( pIMX290Ctx == NULL ) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pSetGain == NULL) || (pSetIntegrationTime == NULL) )
        return ( RET_NULL_POINTER );

    *pSetGain            = pIMX290Ctx->AecCurGain;
    *pSetIntegrationTime = pIMX290Ctx->AecCurIntegrationTime;

    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}

RESULT IMX290_IsiGetResolutionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *pSetResolution
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( pIMX290Ctx == NULL ) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetResolution == NULL )
        return ( RET_NULL_POINTER );

    *pSetResolution = pIMX290Ctx->Config.Resolution;
    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}

RESULT IMX290_IsiGetSensorFpsIss(
    IsiSensorHandle_t   handle,
    uint32_t            *pfps
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    uint32_t ret;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( pIMX290Ctx == NULL )
    {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    if (pIMX290Ctx->KernelDriverFlag)
    {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_FPS, pfps);
    }

    *pfps = pIMX290Ctx->CurrFps;

    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}

RESULT IMX290_IsiSetSensorFpsIss(
    IsiSensorHandle_t   handle,
    uint32_t            fps
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( pIMX290Ctx == NULL )
    {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    if (fps > pIMX290Ctx->MaxFps)
    {
        TRACE( IMX290_ERROR, "%s: set fps(%d) out of range, correct to %d (%d, %d)\n", __func__, fps, pIMX290Ctx->MaxFps, pIMX290Ctx->MinFps, pIMX290Ctx->MaxFps);
        fps = pIMX290Ctx->MaxFps;
    }
    if (fps < pIMX290Ctx->MinFps)
    {
        TRACE( IMX290_ERROR, "%s: set fps(%d) out of range, correct to %d (%d, %d)\n", __func__, fps, pIMX290Ctx->MinFps, pIMX290Ctx->MinFps, pIMX290Ctx->MaxFps);
        fps = pIMX290Ctx->MinFps;
    }
    /*if (pIMX290Ctx->KernelDriverFlag)
    {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_FPS, &fps);
    }else
    {
        uint16_t  FrameLengthLines;
        FrameLengthLines = pIMX290Ctx->FrameLengthLines * pIMX290Ctx->MaxFps / fps;
        result = IMX290_IsiRegisterWriteIss(handle,0x30b2,(FrameLengthLines>>8) & 0xff);
        result |= IMX290_IsiRegisterWriteIss(handle,0x30b3,FrameLengthLines & 0xff);
        if (result != RET_SUCCESS)
        {
            TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
            return ( RET_FAILURE );
        }
        pIMX290Ctx->CurrFps = fps;
        pIMX290Ctx->CurFrameLengthLines = FrameLengthLines;
        pIMX290Ctx->MaxIntegrationLine  = pIMX290Ctx->CurFrameLengthLines - 3;
        pIMX290Ctx->AecMaxIntegrationTime = pIMX290Ctx->MaxIntegrationLine * pIMX290Ctx->one_line_exp_time;
    }*/

    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}

RESULT IMX290_IsiGetAfpsInfoIss(
    IsiSensorHandle_t   handle,
    uint32_t            Resolution,
    IsiAfpsInfo_t*      pAfpsInfo
)
{
    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    RESULT result = RET_SUCCESS;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    if ( pIMX290Ctx == NULL ) {
        TRACE( IMX290_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pAfpsInfo == NULL )
        return ( RET_NULL_POINTER );

    // set current resolution data in info struct
    pAfpsInfo->AecMaxGain           = pIMX290Ctx->AecMaxGain;
    pAfpsInfo->AecMinGain           = pIMX290Ctx->AecMinGain;
    pAfpsInfo->AecMaxIntTime        = pIMX290Ctx->AecMinIntegrationTime;
    pAfpsInfo->AecMaxIntTime        = pIMX290Ctx->AecMaxIntegrationTime;
    pAfpsInfo->AecSlowestResolution = pIMX290Ctx->Resolution;
    pAfpsInfo->CurrResolution       = pIMX290Ctx->Resolution;
    pAfpsInfo->CurrMaxIntTime       = pIMX290Ctx->AecMaxIntegrationTime;
    pAfpsInfo->CurrMinIntTime       = pIMX290Ctx->AecMinIntegrationTime;
    pAfpsInfo->Stage[0].MaxIntTime  = pIMX290Ctx->AecMaxIntegrationTime;
    pAfpsInfo->Stage[0].Resolution  = pIMX290Ctx->Resolution;

    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return ( result );
}



static RESULT IMX290_IsiActivateTestPattern(IsiSensorHandle_t handle, const bool_t enable)
{
    RESULT result = RET_SUCCESS;

    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;


    if (pIMX290Ctx->Configured != BOOL_TRUE)
        return RET_WRONG_STATE;

/*****************************
    if ( BOOL_TRUE == enable )
    {
        result = IMX290_IsiRegisterWriteIss(handle, 0x3253, 0x80);
    }
     else
    {
        result = IMX290_IsiRegisterWriteIss(handle, 0x3253, 0x00);
    }
******************************/
    pIMX290Ctx->TestPattern = enable;

    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);

    return ( result );
}

static RESULT IMX290_IsiEnableHdr(IsiSensorHandle_t handle, const bool_t enable)
{
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    TRACE( IMX290_INFO, "%s: (enter)\n", __func__);

    IMX290_Context_t *pIMX290Ctx = (IMX290_Context_t *)handle;
    if (pIMX290Ctx == NULL || pIMX290Ctx->IsiCtx.HalHandle == NULL)
    {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *)pIMX290Ctx->IsiCtx.HalHandle;

    result = IMX290_IsiSensorSetStreamingIss(handle, 0);
    if (pIMX290Ctx->KernelDriverFlag)
    {
        uint32_t hdr_mode;
        if (enable == 0)
        {
            hdr_mode = SENSOR_MODE_LINEAR;
        }else
        {
            hdr_mode = SENSOR_HDR_MODE_3DOL;
        }
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_HDR_MODE, &hdr_mode);
    }else
    {
   //     result |= IMX290_IsiRegisterWriteIss(handle, 0x3190, enable ? 0x05 : 0x08);
    }

    result |= IMX290_IsiSensorSetStreamingIss(handle,  pIMX290Ctx->Streaming);

    if (result != 0 || ret != 0)
    {
        TRACE( IMX290_ERROR, "%s: change hdr status error\n", __func__);
         return RET_FAILURE;
    }

    pIMX290Ctx->enableHdr = enable;
    TRACE( IMX290_INFO, "%s: (exit)\n", __func__);
    return RET_SUCCESS;
}

RESULT IMX290_IsiGetSensorIss(IsiSensor_t *pIsiSensor)
{
    RESULT result = RET_SUCCESS;
    TRACE( IMX290_INFO, "%s (enter)\n", __func__);

    if ( pIsiSensor != NULL ) {
        pIsiSensor->pszName                         = SensorName;

        pIsiSensor->pIsiSensorCaps                  = &IMX290_g_IsiSensorDefaultConfig;

        pIsiSensor->pIsiCreateSensorIss             = IMX290_IsiCreateSensorIss;

        pIsiSensor->pIsiInitSensorIss               = IMX290_IsiInitSensorIss;
	    pIsiSensor->pIsiResetSensorIss              = IMX290_IsiResetSensorIss;
        pIsiSensor->pIsiReleaseSensorIss            = IMX290_IsiReleaseSensorIss;
        pIsiSensor->pIsiGetCapsIss                  = IMX290_IsiGetCapsIss;
        pIsiSensor->pIsiSetupSensorIss              = IMX290_IsiSetupSensorIss;
        pIsiSensor->pIsiChangeSensorResolutionIss   = IMX290_IsiChangeSensorResolutionIss;
        pIsiSensor->pIsiSensorSetStreamingIss       = IMX290_IsiSensorSetStreamingIss;
        pIsiSensor->pIsiSensorSetPowerIss           = IMX290_IsiSensorSetPowerIss;
        pIsiSensor->pIsiCheckSensorConnectionIss    = IMX290_IsiCheckSensorConnectionIss;
        pIsiSensor->pIsiGetSensorRevisionIss        = IMX290_IsiGetSensorRevisionIss;
        pIsiSensor->pIsiRegisterReadIss             = IMX290_IsiRegisterReadIss;
        pIsiSensor->pIsiRegisterWriteIss            = IMX290_IsiRegisterWriteIss;

        /* AEC functions */
        pIsiSensor->pIsiExposureControlIss          = IMX290_IsiExposureControlIss;
        pIsiSensor->pIsiGetGainLimitsIss            = IMX290_IsiGetGainLimitsIss;
        pIsiSensor->pIsiGetIntegrationTimeLimitsIss = IMX290_IsiGetIntegrationTimeLimitsIss;
        pIsiSensor->pIsiGetCurrentExposureIss       = IMX290_IsiGetCurrentExposureIss;
        pIsiSensor->pIsiGetVSGainIss                    = IMX290_IsiGetVSGainIss;
        pIsiSensor->pIsiGetGainIss                      = IMX290_IsiGetGainIss;
        pIsiSensor->pIsiGetGainIncrementIss             = IMX290_IsiGetGainIncrementIss;
        pIsiSensor->pIsiSetGainIss                      = IMX290_IsiSetGainIss;
        pIsiSensor->pIsiGetIntegrationTimeIss           = IMX290_IsiGetIntegrationTimeIss;
        pIsiSensor->pIsiGetVSIntegrationTimeIss         = IMX290_IsiGetVSIntegrationTimeIss;
        pIsiSensor->pIsiGetIntegrationTimeIncrementIss  = IMX290_IsiGetIntegrationTimeIncrementIss;
        pIsiSensor->pIsiSetIntegrationTimeIss           = IMX290_IsiSetIntegrationTimeIss;
        pIsiSensor->pIsiGetSupportResolutionIss         = IMX290_IsiGetSupportResolutionIss;
        pIsiSensor->pIsiGetResolutionIss                = IMX290_IsiGetResolutionIss;
        pIsiSensor->pIsiGetSensorFpsIss                 = IMX290_IsiGetSensorFpsIss;
        pIsiSensor->pIsiSetSensorFpsIss                 = IMX290_IsiSetSensorFpsIss;
        pIsiSensor->pIsiGetAfpsInfoIss                  = IMX290_IsiGetAfpsInfoIss;

        /* AWB specific functions */

        /* Testpattern */
        pIsiSensor->pIsiActivateTestPattern         = IMX290_IsiActivateTestPattern;
        pIsiSensor->pIsiEnableHdr                   = IMX290_IsiEnableHdr;
        pIsiSensor->pIsiSetBayerPattern             = IMX290_IsiSetBayerPattern;
    } else {
        result = RET_NULL_POINTER;
    }

    TRACE( IMX290_INFO, "%s (exit)\n", __func__);
    return ( result );
}

/*****************************************************************************
* each sensor driver need declare this struct for isi load
*****************************************************************************/
IsiCamDrvConfig_t IsiCamDrvConfig =
{
    0,
    IMX290_IsiGetSensorIss,
    {
        0,                      /**< IsiSensor_t.pszName */
	    0,                      /**< IsiSensor_t.pIsiInitIss>*/
	    0,			            /**< IsiSensor_t.pIsiResetSensorIss>*/
        0,                      /**< IsiSensor_t.pRegisterTable */
        0,                      /**< IsiSensor_t.pIsiSensorCaps */
        0,                      /**< IsiSensor_t.pIsiCreateSensorIss */
        0,                      /**< IsiSensor_t.pIsiReleaseSensorIss */
        0,                      /**< IsiSensor_t.pIsiGetCapsIss */
        0,                      /**< IsiSensor_t.pIsiSetupSensorIss */
        0,                      /**< IsiSensor_t.pIsiChangeSensorResolutionIss */
        0,                      /**< IsiSensor_t.pIsiSensorSetStreamingIss */
        0,                      /**< IsiSensor_t.pIsiSensorSetPowerIss */
        0,                      /**< IsiSensor_t.pIsiCheckSensorConnectionIss */
        0,                      /**< IsiSensor_t.pIsiGetSensorRevisionIss */
        0,                      /**< IsiSensor_t.pIsiRegisterReadIss */
        0,                      /**< IsiSensor_t.pIsiRegisterWriteIss */

        0,                      /**< IsiSensor_t.pIsiExposureControlIss */
        0,                      /**< IsiSensor_t.pIsiGetGainLimitsIss */
        0,                      /**< IsiSensor_t.pIsiGetIntegrationTimeLimitsIss */
        0,                      /**< IsiSensor_t.pIsiGetCurrentExposureIss */
        0,                      /**< IsiSensor_t.pIsiGetGainIss */
        0,                      /**< IsiSensor_t.pIsiGetVSGainIss */
        0,                      /**< IsiSensor_t.pIsiGetGainIncrementIss */
        0,                      /**< IsiSensor_t.pIsiGetGainIncrementIss */
        0,                      /**< IsiSensor_t.pIsiSetGainIss */
        0,                      /**< IsiSensor_t.pIsiGetIntegrationTimeIss */
        0,                      /**< IsiSensor_t.pIsiGetIntegrationTimeIncrementIss */
        0,                      /**< IsiSensor_t.pIsiSetIntegrationTimeIss */
        0,                      /**< IsiSensor_t.pIsiGetResolutionIss */
        0,                      /**< IsiSensor_t.pIsiGetAfpsInfoIss */


        0,                      /**< IsiSensor_t.pIsiMdiInitMotoDriveMds */
        0,                      /**< IsiSensor_t.pIsiMdiSetupMotoDrive */
        0,                      /**< IsiSensor_t.pIsiMdiFocusSet */
        0,                      /**< IsiSensor_t.pIsiMdiFocusGet */
        0,                      /**< IsiSensor_t.pIsiMdiFocusCalibrate */
        0,                      /**< IsiSensor_t.pIsiGetSensorMipiInfoIss */
        0,                      /**< IsiSensor_t.pIsiActivateTestPattern */
        0,                      /**< IsiSensor_t.pIsiEnableHdr */
        0,                      /**< IsiSensor_t.pIsiSetBayerPattern */
    }
};
