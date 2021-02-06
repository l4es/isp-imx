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
#include <sys/ioctl.h>
#include <fcntl.h>
#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "vvsensor.h"

#include "OV2775_priv.h"

CREATE_TRACER( OV2775_INFO , "OV2775: ", INFO,    1);
CREATE_TRACER( OV2775_WARN , "OV2775: ", WARNING, 1);
CREATE_TRACER( OV2775_ERROR, "OV2775: ", ERROR,   1);
CREATE_TRACER( OV2775_DEBUG,     "OV2775: ", INFO, 1);
CREATE_TRACER( OV2775_REG_INFO , "OV2775: ", INFO, 1);
CREATE_TRACER( OV2775_REG_DEBUG, "OV2775: ", INFO, 1);

#ifdef SUBDEV_V4L2
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#undef TRACE
#define TRACE(x, ...)
#endif

#define OV2775_MIN_GAIN_STEP    ( 1.0f/16.0f )  /**< min gain step size used by GUI (hardware min = 1/16; 1/16..32/16 depending on actual gain ) */
#define OV2775_MAX_GAIN_AEC     ( 32.0f )       /**< max. gain used by the AEC (arbitrarily chosen, hardware limit = 62.0, driver limit = 32.0 ) */
#define OV2775_VS_MAX_INTEGRATION_TIME (0.0018)

/*****************************************************************************
 *Sensor Info
*****************************************************************************/
static const char SensorName[16] = "OV2775";

static struct vvcam_mode_info pov2775_mode_info[] = {
	{
		.index     = 0,
		.width     = 1920,
		.height    = 1080,
		.fps       = 30,
		.hdr_mode  = SENSOR_MODE_LINEAR,
		.bit_width = 12,
		.bayer_pattern = BAYER_GBRG,
	},
	{
		.index     = 1,
		.width    = 1920,
		.height   = 1080,
		.fps      = 30,
		.hdr_mode = SENSOR_MODE_HDR_STITCH,
		.stitching_mode = SENSOR_STITCHING_3DOL,
		.bit_width = 12,
		.bayer_pattern = BAYER_GBRG,
	},
	{
		.index     = 2,
		.width    = 1280,
		.height   = 720,
		.fps      = 60,
		.hdr_mode = SENSOR_MODE_LINEAR,
		.bit_width = 12,
		.bayer_pattern = BAYER_GBRG,
	}
};

static RESULT OV2775_IsiSensorSetPowerIss(IsiSensorHandle_t handle, bool_t on) {
    RESULT result = RET_SUCCESS;

    int ret = 0;
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    int32_t enable = on;
    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_POWER, &enable);
    if (ret != 0) {
        TRACE(OV2775_ERROR, "%s: sensor set power error!\n", __func__);
        return (RET_FAILURE);
    }

    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiResetSensorIss(IsiSensorHandle_t handle) {
    RESULT result = RET_SUCCESS;
    int ret = 0;

    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_RESET, NULL);
    if (ret != 0) {
        TRACE(OV2775_ERROR, "%s: sensor reset error!\n", __func__);
        return (RET_FAILURE);
    }

    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

#ifdef SUBDEV_CHAR
static RESULT OV2775_IsiSensorSetClkIss(IsiSensorHandle_t handle, uint32_t clk) {
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_CLK, &clk);
    if (ret != 0) {
        TRACE(OV2775_ERROR, "%s: sensor set clk error!\n", __func__);
        return (RET_FAILURE);
    }

    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiSensorGetClkIss
    (IsiSensorHandle_t handle, uint32_t * pclk) {
    RESULT result = RET_SUCCESS;
    int ret = 0;

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_CLK, pclk);
    if (ret != 0) {
        TRACE(OV2775_ERROR, "%s: sensor get clk error!\n", __func__);
        return (RET_FAILURE);
    }

    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiConfigSensorSCCBIss(IsiSensorHandle_t handle)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;
    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    static const IsiSccbInfo_t SensorSccbInfo = {
        .slave_addr = (0x6c >> 1),
        .addr_byte = 2,
        .data_byte = 1,
    };

    struct vvcam_sccb_cfg_s sensor_sccb_config;
    sensor_sccb_config.slave_addr = SensorSccbInfo.slave_addr;
    sensor_sccb_config.addr_byte = SensorSccbInfo.addr_byte;
    sensor_sccb_config.data_byte = SensorSccbInfo.data_byte;

    ret =
        ioctl(pHalCtx->sensor_fd, VVSENSORIOC_SENSOR_SCCB_CFG,
          &sensor_sccb_config);
    if (ret != 0) {
        TRACE(OV2775_ERROR, "%s: sensor config sccb info error!\n",
              __func__);
        return (RET_FAILURE);
    }

    TRACE(OV2775_INFO, "%s (exit) result = %d\n", __func__, result);
    return (result);
}
#endif

static RESULT OV2775_IsiRegisterReadIss
    (IsiSensorHandle_t handle, const uint32_t address, uint32_t * p_value) {
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    struct vvcam_sccb_data sccb_data;
    sccb_data.addr = address;
    sccb_data.data = 0;
    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_READ_REG, &sccb_data);
    if (ret != 0) {
        TRACE(OV2775_ERROR, "%s: read sensor register error!\n",
              __func__);
        return (RET_FAILURE);
    }

    *p_value = sccb_data.data;

    TRACE(OV2775_INFO, "%s (exit) result = %d\n", __func__, result);
    return (result);
}

static RESULT OV2775_IsiRegisterWriteIss
    (IsiSensorHandle_t handle, const uint32_t address, const uint32_t value) {
    RESULT result = RET_SUCCESS;
    int ret = 0;
    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    struct vvcam_sccb_data sccb_data;
    sccb_data.addr = address;
    sccb_data.data = value;

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_WRITE_REG, &sccb_data);
    if (ret != 0) {
        TRACE(OV2775_ERROR, "%s: write sensor register error!\n",
              __func__);
        return (RET_FAILURE);
    }

    TRACE(OV2775_INFO, "%s (exit) result = %d\n", __func__, result);
    return (result);
}

static RESULT OV2775_IsiQuerySensorSupportIss(HalHandle_t  HalHandle, vvcam_mode_info_array_t *pSensorSupportInfo)
{
    int ret = 0;
    struct vvcam_mode_info_array *psensor_mode_info_arry;

    HalContext_t *pHalCtx = HalHandle;
    if ( pHalCtx == NULL ) {
        return RET_NULL_POINTER;
    }

    psensor_mode_info_arry = pSensorSupportInfo;
    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_QUERY, psensor_mode_info_arry);
    if (ret != 0) {
        TRACE(OV2775_ERROR, "%s: sensor kernel query error! Use isi query\n",__func__);
        psensor_mode_info_arry->count = sizeof(pov2775_mode_info) / sizeof(struct vvcam_mode_info);
        memcpy(psensor_mode_info_arry->modes, pov2775_mode_info, sizeof(pov2775_mode_info));
    }

    return RET_SUCCESS;
}

static  RESULT OV2775_IsiQuerySensorIss(IsiSensorHandle_t handle, vvcam_mode_info_array_t *pSensorInfo)
{
    RESULT result = RET_SUCCESS;
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;
    OV2775_IsiQuerySensorSupportIss(pHalCtx,pSensorInfo);

    return result;
}

static RESULT OV2775_IsiGetSensorModeIss(IsiSensorHandle_t handle,void *mode)
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL) {
        return (RET_WRONG_HANDLE);
    }
    memcpy(mode,&(pOV2775Ctx->SensorMode), sizeof(pOV2775Ctx->SensorMode));

    return ( RET_SUCCESS );
}

static RESULT OV2775_IsiCreateSensorIss(IsiSensorInstanceConfig_t * pConfig) {
    RESULT result = RET_SUCCESS;
    OV2775_Context_t *pOV2775Ctx;

    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    if (!pConfig || !pConfig->pSensor)
        return (RET_NULL_POINTER);

    pOV2775Ctx = (OV2775_Context_t *) malloc(sizeof(OV2775_Context_t));
    if (!pOV2775Ctx) {
        TRACE(OV2775_ERROR, "%s: Can't allocate ov2775 context\n",
              __func__);
        return (RET_OUTOFMEM);
    }

    MEMSET(pOV2775Ctx, 0, sizeof(OV2775_Context_t));

    result = HalAddRef(pConfig->HalHandle);
    if (result != RET_SUCCESS) {
        free(pOV2775Ctx);
        return (result);
    }

    pOV2775Ctx->IsiCtx.HalHandle = pConfig->HalHandle;
    pOV2775Ctx->IsiCtx.pSensor = pConfig->pSensor;
    pOV2775Ctx->GroupHold = BOOL_FALSE;
    pOV2775Ctx->OldGain = 0;
    pOV2775Ctx->OldIntegrationTime = 0;
    pOV2775Ctx->Configured = BOOL_FALSE;
    pOV2775Ctx->Streaming = BOOL_FALSE;
    pOV2775Ctx->TestPattern = BOOL_FALSE;
    pOV2775Ctx->isAfpsRun = BOOL_FALSE;
    pOV2775Ctx->SensorMode.index = pConfig->SensorModeIndex;
    pConfig->hSensor = (IsiSensorHandle_t) pOV2775Ctx;
#ifdef SUBDEV_CHAR
    struct vvcam_mode_info *SensorDefaultMode = NULL;
    for (int i=0; i < sizeof(pov2775_mode_info)/ sizeof(struct vvcam_mode_info); i++)
    {
        if (pov2775_mode_info[i].index == pOV2775Ctx->SensorMode.index)
        {
            SensorDefaultMode = &(pov2775_mode_info[i]);
            break;
        }
    }

    if (SensorDefaultMode != NULL)
    {
        switch(SensorDefaultMode->index)
        {
            case 0:
                memcpy(pOV2775Ctx->SensorRegCfgFile,
                    "OV2775_mipi4lane_1080p_init.txt",
                    strlen("OV2775_mipi4lane_1080p_init.txt"));
                break;
            case 1: //3Dol mode
                /*memcpy(pOV2775Ctx->SensorRegCfgFile,
                    "OV2775_mipi4lane_1080p_2dol_init.txt",
                    strlen("OV2775_mipi4lane_1080p_2dol_init.txt"));*/
                memcpy(pOV2775Ctx->SensorRegCfgFile,
                    "OV2775_mipi4lane_1080p_3dol_init.txt",
                    strlen("OV2775_mipi4lane_1080p_3dol_init.txt"));
                break;
            default:
                break;
        }

        if (access(pOV2775Ctx->SensorRegCfgFile, F_OK) == 0) {
            pOV2775Ctx->KernelDriverFlag = 0;
            memcpy(&(pOV2775Ctx->SensorMode),SensorDefaultMode,sizeof(struct vvcam_mode_info));
        } else {
            pOV2775Ctx->KernelDriverFlag = 1;
        }
    }else
    {
        pOV2775Ctx->KernelDriverFlag = 1;
    }

    result = OV2775_IsiSensorSetPowerIss(pOV2775Ctx, BOOL_TRUE);
    RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);

    uint32_t SensorClkIn;
    if (pOV2775Ctx->KernelDriverFlag) {
        result = OV2775_IsiSensorGetClkIss(pOV2775Ctx, &SensorClkIn);
        RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);
    }

    result = OV2775_IsiSensorSetClkIss(pOV2775Ctx, SensorClkIn);
    RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);

    result = OV2775_IsiResetSensorIss(pOV2775Ctx);
    RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);

    if (!pOV2775Ctx->KernelDriverFlag) {
        result = OV2775_IsiConfigSensorSCCBIss(pOV2775Ctx);
        RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);
    }

    pOV2775Ctx->pattern = ISI_BPAT_GBGBRGRG;
#endif

#ifdef SUBDEV_V4L2
    pOV2775Ctx->pattern = ISI_BPAT_BGBGGRGR;
    pOV2775Ctx->subdev = HalGetFdHandle(pConfig->HalHandle, HAL_MODULE_SENSOR);
    pOV2775Ctx->KernelDriverFlag = 1;
#endif
    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiGetRegCfgIss(const char *registerFileName,
                     struct vvcam_sccb_array *arry)
{
    if (NULL == registerFileName) {
        TRACE(OV2775_ERROR, "%s:registerFileName is NULL\n", __func__);
        return (RET_NULL_POINTER);
    }
#ifdef SUBDEV_CHAR
    FILE *fp = NULL;
    fp = fopen(registerFileName, "rb");
    if (!fp) {
        TRACE(OV2775_ERROR, "%s:load register file  %s error!\n",
              __func__, registerFileName);
        return (RET_FAILURE);
    }

    char LineBuf[512];
    uint32_t FileTotalLine = 0;
    while (!feof(fp)) {
        fgets(LineBuf, 512, fp);
        FileTotalLine++;
    }

    arry->sccb_data =
        malloc(FileTotalLine * sizeof(struct vvcam_sccb_data));
    if (arry->sccb_data == NULL) {
        TRACE(OV2775_ERROR, "%s:malloc failed NULL Point!\n", __func__,
              registerFileName);
        return (RET_FAILURE);
    }
    rewind(fp);

    arry->count = 0;
    while (!feof(fp)) {
        memset(LineBuf, 0, sizeof(LineBuf));
        fgets(LineBuf, 512, fp);

        int result =
            sscanf(LineBuf, "0x%x 0x%x",
               &(arry->sccb_data[arry->count].addr),
               &(arry->sccb_data[arry->count].data));
        if (result != 2)
            continue;
        arry->count++;

    }
#endif

    return 0;
}

static RESULT OV2775_IsiInitSensorIss(IsiSensorHandle_t handle) {
    RESULT result = RET_SUCCESS;
    int ret = 0;
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;

    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    TRACE(OV2775_INFO, "%s (enter)\n", __func__);
    if (pOV2775Ctx == NULL) {
        return (RET_WRONG_HANDLE);
    }
    if (pOV2775Ctx->KernelDriverFlag) {
#ifdef SUBDEV_CHAR

        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_INIT, &(pOV2775Ctx->SensorMode));
        if (ret != 0) {
            TRACE(OV2775_ERROR, "%s:sensor init error!\n",
                  __func__);
            return (RET_FAILURE);
        }

        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_SENSOR_MODE, &(pOV2775Ctx->SensorMode));
        if (ret != 0) {
            TRACE(OV2775_ERROR, "%s:sensor get mode info error!\n",
                  __func__);
            return (RET_FAILURE);
        }

        struct vvcam_ae_info_s ae_info;
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_AE_INFO, &ae_info);
        if (ret != 0) {
            TRACE(OV2775_ERROR, "%s:sensor get ae info error!\n",
                  __func__);
            return (RET_FAILURE);
        }
        pOV2775Ctx->one_line_exp_time =
            (float)ae_info.one_line_exp_time_ns / 1000000000;
        pOV2775Ctx->MaxIntegrationLine = ae_info.max_interrgation_time;
        pOV2775Ctx->MinIntegrationLine = ae_info.min_interrgation_time;
        pOV2775Ctx->gain_accuracy = ae_info.gain_accuracy;
        pOV2775Ctx->AecMinGain = (float)(ae_info.min_gain) / ae_info.gain_accuracy;
        pOV2775Ctx->AecMaxGain = (float)(ae_info.max_gain) / ae_info.gain_accuracy;

        pOV2775Ctx->MaxFps  = pOV2775Ctx->SensorMode.fps;
        pOV2775Ctx->MinFps  = 1;
        pOV2775Ctx->CurrFps = pOV2775Ctx->MaxFps;
#endif

#ifdef SUBDEV_V4L2
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_SENSOR_MODE, &(pOV2775Ctx->SensorMode));
        if (ret != 0) {
            TRACE(OV2775_ERROR, "%s:sensor get mode info error!\n",
                  __func__);
            return (RET_FAILURE);
        }

        pOV2775Ctx->one_line_exp_time  = (float) (pOV2775Ctx->SensorMode.ae_info.one_line_exp_time_ns) / 1000000000;
        pOV2775Ctx->MaxIntegrationLine = pOV2775Ctx->SensorMode.ae_info.max_interrgation_time;
        pOV2775Ctx->MinIntegrationLine = pOV2775Ctx->SensorMode.ae_info.min_interrgation_time;
        pOV2775Ctx->gain_accuracy      = pOV2775Ctx->SensorMode.ae_info.gain_accuracy;
        pOV2775Ctx->AecMaxGain         = (float)(pOV2775Ctx->SensorMode.ae_info.max_gain) /pOV2775Ctx->gain_accuracy ;
        pOV2775Ctx->AecMinGain         = (float)(pOV2775Ctx->SensorMode.ae_info.min_gain) / pOV2775Ctx->gain_accuracy ;
        pOV2775Ctx->MaxFps             = pOV2775Ctx->SensorMode.fps;
        pOV2775Ctx->CurrFps            = pOV2775Ctx->MaxFps;

        if (pOV2775Ctx->SensorMode.hdr_mode == SENSOR_MODE_HDR_STITCH)
        {
            pOV2775Ctx->enableHdr = 1;
        }
#endif

    } else {
        struct vvcam_sccb_array arry;
        result = OV2775_IsiGetRegCfgIss(pOV2775Ctx->SensorRegCfgFile, &arry);
        if (result != 0) {
            TRACE(OV2775_ERROR,
                  "%s:OV2775_IsiGetRegCfgIss error!\n", __func__);
            return (RET_FAILURE);
        }

        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_WRITE_ARRAY, &arry);
        if (ret != 0) {
            TRACE(OV2775_ERROR, "%s:Sensor Write Reg arry error!\n",
                  __func__);
            return (RET_FAILURE);
        }

        switch(pOV2775Ctx->SensorMode.index)
        {
            case 0:
                pOV2775Ctx->one_line_exp_time = 0.000069607;
                pOV2775Ctx->FrameLengthLines = 0x466;
                pOV2775Ctx->CurFrameLengthLines = pOV2775Ctx->FrameLengthLines;
                pOV2775Ctx->MaxIntegrationLine = pOV2775Ctx->CurFrameLengthLines - 3;
                pOV2775Ctx->MinIntegrationLine = 1;
                pOV2775Ctx->AecMaxGain = 24;
                pOV2775Ctx->AecMinGain = 3;
                break;
            case 1:
                pOV2775Ctx->one_line_exp_time = 0.000069607;
                pOV2775Ctx->FrameLengthLines = 0x466;
                pOV2775Ctx->CurFrameLengthLines = pOV2775Ctx->FrameLengthLines;
                pOV2775Ctx->MaxIntegrationLine = pOV2775Ctx->CurFrameLengthLines - 3;
                pOV2775Ctx->MinIntegrationLine = 1;
                pOV2775Ctx->AecMaxGain = 21;
                pOV2775Ctx->AecMinGain = 3;
                break;
            default:
                return ( RET_NOTAVAILABLE );
                break;
        }
        pOV2775Ctx->MaxFps  = pOV2775Ctx->SensorMode.fps;
        pOV2775Ctx->MinFps  = 1;
        pOV2775Ctx->CurrFps = pOV2775Ctx->MaxFps;
    }


    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiReleaseSensorIss(IsiSensorHandle_t handle) {
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    if (pOV2775Ctx == NULL)
        return (RET_WRONG_HANDLE);

    (void)OV2775_IsiSensorSetStreamingIss(pOV2775Ctx, BOOL_FALSE);
    (void)OV2775_IsiSensorSetPowerIss(pOV2775Ctx, BOOL_FALSE);
    (void)HalDelRef(pOV2775Ctx->IsiCtx.HalHandle);

    MEMSET(pOV2775Ctx, 0, sizeof(OV2775_Context_t));
    free(pOV2775Ctx);
    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiGetCapsIss(IsiSensorHandle_t handle, IsiSensorCaps_t * pIsiSensorCaps) {
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;

    RESULT result = RET_SUCCESS;

    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    if (pOV2775Ctx == NULL)
        return (RET_WRONG_HANDLE);

    if (pIsiSensorCaps == NULL) {
        return (RET_NULL_POINTER);
    }

    pIsiSensorCaps->BusWidth          = pOV2775Ctx->SensorMode.bit_width;
    pIsiSensorCaps->Mode              = ISI_MODE_BAYER;
    pIsiSensorCaps->FieldSelection    = ISI_FIELDSEL_BOTH;
    pIsiSensorCaps->YCSequence        = ISI_YCSEQ_YCBYCR;              
    pIsiSensorCaps->Conv422           = ISI_CONV422_NOCOSITED;    
    pIsiSensorCaps->BPat              = pOV2775Ctx->SensorMode.bayer_pattern;
    pIsiSensorCaps->HPol              = ISI_HPOL_REFPOS;
    pIsiSensorCaps->VPol              = ISI_VPOL_NEG;    
    pIsiSensorCaps->Edge              = ISI_EDGE_RISING;    
    pIsiSensorCaps->Resolution.width  = pOV2775Ctx->SensorMode.width;
    pIsiSensorCaps->Resolution.height = pOV2775Ctx->SensorMode.height;
    pIsiSensorCaps->SmiaMode          = ISI_SMIA_OFF;
    pIsiSensorCaps->MipiLanes         = ISI_MIPI_4LANES;  
    pIsiSensorCaps->enableHdr         = pOV2775Ctx->enableHdr;
    if (pIsiSensorCaps->BusWidth == 10) {
        pIsiSensorCaps->MipiMode      = ISI_MIPI_MODE_RAW_10;
    }else if (pIsiSensorCaps->BusWidth == 12){
        pIsiSensorCaps->MipiMode      = ISI_MIPI_MODE_RAW_12;
    }else{
        pIsiSensorCaps->MipiMode      = ISI_MIPI_OFF;
    }
    
    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_AecSetModeParameters
    (OV2775_Context_t * pOV2775Ctx, const IsiSensorConfig_t * pConfig) {
    RESULT result = RET_SUCCESS;
    TRACE(OV2775_INFO, "%s%s: (enter)\n", __func__,
          pOV2775Ctx->isAfpsRun ? "(AFPS)" : "");

    pOV2775Ctx->AecIntegrationTimeIncrement = pOV2775Ctx->one_line_exp_time;
    pOV2775Ctx->AecMinIntegrationTime =
        pOV2775Ctx->one_line_exp_time * pOV2775Ctx->MinIntegrationLine;
    pOV2775Ctx->AecMaxIntegrationTime =
        pOV2775Ctx->one_line_exp_time * pOV2775Ctx->MaxIntegrationLine;

    TRACE(OV2775_DEBUG, "%s%s: AecMaxIntegrationTime = %f \n", __func__,
          pOV2775Ctx->isAfpsRun ? "(AFPS)" : "",
          pOV2775Ctx->AecMaxIntegrationTime);

    pOV2775Ctx->AecGainIncrement = OV2775_MIN_GAIN_STEP;

    //reflects the state of the sensor registers, must equal default settings
    pOV2775Ctx->AecCurGain = pOV2775Ctx->AecMinGain;
    pOV2775Ctx->AecCurIntegrationTime = 0.0f;
    pOV2775Ctx->OldGain = 0;
    pOV2775Ctx->OldIntegrationTime = 0;

    TRACE(OV2775_INFO, "%s%s: (exit)\n", __func__,
          pOV2775Ctx->isAfpsRun ? "(AFPS)" : "");

    return (result);
}

#ifdef SUBDEV_V4L2
RESULT OV2775_Private_SetFormat(IsiSensorHandle_t handle, int width, int height, unsigned int hdr_mode)
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    TRACE(OV2775_INFO, "%s: %d %d %d\n", __func__, width, height, hdr_mode);

    ioctl(pOV2775Ctx->subdev, VVSENSORIOC_S_HDR_MODE, &hdr_mode);

    struct v4l2_subdev_format format;
    format.format.width = width;
    format.format.height = height;
    format.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    format.pad = 0;
    int rc = ioctl(pOV2775Ctx->subdev, VIDIOC_SUBDEV_S_FMT, &format);
    return rc == 0 ? RET_SUCCESS : RET_FAILURE;
}
#endif

static RESULT OV2775_IsiSetupSensorIss
    (IsiSensorHandle_t handle, const IsiSensorConfig_t * pConfig) {
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    if (!pOV2775Ctx) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pConfig) {
        TRACE(OV2775_ERROR,
              "%s: Invalid configuration (NULL pointer detected)\n",
              __func__);
        return (RET_NULL_POINTER);
    }

    if (pOV2775Ctx->Streaming != BOOL_FALSE) {
        return RET_WRONG_STATE;
    }

    memcpy(&pOV2775Ctx->Config, pConfig, sizeof(IsiSensorConfig_t));

    /* 1.) SW reset of image sensor (via I2C register interface)  be careful, bits 6..0 are reserved, reset bit is not sticky */
    TRACE(OV2775_DEBUG, "%s: OV2775 System-Reset executed\n", __func__);
    osSleep(100);

    result = OV2775_AecSetModeParameters(pOV2775Ctx, pConfig);
    if (result != RET_SUCCESS) {
        TRACE(OV2775_ERROR, "%s: SetupOutputWindow failed.\n",
              __func__);
        return (result);
    }
#ifdef SUBDEV_V4L2
     OV2775_Private_SetFormat(pOV2775Ctx,
                            pOV2775Ctx->SensorMode.width,
                            pOV2775Ctx->SensorMode.height,
                            pOV2775Ctx->SensorMode.hdr_mode);
#endif

    pOV2775Ctx->Configured = BOOL_TRUE;
    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return 0;
}

static RESULT OV2775_IsiChangeSensorResolutionIss(IsiSensorHandle_t handle, uint16_t width, uint16_t height) {
    RESULT result = RET_SUCCESS;
    
    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiSensorSetStreamingIss
    (IsiSensorHandle_t handle, bool_t on) {
    RESULT result = RET_SUCCESS;
    int ret = 0;
    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    if ((pOV2775Ctx->Configured != BOOL_TRUE)
        || (pOV2775Ctx->Streaming == on))
        return RET_WRONG_STATE;

    int32_t enable = (uint32_t) on;
    if (pOV2775Ctx->KernelDriverFlag) {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_STREAM, &enable);

    } else {
        ret = OV2775_IsiRegisterWriteIss(handle, 0x3012, on);
    }

    if (ret != 0) {
        return (RET_FAILURE);
    }

    pOV2775Ctx->Streaming = on;

    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiCheckSensorConnectionIss(IsiSensorHandle_t handle) {
    RESULT result = RET_SUCCESS;
    int ret = 0;
    uint32_t correct_id = 0x2770;
    uint32_t sensor_id = 0;

    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    if (pOV2775Ctx->KernelDriverFlag) {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_RESERVE_ID,
              &correct_id);
        if (ret != 0) {
            TRACE(OV2775_ERROR,
                  "%s: Read Sensor correct ID Error! \n", __func__);
            return (RET_FAILURE);
        }

        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_CHIP_ID,
              &sensor_id);
        if (ret != 0) {
            TRACE(OV2775_ERROR,
                  "%s: Read Sensor chip ID Error! \n", __func__);
            return (RET_FAILURE);
        }
    } else {

        result = OV2775_IsiGetSensorRevisionIss(handle, &sensor_id);
        if (result != RET_SUCCESS) {
            TRACE(OV2775_ERROR, "%s: Read Sensor ID Error! \n",
                  __func__);
            return (RET_FAILURE);
        }
    }

    if (correct_id != sensor_id) {
        TRACE(OV2775_ERROR, "%s:ChipID =0x%x sensor_id=%x error! \n",
              __func__, correct_id, sensor_id);
        return (RET_FAILURE);
    }

    TRACE(OV2775_INFO,
          "%s ChipID = 0x%08x, sensor_id = 0x%08x, success! \n", __func__,
          correct_id, sensor_id);
    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiGetSensorRevisionIss
    (IsiSensorHandle_t handle, uint32_t * p_value) {
    RESULT result = RET_SUCCESS;
    int ret = 0;
    uint32_t reg_val;
    uint32_t sensor_id;

    TRACE(OV2775_INFO, "%s (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    if (!p_value)
        return (RET_NULL_POINTER);

    if (pOV2775Ctx->KernelDriverFlag) {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_CHIP_ID,
              &sensor_id);
        if (ret != 0) {
            TRACE(OV2775_ERROR, "%s: Read Sensor ID Error! \n",
                  __func__);
            return (RET_FAILURE);
        }
    } else {
        reg_val = 0;
        result = OV2775_IsiRegisterReadIss(handle, 0x300a, &reg_val);
        sensor_id = (reg_val & 0xff) << 8;

        reg_val = 0;
        result |= OV2775_IsiRegisterReadIss(handle, 0x300b, &reg_val);
        sensor_id |= (reg_val & 0xff);

    }

    *p_value = sensor_id;
    TRACE(OV2775_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiGetGainLimitsIss
    (IsiSensorHandle_t handle, float *pMinGain, float *pMaxGain) {
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (pOV2775Ctx == NULL) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if ((pMinGain == NULL) || (pMaxGain == NULL)) {
        TRACE(OV2775_ERROR, "%s: NULL pointer received!!\n");
        return (RET_NULL_POINTER);
    }

    *pMinGain = pOV2775Ctx->AecMinGain;
    *pMaxGain = pOV2775Ctx->AecMaxGain;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiGetIntegrationTimeLimitsIss
    (IsiSensorHandle_t handle,
     float *pMinIntegrationTime, float *pMaxIntegrationTime) {
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);
    if (pOV2775Ctx == NULL) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if ((pMinIntegrationTime == NULL) || (pMaxIntegrationTime == NULL)) {
        TRACE(OV2775_ERROR, "%s: NULL pointer received!!\n");
        return (RET_NULL_POINTER);
    }

    *pMinIntegrationTime = pOV2775Ctx->AecMinIntegrationTime;
    *pMaxIntegrationTime = pOV2775Ctx->AecMaxIntegrationTime;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);
    return (result);
}

RESULT OV2775_IsiGetGainIss(IsiSensorHandle_t handle, float *pSetGain) {
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (pOV2775Ctx == NULL) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (pSetGain == NULL) {
        return (RET_NULL_POINTER);
    }

    *pSetGain = pOV2775Ctx->AecCurGain;

    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OV2775_IsiGetLongGainIss(IsiSensorHandle_t handle, float *gain) 
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (pOV2775Ctx == NULL) {
        TRACE(OV2775_ERROR,"%s: Invalid sensor handle (NULL pointer detected)\n",__func__);
        return (RET_WRONG_HANDLE);
    }

    if (gain == NULL) {
        return (RET_NULL_POINTER);
    }

    *gain = pOV2775Ctx->AecCurLongGain;

    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);

    return (RET_SUCCESS);
}

RESULT OV2775_IsiGetVSGainIss(IsiSensorHandle_t handle, float *pSetGain) {
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (pOV2775Ctx == NULL) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (pSetGain == NULL) {
        return (RET_NULL_POINTER);
    }

    *pSetGain = pOV2775Ctx->AecCurVSGain;

    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);

    return (result);
}

RESULT OV2775_IsiGetGainIncrementIss(IsiSensorHandle_t handle, float *pIncr) {
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (pOV2775Ctx == NULL) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (pIncr == NULL)
        return (RET_NULL_POINTER);

    *pIncr = pOV2775Ctx->AecGainIncrement;

    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);

    return (result);
}

RESULT OV2775_IsiSetGainIss
    (IsiSensorHandle_t handle,
     float NewGain, float *pSetGain, float *hdr_ratio) {

    RESULT result = RET_SUCCESS;
    int32_t ret = 0;

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    if (pOV2775Ctx->KernelDriverFlag) {
        uint32_t SensorGain = 0;
        SensorGain = NewGain * pOV2775Ctx->gain_accuracy;

    #ifdef SUBDEV_CHAR
        if (pOV2775Ctx->enableHdr == true) {
            uint32_t SensorHdrRatio = (uint32_t)*hdr_ratio;
            ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_HDR_RADIO, &SensorHdrRatio);
        }
    #endif
        ret |= ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_GAIN, &SensorGain);
        if (ret != 0) {
            TRACE(OV2775_ERROR, "%s: set sensor gain error\n",
                  __func__);
            return RET_FAILURE;
        }
    } else {
        float Gain = 0.0f;
        float dGainHcg = 0.0f;
        float dGainLcg = 0.0f;
        uint32_t gainHcg = 0;
        uint32_t gainLcg = 0;
        uint32_t againHcg = 0;
        uint32_t againLcg = 0;
        uint32_t data = 0;
        //struct sensor_gain_context gain;
        TRACE(OV2775_INFO, "%s: (Linear mode enter)\n", __func__);

        if ((pSetGain == NULL) || (hdr_ratio == NULL))
            return (RET_NULL_POINTER);

        if (NewGain < pOV2775Ctx->AecMinGain)
            NewGain = pOV2775Ctx->AecMinGain;
        if (NewGain > pOV2775Ctx->AecMaxGain)
            NewGain = pOV2775Ctx->AecMaxGain;
        if (pOV2775Ctx->enableHdr == true) {
            Gain = NewGain * 11;
            if (Gain < 22.0) {
                dGainLcg = NewGain;
                againLcg = 0x00;
            } else if (Gain < 44.0) {
                dGainLcg = NewGain;
                againLcg = 0x00;
            } else if (Gain < 88.0) {
                dGainLcg = NewGain / 2.0;
                againLcg = 0x01;
            } else if (Gain < 176) {
                dGainLcg = NewGain / 4.0;
                againLcg = 0x02;
            } else {
                dGainLcg = NewGain / 8.0;
                againLcg = 0x03;
            }

            if (NewGain < 3.0) {
                dGainHcg = 0x02;
                againHcg = 0x00;
            } else if (NewGain < 4.375 && NewGain >= 3.0) {
                dGainHcg = NewGain / 2.0;
                againHcg = 0x01;
            } else if (NewGain < 8.750 && NewGain >= 4.375) {
                dGainHcg = NewGain / 4.0;
                againHcg = 0x02;
            } else {
                dGainHcg = NewGain / 8.0;
                againHcg = 0x03;
            }

            gainLcg = (dGainLcg * (*hdr_ratio)) * 256;
            gainHcg = dGainLcg * 256;
            OV2775_IsiRegisterReadIss(handle, 0x30bb, &data);
            data = (data & againHcg);
            data = (data & (((againLcg << 2) & 0x0c)));
            if ((dGainHcg != pOV2775Ctx->OldGainHcg)
                || (againHcg != pOV2775Ctx->OldAGainHcg)) {
                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x3467,
                                   0x00);
                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x3464,
                                   0x04);

                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x315a,
                                   (gainHcg >> 8) &
                                   0xff);
                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x355b,
                                   gainHcg & 0xff);

                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x315c,
                                   (gainLcg >> 8) &
                                   0xff);
                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x315d,
                                   gainLcg & 0xff);

                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x30bb,
                                   data);

                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x3464,
                                   0x14);
                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x3467,
                                   0x01);
            }
        } else {
            if (NewGain < 3.0) {
                NewGain = 3.0;
                dGainHcg = NewGain;
                againHcg = 0x00;
            } else if (NewGain < 6.0 && NewGain >= 3.0) {
                dGainHcg = NewGain;
                againHcg = 0x00;
            } else if (NewGain < 12.0 && NewGain >= 6.0) {
                dGainHcg = NewGain / 2.0;
                againHcg = 0x01;
            } else if (NewGain < 24.0 && NewGain >= 12.0) {
                dGainHcg = NewGain / 4.0;
                againHcg = 0x02;
            } else if (NewGain >= 24.0) {
                dGainHcg = NewGain / 8.0;
                againHcg = 0x03;
            }
            gainHcg = dGainHcg * 256;

            OV2775_IsiRegisterReadIss(handle, 0x30bb, &data);
            data = (data & (againHcg));

            if ((dGainHcg != pOV2775Ctx->OldGainHcg)
                || (againHcg != pOV2775Ctx->OldAGainHcg)) {

                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x3467,
                                   0x00);
                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x3464,
                                   0x04);

                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x315a,
                                   ((gainHcg >> 8) &
                                0x00ff));
                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x315b,
                                   (gainHcg &
                                0x00ff));

                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x30bb,
                                   data);

                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x3464,
                                   0x14);
                result =
                    OV2775_IsiRegisterWriteIss(handle, 0x3467,
                                   0x01);

            }
            if ((dGainHcg != pOV2775Ctx->OldGainHcg)
                || (againHcg != pOV2775Ctx->OldAGainHcg)) {

                pOV2775Ctx->OldGainHcg = dGainHcg;
                pOV2775Ctx->OldAGainHcg = againHcg;
                pOV2775Ctx->OldAGainLcg = dGainLcg;
            }
        }

    }

    pOV2775Ctx->AecCurGain = ((float)(NewGain));

    *pSetGain = pOV2775Ctx->AecCurGain;
    TRACE(OV2775_DEBUG, "%s: g=%f\n", __func__, *pSetGain);
    return (result);
}

RESULT OV2775_IsiSetLongGainIss(IsiSensorHandle_t handle, float gain)
{
    int ret = 0;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;

    if (!pOV2775Ctx || !pOV2775Ctx->IsiCtx.HalHandle) 
    {
        TRACE(OV2775_ERROR,"%s: Invalid sensor handle (NULL pointer detected)\n",__func__);
        return (RET_WRONG_HANDLE);
    }
    
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;
    if (pOV2775Ctx->KernelDriverFlag)
    {
        uint32_t SensorGain = 0;
        SensorGain = gain * pOV2775Ctx->gain_accuracy;
        if (pOV2775Ctx->LastLongGain != SensorGain)
        {
            ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_LONG_GAIN, &SensorGain);
            if (ret != 0)
            {
                return (RET_FAILURE);
                TRACE(OV2775_ERROR,"%s: set long gain failed\n");

            }
            pOV2775Ctx->LastLongGain = SensorGain;
            pOV2775Ctx->AecCurLongGain = gain;
        }
        
    }

    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (RET_SUCCESS);
}

RESULT OV2775_IsiSetVSGainIss
    (IsiSensorHandle_t handle,
     float NewIntegrationTime,
     float NewGain, float *pSetGain, float *hdr_ratio) {
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;
    RESULT result = RET_SUCCESS;

    float Gain = 0.0f;

    uint32_t ucGain = 0U;
    uint32_t again = 0U;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (!pOV2775Ctx) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pSetGain || !hdr_ratio)
        return (RET_NULL_POINTER);

    if (pOV2775Ctx->KernelDriverFlag) {
        uint32_t SensorGain = 0;
        SensorGain = NewGain * pOV2775Ctx->gain_accuracy;
        ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_VSGAIN, &SensorGain);
    } else {
        Gain = NewGain / 2.0;
        if (Gain < 1.0)
            Gain = 1.0;
        ucGain = (uint32_t) (Gain * 256);

        TRACE(OV2775_INFO,
              "%s: set Gain ucGainvs:=0x%03x NewIntegrationTime = %f,NewGain=%f\n",
              __func__, ucGain, NewIntegrationTime, NewGain);

        if (pOV2775Ctx->OldVsGain != ucGain) {

            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3467, 0x00);
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3464, 0x04);

            result =
                OV2775_IsiRegisterWriteIss(handle, 0x315e,
                               (ucGain >> 8) & 0xff);
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x315f,
                               ucGain & 0xff);

            result =
                OV2775_IsiRegisterReadIss(handle, 0x30bb, &again);
            again &= 0x3f;
            again |= 8;
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x30bb, again);

            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3464, 0x14);
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3467, 0x01);

            pOV2775Ctx->OldVsGain = ucGain;
        }
    }

    pOV2775Ctx->AecCurVSGain = NewGain;
    *pSetGain = pOV2775Ctx->AecCurGain;
    TRACE(OV2775_DEBUG, "%s: g=%f\n", __func__, *pSetGain);
    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OV2775_IsiSetBayerPattern(IsiSensorHandle_t handle, uint8_t pattern)
{

    RESULT result = RET_SUCCESS;
    uint8_t h_shift = 0, v_shift = 0;
    uint32_t val_h = 0, val_l = 0;
    uint16_t val = 0;
    uint8_t Start_p = 0;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    // pattern 0:B 1:GB 2:GR 3:R
    result = OV2775_IsiSensorSetStreamingIss(handle, 0);
    switch (pattern) {
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

    OV2775_IsiRegisterReadIss(handle, 0x30a0, &val_h);
    OV2775_IsiRegisterReadIss(handle, 0x30a1, &val_l);
    val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + h_shift;
    val_h = (val >> 8) & 0xff;
    val_l = val & 0xff;
    OV2775_IsiRegisterWriteIss(handle, 0x30a0, (uint8_t)val_h);
    OV2775_IsiRegisterWriteIss(handle, 0x30a1, (uint8_t)val_l);

    OV2775_IsiRegisterReadIss(handle, 0x30a2, &val_h);
    OV2775_IsiRegisterReadIss(handle, 0x30a3, &val_l);
    val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + v_shift;
    val_h = (val >> 8) & 0xff;
    val_l = val & 0xff;
    OV2775_IsiRegisterWriteIss(handle, 0x30a2, (uint8_t)val_h);
    OV2775_IsiRegisterWriteIss(handle, 0x30a3, (uint8_t)val_l);

    OV2775_IsiRegisterReadIss(handle, 0x30a4, &val_h);
    OV2775_IsiRegisterReadIss(handle, 0x30a5, &val_l);
    val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + h_shift;
    val_h = (val >> 8) & 0xff;
    val_l = val & 0xff;
    OV2775_IsiRegisterWriteIss(handle, 0x30a4, (uint8_t)val_h);
    OV2775_IsiRegisterWriteIss(handle, 0x30a5, (uint8_t)val_l);

    OV2775_IsiRegisterReadIss(handle, 0x30a6, &val_h);
    OV2775_IsiRegisterReadIss(handle, 0x30a7, &val_l);
    val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + v_shift;
    val_h = (val >> 8) & 0xff;
    val_l = val & 0xff;
    OV2775_IsiRegisterWriteIss(handle, 0x30a6, (uint8_t)val_h);
    OV2775_IsiRegisterWriteIss(handle, 0x30a7, (uint8_t)val_l);

    pOV2775Ctx->pattern = pattern;
    result = OV2775_IsiSensorSetStreamingIss(handle, pOV2775Ctx->Streaming);

    return (result);
}

RESULT OV2775_IsiGetIntegrationTimeIss
    (IsiSensorHandle_t handle, float *pSetIntegrationTime)
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (!pOV2775Ctx) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pSetIntegrationTime)
        return (RET_NULL_POINTER);
    *pSetIntegrationTime = pOV2775Ctx->AecCurIntegrationTime;
    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OV2775_IsiGetLongIntegrationTimeIss(IsiSensorHandle_t handle, float *pIntegrationTime)
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (!pOV2775Ctx) {
        TRACE(OV2775_ERROR,"%s: Invalid sensor handle (NULL pointer detected)\n",__func__);
        return (RET_WRONG_HANDLE);
    }
    if (!pIntegrationTime)
        return (RET_NULL_POINTER);

    *pIntegrationTime = pOV2775Ctx->AecCurLongIntegrationTime;
    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (RET_SUCCESS);
}

RESULT OV2775_IsiGetVSIntegrationTimeIss
    (IsiSensorHandle_t handle, float *pSetIntegrationTime)
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (!pOV2775Ctx) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }
    if (!pSetIntegrationTime)
        return (RET_NULL_POINTER);

    *pSetIntegrationTime = pOV2775Ctx->AecCurVSIntegrationTime;
    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OV2775_IsiGetIntegrationTimeIncrementIss
    (IsiSensorHandle_t handle, float *pIncr)
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (!pOV2775Ctx) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pIncr)
        return (RET_NULL_POINTER);

    //_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
    *pIncr = pOV2775Ctx->AecIntegrationTimeIncrement;
    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OV2775_IsiSetIntegrationTimeIss
    (IsiSensorHandle_t handle,
     float NewIntegrationTime,
     float *pSetIntegrationTime,
     uint8_t * pNumberOfFramesToSkip, float *hdr_ratio)
{
    RESULT result = RET_SUCCESS;

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    uint32_t exp_line = 0;
    uint32_t exp_line_old = 0;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (!pOV2775Ctx) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pSetIntegrationTime || !pNumberOfFramesToSkip) {
        TRACE(OV2775_ERROR,
              "%s: Invalid parameter (NULL pointer detected)\n",
              __func__);
        return (RET_NULL_POINTER);
    }

    exp_line = NewIntegrationTime / pOV2775Ctx->one_line_exp_time;
    exp_line_old = exp_line;
    exp_line =
        MIN(pOV2775Ctx->MaxIntegrationLine,
        MAX(pOV2775Ctx->MinIntegrationLine, exp_line));

    TRACE(OV2775_DEBUG, "%s: set AEC_PK_EXPO=0x%05x\n", __func__, exp_line);

    if (exp_line != pOV2775Ctx->OldIntegrationTime) {
        if (pOV2775Ctx->KernelDriverFlag) {
            ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_EXP, &exp_line);
        } else {
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3467, 0x00);
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3464, 0x04);

            result =
                OV2775_IsiRegisterWriteIss(handle, 0x30b6,
                               (exp_line >> 8) & 0xff);
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x30b7,
                               exp_line & 0xff);

            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3464, 0x14);
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3467, 0x01);
        }

        pOV2775Ctx->OldIntegrationTime = exp_line;    // remember current integration time
        pOV2775Ctx->AecCurIntegrationTime =
            exp_line * pOV2775Ctx->one_line_exp_time;

        *pNumberOfFramesToSkip = 1U;    //skip 1 frame
    } else {
        *pNumberOfFramesToSkip = 0U;    //no frame skip
    }

    if (exp_line_old != exp_line) {
        *pSetIntegrationTime = pOV2775Ctx->AecCurIntegrationTime;
    } else {
        *pSetIntegrationTime = NewIntegrationTime;
    }

    TRACE(OV2775_DEBUG, "%s: Ti=%f\n", __func__, *pSetIntegrationTime);
    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OV2775_IsiSetLongIntegrationTimeIss(IsiSensorHandle_t handle,float IntegrationTime)
{
    int ret;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (!handle || !pOV2775Ctx->IsiCtx.HalHandle) 
    {
        TRACE(OV2775_ERROR,"%s: Invalid sensor handle (NULL pointer detected)\n",__func__);
        return (RET_WRONG_HANDLE);
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;
    
    uint32_t exp_line = 0;
    exp_line = IntegrationTime / pOV2775Ctx->one_line_exp_time;
    exp_line = MIN(pOV2775Ctx->MaxIntegrationLine, MAX(pOV2775Ctx->MinIntegrationLine, exp_line));

    if (exp_line != pOV2775Ctx->LastLongExpLine)
    {
        if (pOV2775Ctx->KernelDriverFlag)
        {
            ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_LONG_EXP, &exp_line);
            if (ret != 0)
            {
                TRACE(OV2775_ERROR,"%s: set long gain failed\n");
                return RET_FAILURE;
            }
        }
        
        pOV2775Ctx->LastLongExpLine = exp_line;
        pOV2775Ctx->AecCurLongIntegrationTime =  pOV2775Ctx->LastLongExpLine*pOV2775Ctx->one_line_exp_time; 
    }


    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (RET_SUCCESS); 
}

RESULT OV2775_IsiSetVSIntegrationTimeIss
    (IsiSensorHandle_t handle,
     float NewIntegrationTime,
     float *pSetVSIntegrationTime,
     uint8_t * pNumberOfFramesToSkip, float *hdr_ratio)
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;
    RESULT result = RET_SUCCESS;
    uint32_t exp_line = 0;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (!pOV2775Ctx) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pSetVSIntegrationTime || !pNumberOfFramesToSkip) {
        TRACE(OV2775_ERROR,
              "%s: Invalid parameter (NULL pointer detected)\n",
              __func__);
        return (RET_NULL_POINTER);
    }

    TRACE(OV2775_INFO,
          "%s:  maxIntegrationTime-=%f minIntegrationTime = %f\n", __func__,
          pOV2775Ctx->AecMaxIntegrationTime,
          pOV2775Ctx->AecMinIntegrationTime);


    exp_line = NewIntegrationTime / pOV2775Ctx->one_line_exp_time;
    exp_line =
        MIN(pOV2775Ctx->MaxIntegrationLine,
        MAX(pOV2775Ctx->MinIntegrationLine, exp_line));

    if (exp_line != pOV2775Ctx->OldVsIntegrationTime) {
        if (pOV2775Ctx->KernelDriverFlag) {
            ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_VSEXP, &exp_line);
        } else {
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3467, 0x00);
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3464, 0x04);

            result =
                OV2775_IsiRegisterWriteIss(handle, 0x30b8,
                               (exp_line >> 8) & 0xff);
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x30b9,
                               exp_line & 0xff);

            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3467, 0x14);
            result =
                OV2775_IsiRegisterWriteIss(handle, 0x3464, 0x01);
        }

        pOV2775Ctx->OldVsIntegrationTime = exp_line;
        pOV2775Ctx->AecCurVSIntegrationTime = exp_line * pOV2775Ctx->one_line_exp_time;    //remember current integration time
        *pNumberOfFramesToSkip = 1U;    //skip 1 frame
    } else {
        *pNumberOfFramesToSkip = 0U;    //no frame skip
    }

    *pSetVSIntegrationTime = pOV2775Ctx->AecCurVSIntegrationTime;

    TRACE(OV2775_DEBUG, "%s: NewIntegrationTime=%f\n", __func__,
          NewIntegrationTime);
    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OV2775_IsiExposureControlIss
    (IsiSensorHandle_t handle,
     float NewGain,
     float NewIntegrationTime,
     uint8_t * pNumberOfFramesToSkip,
     float *pSetGain, float *pSetIntegrationTime, float *hdr_ratio)
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (pOV2775Ctx == NULL) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if ((pNumberOfFramesToSkip == NULL) || (pSetGain == NULL)
        || (pSetIntegrationTime == NULL)) {
        TRACE(OV2775_ERROR,
              "%s: Invalid parameter (NULL pointer detected)\n",
              __func__);
        return (RET_NULL_POINTER);
    }

    TRACE(OV2775_DEBUG, "%s: g=%f, Ti=%f\n", __func__, NewGain,
          NewIntegrationTime);

    float long_gain=0;
    float long_exp=0;
    float short_gain=0;
    float short_exp=0;

    if (pOV2775Ctx->enableHdr)
    {

        long_exp = NewIntegrationTime;
        long_gain = NewGain;

        float short_exposure_measure = NewIntegrationTime*NewGain / *hdr_ratio;

        if (short_exposure_measure < 48 * pOV2775Ctx->one_line_exp_time * pOV2775Ctx->AecMinGain)
        {
            short_exp = short_exposure_measure / pOV2775Ctx->AecMinGain;
            short_gain = pOV2775Ctx->AecMinGain;
        }else
        {
            short_exp = 48 * pOV2775Ctx->one_line_exp_time;
            short_gain = short_exposure_measure / short_exp;
        }

    }else
    {
        long_exp = NewIntegrationTime;
        long_gain = NewGain;
    }

    if (pOV2775Ctx->enableHdr)
    {
        result = OV2775_IsiSetVSIntegrationTimeIss(handle,
                              short_exp,
                              pSetIntegrationTime,
                              pNumberOfFramesToSkip,
                              hdr_ratio);
        result =
            OV2775_IsiSetVSGainIss(handle, short_exp, short_gain,
                       pSetGain, hdr_ratio);

        result = OV2775_IsiSetLongGainIss(handle, long_gain * (*hdr_ratio));
    }
    result =
        OV2775_IsiSetIntegrationTimeIss(handle, long_exp,
                        pSetIntegrationTime,
                        pNumberOfFramesToSkip, hdr_ratio);
    TRACE(OV2775_DEBUG, "%s: set: NewGain=%f, hcgTi=%f, hcgskip=%d\n",
          __func__, NewGain, NewIntegrationTime, *pNumberOfFramesToSkip);
    result = OV2775_IsiSetGainIss(handle, long_gain, pSetGain, hdr_ratio);
    TRACE(OV2775_DEBUG, "%s: set: NewGain=%f, hcgTi=%f, hcgskip=%d\n",
          __func__, NewGain, NewIntegrationTime, *pNumberOfFramesToSkip);
    TRACE(OV2775_DEBUG, "%s: set: vsg=%f, vsTi=%f, vsskip=%d\n", __func__,
          NewGain, NewIntegrationTime, *pNumberOfFramesToSkip);
    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);

    return result;
}

RESULT OV2775_IsiGetCurrentExposureIss
    (IsiSensorHandle_t handle, float *pSetGain, float *pSetIntegrationTime) {
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (pOV2775Ctx == NULL) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if ((pSetGain == NULL) || (pSetIntegrationTime == NULL))
        return (RET_NULL_POINTER);

    *pSetGain = pOV2775Ctx->AecCurGain;
    *pSetIntegrationTime = pOV2775Ctx->AecCurIntegrationTime;

    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OV2775_IsiGetResolutionIss(IsiSensorHandle_t handle, uint16_t *pwidth, uint16_t *pheight) {
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (pOV2775Ctx == NULL) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    *pwidth = pOV2775Ctx->SensorMode.width;
    *pheight =  pOV2775Ctx->SensorMode.height;

    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OV2775_IsiGetSensorFpsIss(IsiSensorHandle_t handle, uint32_t * pfps)
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    if (pOV2775Ctx == NULL) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    if (pOV2775Ctx->KernelDriverFlag) {
        ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_FPS, pfps);
        pOV2775Ctx->CurrFps = *pfps;
    }

    *pfps = pOV2775Ctx->CurrFps;

    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OV2775_IsiSetSensorFpsIss(IsiSensorHandle_t handle, uint32_t fps)
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);
  
    if (pOV2775Ctx == NULL) {
        TRACE(OV2775_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    if (fps > pOV2775Ctx->MaxFps) {
        TRACE(OV2775_ERROR,
              "%s: set fps(%d) out of range, correct to %d (%d, %d)\n",
              __func__, fps, pOV2775Ctx->MaxFps, pOV2775Ctx->MinFps,
              pOV2775Ctx->MaxFps);
        fps = pOV2775Ctx->MaxFps;
    }
    if (fps < pOV2775Ctx->MinFps) {
        TRACE(OV2775_ERROR,
              "%s: set fps(%d) out of range, correct to %d (%d, %d)\n",
              __func__, fps, pOV2775Ctx->MinFps, pOV2775Ctx->MinFps,
              pOV2775Ctx->MaxFps);
        fps = pOV2775Ctx->MinFps;
    }
    if (pOV2775Ctx->KernelDriverFlag) {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_FPS, &fps);
        if (ret != 0) {
            TRACE(OV2775_ERROR, "%s: set sensor fps=%d error\n",
                  __func__);
            return (RET_FAILURE);
        }

        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_SENSOR_MODE, &(pOV2775Ctx->SensorMode));
        {
            pOV2775Ctx->MaxIntegrationLine = pOV2775Ctx->SensorMode.ae_info.max_interrgation_time;
            pOV2775Ctx->AecMaxIntegrationTime = pOV2775Ctx->MaxIntegrationLine * pOV2775Ctx->one_line_exp_time;
        }
#ifdef SUBDEV_CHAR
        struct vvcam_ae_info_s ae_info;
        ret =
            ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_AE_INFO, &ae_info);
        if (ret != 0) {
            TRACE(OV2775_ERROR, "%s:sensor get ae info error!\n",
                  __func__);
            return (RET_FAILURE);
        }
        pOV2775Ctx->one_line_exp_time =
            (float)ae_info.one_line_exp_time_ns / 1000000000;
        pOV2775Ctx->MaxIntegrationLine = ae_info.max_interrgation_time;
        pOV2775Ctx->AecMaxIntegrationTime =
            pOV2775Ctx->MaxIntegrationLine *
            pOV2775Ctx->one_line_exp_time;
#endif
    } else {
        uint16_t FrameLengthLines;
        FrameLengthLines =
            pOV2775Ctx->FrameLengthLines * pOV2775Ctx->MaxFps / fps;
        result =
            OV2775_IsiRegisterWriteIss(handle, 0x30b2,
                           (FrameLengthLines >> 8) & 0xff);
        result |=
            OV2775_IsiRegisterWriteIss(handle, 0x30b3,
                           FrameLengthLines & 0xff);
        if (result != RET_SUCCESS) {
            TRACE(OV2775_ERROR,
                  "%s: Invalid sensor handle (NULL pointer detected)\n",
                  __func__);
            return (RET_FAILURE);
        }
        pOV2775Ctx->CurrFps = fps;
        pOV2775Ctx->CurFrameLengthLines = FrameLengthLines;
        pOV2775Ctx->MaxIntegrationLine =
            pOV2775Ctx->CurFrameLengthLines - 3;
        pOV2775Ctx->AecMaxIntegrationTime =
            pOV2775Ctx->MaxIntegrationLine *
            pOV2775Ctx->one_line_exp_time;
    }

    TRACE(OV2775_INFO, "%s: set sensor fps = %d\n", __func__,
          pOV2775Ctx->CurrFps);

    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return (result);
}

static RESULT OV2775_IsiActivateTestPattern(IsiSensorHandle_t handle,
                        const bool_t enable)
{
    RESULT result = RET_SUCCESS;

    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    if (pOV2775Ctx->Configured != BOOL_TRUE)
        return RET_WRONG_STATE;

    if (BOOL_TRUE == enable) {
        result = OV2775_IsiRegisterWriteIss(handle, 0x3253, 0x80);
    } else {
        result = OV2775_IsiRegisterWriteIss(handle, 0x3253, 0x00);
    }
    pOV2775Ctx->TestPattern = enable;

    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);

    return (result);
}

static RESULT OV2775_IsiEnableHdr(IsiSensorHandle_t handle, const bool_t enable)
{
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    TRACE(OV2775_INFO, "%s: (enter)\n", __func__);

    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

#ifdef SUBDEV_CHAR
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;
    result = OV2775_IsiSensorSetStreamingIss(handle, 0);

    if (pOV2775Ctx->KernelDriverFlag) {
        uint32_t hdr_mode;
        if (enable == 0) {
            hdr_mode = SENSOR_MODE_LINEAR;
        } else {
            hdr_mode = SENSOR_MODE_HDR_STITCH;
        }
        ret =
            ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_HDR_MODE,
              &hdr_mode);
    } else {
        result |=
            OV2775_IsiRegisterWriteIss(handle, 0x3190,
                           enable ? 0x05 : 0x08);
    }
    result |=
        OV2775_IsiSensorSetStreamingIss(handle, pOV2775Ctx->Streaming);
#endif

    if (result != 0 || ret != 0) {
        TRACE(OV2775_ERROR, "%s: change hdr status error\n", __func__);
        return RET_FAILURE;
    }

    pOV2775Ctx->enableHdr = enable;
    TRACE(OV2775_INFO, "%s: (exit)\n", __func__);
    return RET_SUCCESS;
}

static RESULT OV2775_IsiSensorSetBlcIss(IsiSensorHandle_t handle, sensor_blc_t * pblc)
{
    int32_t ret = 0;
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_WRONG_HANDLE;
    }

    if (pblc == NULL)
        return RET_NULL_POINTER;

    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_BLC, pblc);
    if (ret != 0)
    {
         TRACE(OV2775_ERROR, "%s: set wb error\n", __func__);
    }

    return RET_SUCCESS;
}

static RESULT OV2775_IsiSensorSetWBIss(IsiSensorHandle_t handle, sensor_white_balance_t * pwb)
{
    int32_t ret = 0;
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_WRONG_HANDLE;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    if (pwb == NULL)
        return RET_NULL_POINTER;

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_WB, pwb);
    if (ret != 0)
    {
         TRACE(OV2775_ERROR, "%s: set wb error\n", __func__);
    }

    return RET_SUCCESS;
}

static RESULT OV2775_IsiGetSensorAWBModeIss(IsiSensorHandle_t  handle, IsiSensorAwbMode_t *pawbmode)
{
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    if (pOV2775Ctx->SensorMode.hdr_mode == SENSOR_MODE_HDR_NATIVE){
        *pawbmode = ISI_SENSOR_AWB_MODE_SENSOR;
    }else{
        *pawbmode = ISI_SENSOR_AWB_MODE_NORMAL;
    }
    return RET_SUCCESS;
}

static RESULT OV2775_IsiSensorGetExpandCurveIss(IsiSensorHandle_t handle, sensor_expand_curve_t * pexpand_curve)
{
    int32_t ret = 0;
    OV2775_Context_t *pOV2775Ctx = (OV2775_Context_t *) handle;
    if (pOV2775Ctx == NULL || pOV2775Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOV2775Ctx->IsiCtx.HalHandle;

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_EXPAND_CURVE, pexpand_curve);
    if (ret != 0)
    {
        TRACE(OV2775_ERROR, "%s: get  expand cure error\n", __func__);
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}

RESULT OV2775_IsiGetSensorIss(IsiSensor_t *pIsiSensor)
{
    RESULT result = RET_SUCCESS;
    TRACE( OV2775_INFO, "%s (enter)\n", __func__);

    if ( pIsiSensor != NULL ) {
        pIsiSensor->pszName                         = SensorName;
        pIsiSensor->pIsiCreateSensorIss             = OV2775_IsiCreateSensorIss;

        pIsiSensor->pIsiInitSensorIss               = OV2775_IsiInitSensorIss;
        pIsiSensor->pIsiGetSensorModeIss            = OV2775_IsiGetSensorModeIss;
        pIsiSensor->pIsiResetSensorIss              = OV2775_IsiResetSensorIss;
        pIsiSensor->pIsiReleaseSensorIss            = OV2775_IsiReleaseSensorIss;
        pIsiSensor->pIsiGetCapsIss                  = OV2775_IsiGetCapsIss;
        pIsiSensor->pIsiSetupSensorIss              = OV2775_IsiSetupSensorIss;
        pIsiSensor->pIsiChangeSensorResolutionIss   = OV2775_IsiChangeSensorResolutionIss;
        pIsiSensor->pIsiSensorSetStreamingIss       = OV2775_IsiSensorSetStreamingIss;
        pIsiSensor->pIsiSensorSetPowerIss           = OV2775_IsiSensorSetPowerIss;
        pIsiSensor->pIsiCheckSensorConnectionIss    = OV2775_IsiCheckSensorConnectionIss;
        pIsiSensor->pIsiGetSensorRevisionIss        = OV2775_IsiGetSensorRevisionIss;
        pIsiSensor->pIsiRegisterReadIss             = OV2775_IsiRegisterReadIss;
        pIsiSensor->pIsiRegisterWriteIss            = OV2775_IsiRegisterWriteIss;

        /* AEC functions */
        pIsiSensor->pIsiExposureControlIss          = OV2775_IsiExposureControlIss;
        pIsiSensor->pIsiGetGainLimitsIss            = OV2775_IsiGetGainLimitsIss;
        pIsiSensor->pIsiGetIntegrationTimeLimitsIss = OV2775_IsiGetIntegrationTimeLimitsIss;
        pIsiSensor->pIsiGetCurrentExposureIss       = OV2775_IsiGetCurrentExposureIss;
        pIsiSensor->pIsiGetVSGainIss                    = OV2775_IsiGetVSGainIss;
        pIsiSensor->pIsiGetGainIss                      = OV2775_IsiGetGainIss;
        pIsiSensor->pIsiGetLongGainIss                  = OV2775_IsiGetLongGainIss;
        pIsiSensor->pIsiGetGainIncrementIss             = OV2775_IsiGetGainIncrementIss;
        pIsiSensor->pIsiSetGainIss                      = OV2775_IsiSetGainIss;
        pIsiSensor->pIsiGetIntegrationTimeIss           = OV2775_IsiGetIntegrationTimeIss;
        pIsiSensor->pIsiGetVSIntegrationTimeIss         = OV2775_IsiGetVSIntegrationTimeIss;
        pIsiSensor->pIsiGetLongIntegrationTimeIss       = OV2775_IsiGetLongIntegrationTimeIss;
        pIsiSensor->pIsiGetIntegrationTimeIncrementIss  = OV2775_IsiGetIntegrationTimeIncrementIss;
        pIsiSensor->pIsiSetIntegrationTimeIss           = OV2775_IsiSetIntegrationTimeIss;
        pIsiSensor->pIsiQuerySensorIss                  = OV2775_IsiQuerySensorIss;
        pIsiSensor->pIsiGetResolutionIss                = OV2775_IsiGetResolutionIss;
        pIsiSensor->pIsiGetSensorFpsIss                 = OV2775_IsiGetSensorFpsIss;
        pIsiSensor->pIsiSetSensorFpsIss                 = OV2775_IsiSetSensorFpsIss;
        pIsiSensor->pIsiSensorGetExpandCurveIss         = OV2775_IsiSensorGetExpandCurveIss;

        /* AWB specific functions */

        /* Testpattern */
        pIsiSensor->pIsiActivateTestPattern         = OV2775_IsiActivateTestPattern;
        pIsiSensor->pIsiEnableHdr                   = OV2775_IsiEnableHdr;
        pIsiSensor->pIsiSetBayerPattern             = OV2775_IsiSetBayerPattern;

        pIsiSensor->pIsiSensorSetBlcIss             = OV2775_IsiSensorSetBlcIss;
        pIsiSensor->pIsiSensorSetWBIss              = OV2775_IsiSensorSetWBIss;
        pIsiSensor->pIsiGetSensorAWBModeIss         = OV2775_IsiGetSensorAWBModeIss;

    } else {
        result = RET_NULL_POINTER;
    }

    TRACE( OV2775_INFO, "%s (exit)\n", __func__);
    return ( result );
}

/*****************************************************************************
* each sensor driver need declare this struct for isi load
*****************************************************************************/
IsiCamDrvConfig_t IsiCamDrvConfig = {
    0,
    OV2775_IsiQuerySensorSupportIss,
    OV2775_IsiGetSensorIss,
    {
     0,            /**< IsiSensor_t.pszName */
     0,            /**< IsiSensor_t.pIsiInitIss>*/
     0,              /**< IsiSensor_t.pIsiResetSensorIss>*/
     0,            /**< IsiSensor_t.pRegisterTable */
     0,            /**< IsiSensor_t.pIsiSensorCaps */
     0,            /**< IsiSensor_t.pIsiCreateSensorIss */
     0,            /**< IsiSensor_t.pIsiReleaseSensorIss */
     0,            /**< IsiSensor_t.pIsiGetCapsIss */
     0,            /**< IsiSensor_t.pIsiSetupSensorIss */
     0,            /**< IsiSensor_t.pIsiChangeSensorResolutionIss */
     0,            /**< IsiSensor_t.pIsiSensorSetStreamingIss */
     0,            /**< IsiSensor_t.pIsiSensorSetPowerIss */
     0,            /**< IsiSensor_t.pIsiCheckSensorConnectionIss */
     0,            /**< IsiSensor_t.pIsiGetSensorRevisionIss */
     0,            /**< IsiSensor_t.pIsiRegisterReadIss */
     0,            /**< IsiSensor_t.pIsiRegisterWriteIss */

     0,            /**< IsiSensor_t.pIsiExposureControlIss */
     0,            /**< IsiSensor_t.pIsiGetGainLimitsIss */
     0,            /**< IsiSensor_t.pIsiGetIntegrationTimeLimitsIss */
     0,            /**< IsiSensor_t.pIsiGetCurrentExposureIss */
     0,            /**< IsiSensor_t.pIsiGetGainIss */
     0,            /**< IsiSensor_t.pIsiGetVSGainIss */
     0,            /**< IsiSensor_t.pIsiGetGainIncrementIss */
     0,            /**< IsiSensor_t.pIsiGetGainIncrementIss */
     0,            /**< IsiSensor_t.pIsiSetGainIss */
     0,            /**< IsiSensor_t.pIsiGetIntegrationTimeIss */
     0,            /**< IsiSensor_t.pIsiGetIntegrationTimeIncrementIss */
     0,            /**< IsiSensor_t.pIsiSetIntegrationTimeIss */
     0,            /**< IsiSensor_t.pIsiGetResolutionIss */
     0,            /**< IsiSensor_t.pIsiGetAfpsInfoIss */

     0,            /**< IsiSensor_t.pIsiMdiInitMotoDriveMds */
     0,            /**< IsiSensor_t.pIsiMdiSetupMotoDrive */
     0,            /**< IsiSensor_t.pIsiMdiFocusSet */
     0,            /**< IsiSensor_t.pIsiMdiFocusGet */
     0,            /**< IsiSensor_t.pIsiMdiFocusCalibrate */
     0,            /**< IsiSensor_t.pIsiGetSensorMipiInfoIss */
     0,            /**< IsiSensor_t.pIsiActivateTestPattern */
     0,            /**< IsiSensor_t.pIsiEnableHdr */
     0,            /**< IsiSensor_t.pIsiSetBayerPattern */
     }
};
