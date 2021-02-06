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

#include "OS08a20_priv.h"

CREATE_TRACER( OS08a20_INFO , "OS08a20: ", INFO,    1);
CREATE_TRACER( OS08a20_WARN , "OS08a20: ", WARNING, 1);
CREATE_TRACER( OS08a20_ERROR, "OS08a20: ", ERROR,   1);
CREATE_TRACER( OS08a20_DEBUG,     "OS08a20: ", INFO, 1);
CREATE_TRACER( OS08a20_REG_INFO , "OS08a20: ", INFO, 1);
CREATE_TRACER( OS08a20_REG_DEBUG, "OS08a20: ", INFO, 1);

#ifdef SUBDEV_V4L2
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#undef TRACE
#define TRACE(x, ...)
#endif

#define OS08a20_MIN_GAIN_STEP    ( 1.0f/16.0f )  /**< min gain step size used by GUI (hardware min = 1/16; 1/16..32/16 depending on actual gain ) */
#define OS08a20_MAX_GAIN_AEC     ( 32.0f )       /**< max. gain used by the AEC (arbitrarily chosen, hardware limit = 62.0, driver limit = 32.0 ) */
#define OS08a20_VS_MAX_INTEGRATION_TIME (0.0018)

/*****************************************************************************
 *Sensor Info
*****************************************************************************/
static const char SensorName[16] = "OS08a20";

static struct vvcam_mode_info pos08a20_mode_info[] = {
    {
		.index     = 0,
        .width     = 1920,
        .height    = 1080,
        .fps       = 30,
        .hdr_mode  = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_GBRG,
    },
    {
		.index     = 1,
        .width    = 1920,
        .height   = 1080,
        .fps      = 30,
        .hdr_mode = SENSOR_MODE_HDR_STITCH,
        .stitching_mode = SENSOR_STITCHING_3DOL,
        .bit_width = 10,
        .bayer_pattern = BAYER_GBRG,
    },
    {
		.index     = 2,
        .width    = 3840,
        .height   = 2160,
        .fps      = 40,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_GBRG,
    },
    {
        .index     = 3,
	    .width    = 3840,
        .height   = 2160,
        .fps      = 40,
        .hdr_mode = SENSOR_MODE_HDR_STITCH,
        .bit_width = 12,
        .bayer_pattern = BAYER_GBRG,

    },
};

static RESULT OS08a20_IsiSensorSetPowerIss(IsiSensorHandle_t handle, bool_t on) {
    RESULT result = RET_SUCCESS;

    int ret = 0;
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    int32_t enable = on;
    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_POWER, &enable);
    if (ret != 0) {
        TRACE(OS08a20_ERROR, "%s: sensor set power error!\n", __func__);
        return (RET_FAILURE);
    }

    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiResetSensorIss(IsiSensorHandle_t handle) {
    RESULT result = RET_SUCCESS;
    int ret = 0;

    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_RESET, NULL);
    if (ret != 0) {
        TRACE(OS08a20_ERROR, "%s: sensor reset error!\n", __func__);
        return (RET_FAILURE);
    }

    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

#ifdef SUBDEV_CHAR
static RESULT OS08a20_IsiSensorSetClkIss(IsiSensorHandle_t handle, uint32_t clk) {
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_CLK, &clk);
    if (ret != 0) {
        TRACE(OS08a20_ERROR, "%s: sensor set clk error!\n", __func__);
        return (RET_FAILURE);
    }

    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiSensorGetClkIss
    (IsiSensorHandle_t handle, uint32_t * pclk) {
    RESULT result = RET_SUCCESS;
    int ret = 0;

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_CLK, pclk);
    if (ret != 0) {
        TRACE(OS08a20_ERROR, "%s: sensor get clk error!\n", __func__);
        return (RET_FAILURE);
    }

    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiConfigSensorSCCBIss(IsiSensorHandle_t handle)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;
    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

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
        TRACE(OS08a20_ERROR, "%s: sensor config sccb info error!\n",
              __func__);
        return (RET_FAILURE);
    }

    TRACE(OS08a20_INFO, "%s (exit) result = %d\n", __func__, result);
    return (result);
}
#endif

static RESULT OS08a20_IsiRegisterReadIss
    (IsiSensorHandle_t handle, const uint32_t address, uint32_t * p_value) {
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    struct vvcam_sccb_data sccb_data;
    sccb_data.addr = address;
    sccb_data.data = 0;
    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_READ_REG, &sccb_data);
    if (ret != 0) {
        TRACE(OS08a20_ERROR, "%s: read sensor register error!\n",
              __func__);
        return (RET_FAILURE);
    }

    *p_value = sccb_data.data;

    TRACE(OS08a20_INFO, "%s (exit) result = %d\n", __func__, result);
    return (result);
}

static RESULT OS08a20_IsiRegisterWriteIss
    (IsiSensorHandle_t handle, const uint32_t address, const uint32_t value) {
    RESULT result = RET_SUCCESS;
    int ret = 0;
    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    struct vvcam_sccb_data sccb_data;
    sccb_data.addr = address;
    sccb_data.data = value;

    ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_WRITE_REG, &sccb_data);
    if (ret != 0) {
        TRACE(OS08a20_ERROR, "%s: write sensor register error!\n",
              __func__);
        return (RET_FAILURE);
    }

    TRACE(OS08a20_INFO, "%s (exit) result = %d\n", __func__, result);
    return (result);
}

static RESULT OS08a20_IsiQuerySensorSupportIss(HalHandle_t  HalHandle, vvcam_mode_info_array_t *pSensorSupportInfo)
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
        TRACE(OS08a20_ERROR, "%s: sensor kernel query error! Use isi query\n",__func__);
        psensor_mode_info_arry->count = sizeof(pos08a20_mode_info) / sizeof(struct vvcam_mode_info);
        memcpy(psensor_mode_info_arry->modes, pos08a20_mode_info, sizeof(pos08a20_mode_info));
    }

    return RET_SUCCESS;
}

static  RESULT OS08a20_IsiQuerySensorIss(IsiSensorHandle_t handle, vvcam_mode_info_array_t *pSensorInfo)
{
    RESULT result = RET_SUCCESS;
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;
    OS08a20_IsiQuerySensorSupportIss(pHalCtx,pSensorInfo);

    return result;
}

static RESULT OS08a20_IsiGetSensorModeIss(IsiSensorHandle_t handle,void *mode)
{
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL) {
        return (RET_WRONG_HANDLE);
    }
    memcpy(mode,&(pOS08a20Ctx->SensorMode), sizeof(pOS08a20Ctx->SensorMode));

    return ( RET_SUCCESS );
}

static RESULT OS08a20_IsiCreateSensorIss(IsiSensorInstanceConfig_t * pConfig) {
    RESULT result = RET_SUCCESS;
    OS08a20_Context_t *pOS08a20Ctx;

    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    if (!pConfig || !pConfig->pSensor)
        return (RET_NULL_POINTER);

    pOS08a20Ctx = (OS08a20_Context_t *) malloc(sizeof(OS08a20_Context_t));
    if (!pOS08a20Ctx) {
        TRACE(OS08a20_ERROR, "%s: Can't allocate os08a20 context\n",
              __func__);
        return (RET_OUTOFMEM);
    }

    MEMSET(pOS08a20Ctx, 0, sizeof(OS08a20_Context_t));

    result = HalAddRef(pConfig->HalHandle);
    if (result != RET_SUCCESS) {
        free(pOS08a20Ctx);
        return (result);
    }

    pOS08a20Ctx->IsiCtx.HalHandle = pConfig->HalHandle;
    pOS08a20Ctx->IsiCtx.pSensor = pConfig->pSensor;
    pOS08a20Ctx->GroupHold = BOOL_FALSE;
    pOS08a20Ctx->OldGain = 0;
    pOS08a20Ctx->OldIntegrationTime = 0;
    pOS08a20Ctx->Configured = BOOL_FALSE;
    pOS08a20Ctx->Streaming = BOOL_FALSE;
    pOS08a20Ctx->TestPattern = BOOL_FALSE;
    pOS08a20Ctx->isAfpsRun = BOOL_FALSE;
    pOS08a20Ctx->SensorMode.index = pConfig->SensorModeIndex;
    pConfig->hSensor = (IsiSensorHandle_t) pOS08a20Ctx;
#ifdef SUBDEV_CHAR

    switch(pOS08a20Ctx->SensorMode.index)
    {
        case 0:
            memcpy(pOS08a20Ctx->SensorRegCfgFile,
               "OS08a20_mipi4lane_1080p_init.txt",
               strlen("OS08a20_mipi4lane_1080p_init.txt"));
            break;
        case 1:
            memcpy(pOS08a20Ctx->SensorRegCfgFile,
                   "OS08a20_mipi4lane_1080p_2dol_init.txt",
                   strlen("OS08a20_mipi4lane_1080p_2dol_init.txt"));
            break;
        case 2:
            memcpy(pOS08a20Ctx->SensorRegCfgFile,
                   "OS08a20_mipi4lane_4k_2dol_init.txt",
                   strlen("OS08a20_mipi4lane_4k_2dol_init.txt"));
            break;
        case 3:
            memcpy(pOS08a20Ctx->SensorRegCfgFile,
                   "OS08a20_mipi4lane_4k_2dol_init.txt",
                   strlen("OS08a20_mipi4lane_4k_2dol_init.txt"));
            break;
        default:
            TRACE(OS08a20_INFO, "%s:not support sensor mode %d\n", __func__,pOS08a20Ctx->SensorMode.index);
            return RET_NOTSUPP;
            break;
    }

    if (access(pOS08a20Ctx->SensorRegCfgFile, F_OK) == 0) {
        pOS08a20Ctx->KernelDriverFlag = 0;
    } else {
        pOS08a20Ctx->KernelDriverFlag = 1;
    }

    result = OS08a20_IsiSensorSetPowerIss(pOS08a20Ctx, BOOL_TRUE);
    RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);

    uint32_t SensorClkIn;
    if (pOS08a20Ctx->KernelDriverFlag) {
        result = OS08a20_IsiSensorGetClkIss(pOS08a20Ctx, &SensorClkIn);
        RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);
    }

    result = OS08a20_IsiSensorSetClkIss(pOS08a20Ctx, SensorClkIn);
    RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);

    result = OS08a20_IsiResetSensorIss(pOS08a20Ctx);
    RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);

    if (!pOS08a20Ctx->KernelDriverFlag) {
        result = OS08a20_IsiConfigSensorSCCBIss(pOS08a20Ctx);
        RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);
    }

    pOS08a20Ctx->pattern = ISI_BPAT_GBGBRGRG;
#endif

#ifdef SUBDEV_V4L2
    pOS08a20Ctx->pattern = ISI_BPAT_BGBGGRGR;
    pOS08a20Ctx->subdev = HalGetFdHandle(pConfig->HalHandle, HAL_MODULE_SENSOR);//two sensors??
    //HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;
    //pHalCtx->sensor_fd = pOS08a20Ctx->subdev;
    pOS08a20Ctx->KernelDriverFlag = 1;
#endif
    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiGetRegCfgIss(const char *registerFileName,
                     struct vvcam_sccb_array *arry)
{
    if (NULL == registerFileName) {
        TRACE(OS08a20_ERROR, "%s:registerFileName is NULL\n", __func__);
        return (RET_NULL_POINTER);
    }
#ifdef SUBDEV_CHAR
    FILE *fp = NULL;
    fp = fopen(registerFileName, "rb");
    if (!fp) {
        TRACE(OS08a20_ERROR, "%s:load register file  %s error!\n",
              __func__, registerFileName);
        return (RET_FAILURE);
    }

    char LineBuf[512];
    unsigned int FileTotalLine = 0;
    while (!feof(fp)) {
        fgets(LineBuf, 512, fp);
        FileTotalLine++;
    }

    arry->sccb_data =
        malloc(FileTotalLine * sizeof(struct vvcam_sccb_data));
    if (arry->sccb_data == NULL) {
        TRACE(OS08a20_ERROR, "%s:malloc failed NULL Point!\n", __func__,
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

static RESULT OS08a20_IsiInitSensorIss(IsiSensorHandle_t handle) {
    RESULT result = RET_SUCCESS;
    int ret = 0;
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;

    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);
    if (pOS08a20Ctx == NULL) {
        return (RET_WRONG_HANDLE);
    }
    if (pOS08a20Ctx->KernelDriverFlag) {
#ifdef SUBDEV_CHAR
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_INIT, &(pOS08a20Ctx->SensorMode));
        if (ret != 0) {
            TRACE(OS08a20_ERROR, "%s:sensor init error!\n",
                  __func__);
            return (RET_FAILURE);
        }
        struct vvcam_ae_info_s ae_info;
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_AE_INFO, &ae_info);
        if (ret != 0) {
            TRACE(OS08a20_ERROR, "%s:sensor get ae info error!\n",
                  __func__);
            return (RET_FAILURE);
        }
        pOS08a20Ctx->one_line_exp_time =
            (float)ae_info.one_line_exp_time_ns / 1000000000;
        pOS08a20Ctx->MaxIntegrationLine = ae_info.max_interrgation_time;
        pOS08a20Ctx->MinIntegrationLine = ae_info.min_interrgation_time;
        pOS08a20Ctx->gain_accuracy = ae_info.gain_accuracy;
        pOS08a20Ctx->AecMinGain = (float)(ae_info.min_gain) / ae_info.gain_accuracy;
        pOS08a20Ctx->AecMaxGain = (float)(ae_info.max_gain) / ae_info.gain_accuracy;
        pOS08a20Ctx->CurrFps = pOS08a20Ctx->MaxFps;
#endif

#ifdef SUBDEV_V4L2
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_SENSOR_MODE, &(pOS08a20Ctx->SensorMode));
        if (ret != 0) {
            TRACE(OS08a20_ERROR, "%s:sensor get mode info error!\n",
                  __func__);
            return (RET_FAILURE);
        }
        if (pOS08a20Ctx->SensorMode.hdr_mode != SENSOR_MODE_LINEAR)
        {
            pOS08a20Ctx->enableHdr = 1;
        }else
        {
             pOS08a20Ctx->enableHdr = 0;
        }
        
        pOS08a20Ctx->one_line_exp_time  = (float) (pOS08a20Ctx->SensorMode.ae_info.one_line_exp_time_ns) / 1000000000;
        pOS08a20Ctx->MaxIntegrationLine = pOS08a20Ctx->SensorMode.ae_info.max_interrgation_time;
        pOS08a20Ctx->MinIntegrationLine = pOS08a20Ctx->SensorMode.ae_info.min_interrgation_time;
        pOS08a20Ctx->gain_accuracy      = pOS08a20Ctx->SensorMode.ae_info.gain_accuracy;
        pOS08a20Ctx->AecMaxGain         = (float)(pOS08a20Ctx->SensorMode.ae_info.max_gain) /pOS08a20Ctx->gain_accuracy ;
        pOS08a20Ctx->AecMinGain         = (float)(pOS08a20Ctx->SensorMode.ae_info.min_gain) / pOS08a20Ctx->gain_accuracy ;
        pOS08a20Ctx->MaxFps             = pOS08a20Ctx->SensorMode.fps;
        pOS08a20Ctx->CurrFps            = pOS08a20Ctx->MaxFps;

#endif

    } else {
        struct vvcam_sccb_array arry;
        result = OS08a20_IsiGetRegCfgIss(pOS08a20Ctx->SensorRegCfgFile, &arry);
        if (result != 0) {
            TRACE(OS08a20_ERROR,
                  "%s:OS08a20_IsiGetRegCfgIss error!\n", __func__);
            return (RET_FAILURE);
        }

        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_WRITE_ARRAY, &arry);
        if (ret != 0) {
            TRACE(OS08a20_ERROR, "%s:Sensor Write Reg arry error!\n",
                  __func__);
            return (RET_FAILURE);
        }

        switch(pOS08a20Ctx->SensorMode.index)
        {
            case 0:
                pOS08a20Ctx->one_line_exp_time = 0.000069607;
                pOS08a20Ctx->FrameLengthLines = 0x870;
                pOS08a20Ctx->CurFrameLengthLines = pOS08a20Ctx->FrameLengthLines;
                pOS08a20Ctx->MaxIntegrationLine = pOS08a20Ctx->CurFrameLengthLines - 3;
                pOS08a20Ctx->MinIntegrationLine = 1;
                pOS08a20Ctx->AecMaxGain = 24;
                pOS08a20Ctx->AecMinGain = 3;
                pOS08a20Ctx->CurrFps = pOS08a20Ctx->MaxFps;
                break;
            case 1:
                pOS08a20Ctx->one_line_exp_time = 0.000069607;
                pOS08a20Ctx->FrameLengthLines = 0x870;
                pOS08a20Ctx->CurFrameLengthLines = pOS08a20Ctx->FrameLengthLines;
                pOS08a20Ctx->MaxIntegrationLine = pOS08a20Ctx->CurFrameLengthLines - 3;
                pOS08a20Ctx->MinIntegrationLine = 1;
                pOS08a20Ctx->AecMaxGain = 24;
                pOS08a20Ctx->AecMinGain = 3;
                pOS08a20Ctx->CurrFps = pOS08a20Ctx->MaxFps;
                break;
            case 2:
                break;
            case 3:
                break;
            default:
                TRACE(OS08a20_INFO, "%s:not support sensor mode %d\n", __func__,pOS08a20Ctx->SensorMode.index);
                return RET_NOTSUPP;
                break;
        }
    }

    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiReleaseSensorIss(IsiSensorHandle_t handle) {
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    if (pOS08a20Ctx == NULL)
        return (RET_WRONG_HANDLE);

    (void)OS08a20_IsiSensorSetStreamingIss(pOS08a20Ctx, BOOL_FALSE);
    (void)OS08a20_IsiSensorSetPowerIss(pOS08a20Ctx, BOOL_FALSE);
    (void)HalDelRef(pOS08a20Ctx->IsiCtx.HalHandle);

    MEMSET(pOS08a20Ctx, 0, sizeof(OS08a20_Context_t));
    free(pOS08a20Ctx);
    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiGetCapsIss
    (IsiSensorHandle_t handle, IsiSensorCaps_t * pIsiSensorCaps) {
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;

    RESULT result = RET_SUCCESS;

    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    if (pOS08a20Ctx == NULL)
        return (RET_WRONG_HANDLE);

    if (pIsiSensorCaps == NULL) {
        return (RET_NULL_POINTER);
    } 
    
    pIsiSensorCaps->BusWidth          = pOS08a20Ctx->SensorMode.bit_width;
    pIsiSensorCaps->Mode              = ISI_MODE_BAYER;
    pIsiSensorCaps->FieldSelection    = ISI_FIELDSEL_BOTH;
    pIsiSensorCaps->YCSequence        = ISI_YCSEQ_YCBYCR;    
    pIsiSensorCaps->Conv422           = ISI_CONV422_NOCOSITED;    
    pIsiSensorCaps->BPat              = pOS08a20Ctx->SensorMode.bayer_pattern;
    pIsiSensorCaps->HPol              = ISI_HPOL_REFPOS;
    pIsiSensorCaps->VPol              = ISI_VPOL_NEG;   
    pIsiSensorCaps->Edge              = ISI_EDGE_RISING;    
    pIsiSensorCaps->Resolution.width  = pOS08a20Ctx->SensorMode.width;
    pIsiSensorCaps->Resolution.height = pOS08a20Ctx->SensorMode.height;
    pIsiSensorCaps->SmiaMode          = ISI_SMIA_OFF;   
    pIsiSensorCaps->MipiLanes         = ISI_MIPI_4LANES;
    pIsiSensorCaps->enableHdr         = pOS08a20Ctx->enableHdr;

    if (pIsiSensorCaps->BusWidth == 10) {
        pIsiSensorCaps->MipiMode      = ISI_MIPI_MODE_RAW_10;
    }else if (pIsiSensorCaps->BusWidth == 12){
        pIsiSensorCaps->MipiMode      = ISI_MIPI_MODE_RAW_12;
    }else{
        pIsiSensorCaps->MipiMode      = ISI_MIPI_OFF;
    }

    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_AecSetModeParameters
    (OS08a20_Context_t * pOS08a20Ctx, const IsiSensorConfig_t * pConfig) {
    RESULT result = RET_SUCCESS;
    TRACE(OS08a20_INFO, "%s%s: (enter)\n", __func__,
          pOS08a20Ctx->isAfpsRun ? "(AFPS)" : "");

    pOS08a20Ctx->AecIntegrationTimeIncrement = pOS08a20Ctx->one_line_exp_time;
    pOS08a20Ctx->AecMinIntegrationTime =
        pOS08a20Ctx->one_line_exp_time * pOS08a20Ctx->MinIntegrationLine;
    pOS08a20Ctx->AecMaxIntegrationTime =
        pOS08a20Ctx->one_line_exp_time * pOS08a20Ctx->MaxIntegrationLine;

    TRACE(OS08a20_DEBUG, "%s%s: AecMaxIntegrationTime = %f \n", __func__,
          pOS08a20Ctx->isAfpsRun ? "(AFPS)" : "",
          pOS08a20Ctx->AecMaxIntegrationTime);

    pOS08a20Ctx->AecGainIncrement = OS08a20_MIN_GAIN_STEP;

    //reflects the state of the sensor registers, must equal default settings
    pOS08a20Ctx->AecCurGain = pOS08a20Ctx->AecMinGain;
    pOS08a20Ctx->AecCurIntegrationTime = 0.0f;
    pOS08a20Ctx->OldGain = 0;
    pOS08a20Ctx->OldIntegrationTime = 0;

    TRACE(OS08a20_INFO, "%s%s: (exit)\n", __func__,
          pOS08a20Ctx->isAfpsRun ? "(AFPS)" : "");

    return (result);
}

#ifdef SUBDEV_V4L2
RESULT OS08a20_Private_SetFormat(IsiSensorHandle_t handle, int width, int height, bool hdrEnable)
{
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;

    ioctl(pOS08a20Ctx->subdev, VVSENSORIOC_S_HDR_MODE, &hdrEnable);

    struct v4l2_subdev_format format;
    format.format.width = width;
    format.format.height = height;
    format.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    format.pad = 0;
    int rc = ioctl(pOS08a20Ctx->subdev, VIDIOC_SUBDEV_S_FMT, &format);
    return rc == 0 ? RET_SUCCESS : RET_FAILURE;
}
#endif

static RESULT OS08a20_IsiSetupSensorIss
    (IsiSensorHandle_t handle, const IsiSensorConfig_t * pConfig) {
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    if (!pOS08a20Ctx) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pConfig) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid configuration (NULL pointer detected)\n",
              __func__);
        return (RET_NULL_POINTER);
    }

    if (pOS08a20Ctx->Streaming != BOOL_FALSE) {
        return RET_WRONG_STATE;
    }

    memcpy(&pOS08a20Ctx->Config, pConfig, sizeof(IsiSensorConfig_t));

    /* 1.) SW reset of image sensor (via I2C register interface)  be careful, bits 6..0 are reserved, reset bit is not sticky */
    TRACE(OS08a20_DEBUG, "%s: OS08a20 System-Reset executed\n", __func__);
    osSleep(100);

    result = OS08a20_AecSetModeParameters(pOS08a20Ctx, pConfig);
    if (result != RET_SUCCESS) {
        TRACE(OS08a20_ERROR, "%s: SetupOutputWindow failed.\n",
              __func__);
        return (result);
    }
#ifdef SUBDEV_V4L2
    OS08a20_Private_SetFormat(pOS08a20Ctx,
                            pOS08a20Ctx->SensorMode.width,
                            pOS08a20Ctx->SensorMode.height,
                            pOS08a20Ctx->SensorMode.hdr_mode);
#endif

    pOS08a20Ctx->Configured = BOOL_TRUE;
    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return 0;
}

static RESULT OS08a20_IsiChangeSensorResolutionIss(IsiSensorHandle_t handle, uint16_t width, uint16_t height) {
    RESULT result = RET_SUCCESS;
    
    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiSensorSetStreamingIss
    (IsiSensorHandle_t handle, bool_t on) {
    RESULT result = RET_SUCCESS;
    int ret = 0;
    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    if ((pOS08a20Ctx->Configured != BOOL_TRUE)
        || (pOS08a20Ctx->Streaming == on))
        return RET_WRONG_STATE;

    int32_t enable = (uint32_t) on;
    if (pOS08a20Ctx->KernelDriverFlag) {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_STREAM, &enable);

    } else {
        ret = OS08a20_IsiRegisterWriteIss(handle, 0x3012, on);
    }

    if (ret != 0) {
        return (RET_FAILURE);
    }

    pOS08a20Ctx->Streaming = on;

    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiCheckSensorConnectionIss(IsiSensorHandle_t handle) {
    RESULT result = RET_SUCCESS;
    int ret = 0;

    uint32_t sensor_id = 0;
    uint32_t correct_id = 0x5308;

    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    if (pOS08a20Ctx->KernelDriverFlag) {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_RESERVE_ID,
              &correct_id);
        if (ret != 0) {
            TRACE(OS08a20_ERROR,
                  "%s: Read Sensor reserve ID Error! \n", __func__);
            return (RET_FAILURE);
        }

        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_CHIP_ID,
              &sensor_id);
        if (ret != 0) {
            TRACE(OS08a20_ERROR,
                  "%s: Read Sensor chip ID Error! \n", __func__);
            return (RET_FAILURE);
        }
    } else {

        result = OS08a20_IsiGetSensorRevisionIss(handle, &sensor_id);
        if (result != RET_SUCCESS) {
            TRACE(OS08a20_ERROR, "%s: Read Sensor ID Error! \n",
                  __func__);
            return (RET_FAILURE);
        }
    }

    if (correct_id != sensor_id) {
        TRACE(OS08a20_ERROR, "%s:ChipID =0x%x sensor_id=%x error! \n",
              __func__, correct_id, sensor_id);
        return (RET_FAILURE);
    }

    TRACE(OS08a20_INFO,
          "%s ChipID = 0x%08x, sensor_id = 0x%08x, success! \n", __func__,
          correct_id, sensor_id);
    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiGetSensorRevisionIss
    (IsiSensorHandle_t handle, uint32_t * p_value) {
    RESULT result = RET_SUCCESS;
    int ret = 0;
    uint32_t reg_val;
    uint32_t sensor_id;

    TRACE(OS08a20_INFO, "%s (enter)\n", __func__);

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    if (!p_value)
        return (RET_NULL_POINTER);

    if (pOS08a20Ctx->KernelDriverFlag) {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_CHIP_ID,
              &sensor_id);
        if (ret != 0) {
            TRACE(OS08a20_ERROR, "%s: Read Sensor ID Error! \n",
                  __func__);
            return (RET_FAILURE);
        }
    } else {
        reg_val = 0;
        result = OS08a20_IsiRegisterReadIss(handle, 0x300a, &reg_val);
        sensor_id = (reg_val & 0xff) << 8;

        reg_val = 0;
        result |= OS08a20_IsiRegisterReadIss(handle, 0x300b, &reg_val);
        sensor_id |= (reg_val & 0xff);

    }

    *p_value = sensor_id;
    TRACE(OS08a20_INFO, "%s (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiGetGainLimitsIss
    (IsiSensorHandle_t handle, float *pMinGain, float *pMaxGain) {
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (pOS08a20Ctx == NULL) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if ((pMinGain == NULL) || (pMaxGain == NULL)) {
        TRACE(OS08a20_ERROR, "%s: NULL pointer received!!\n");
        return (RET_NULL_POINTER);
    }

    *pMinGain = pOS08a20Ctx->AecMinGain;
    *pMaxGain = pOS08a20Ctx->AecMaxGain;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiGetIntegrationTimeLimitsIss
    (IsiSensorHandle_t handle,
     float *pMinIntegrationTime, float *pMaxIntegrationTime) {
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);
    if (pOS08a20Ctx == NULL) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if ((pMinIntegrationTime == NULL) || (pMaxIntegrationTime == NULL)) {
        TRACE(OS08a20_ERROR, "%s: NULL pointer received!!\n");
        return (RET_NULL_POINTER);
    }

    *pMinIntegrationTime = pOS08a20Ctx->AecMinIntegrationTime;
    *pMaxIntegrationTime = pOS08a20Ctx->AecMaxIntegrationTime;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);
    return (result);
}

RESULT OS08a20_IsiGetGainIss(IsiSensorHandle_t handle, float *pSetGain) {
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (pOS08a20Ctx == NULL) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (pSetGain == NULL) {
        return (RET_NULL_POINTER);
    }
    *pSetGain = pOS08a20Ctx->AecCurGain;

    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OS08a20_IsiGetVSGainIss(IsiSensorHandle_t handle, float *pSetGain) {
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (pOS08a20Ctx == NULL) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (pSetGain == NULL) {
        return (RET_NULL_POINTER);
    }

    *pSetGain = pOS08a20Ctx->AecCurVSGain;

    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);

    return (result);
}

RESULT OS08a20_IsiGetGainIncrementIss(IsiSensorHandle_t handle, float *pIncr) {
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (pOS08a20Ctx == NULL) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (pIncr == NULL)
        return (RET_NULL_POINTER);

    *pIncr = pOS08a20Ctx->AecGainIncrement;

    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);

    return (result);
}

RESULT OS08a20_IsiSetGainIss
    (IsiSensorHandle_t handle,
     float NewGain, float *pSetGain, float *hdr_ratio) {

    RESULT result = RET_SUCCESS;
    int32_t ret = 0;

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    if (pOS08a20Ctx->KernelDriverFlag) {
        uint32_t SensorGain = 0;
        SensorGain = NewGain * pOS08a20Ctx->gain_accuracy;
        ret |= ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_GAIN, &SensorGain);
        if (ret != 0) {
            TRACE(OS08a20_ERROR, "%s: set sensor gain error\n",
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
        TRACE(OS08a20_INFO, "%s: (Linear mode enter)\n", __func__);

        if ((pSetGain == NULL) || (hdr_ratio == NULL))
            return (RET_NULL_POINTER);

        if (NewGain < pOS08a20Ctx->AecMinGain)
            NewGain = pOS08a20Ctx->AecMinGain;
        if (NewGain > pOS08a20Ctx->AecMaxGain)
            NewGain = pOS08a20Ctx->AecMaxGain;
        if (pOS08a20Ctx->enableHdr == true) {
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
            OS08a20_IsiRegisterReadIss(handle, 0x30bb, &data);
            data = (data & againHcg);
            data = (data & (((againLcg << 2) & 0x0c)));
            if ((dGainHcg != pOS08a20Ctx->OldGainHcg)
                || (againHcg != pOS08a20Ctx->OldAGainHcg)) {
                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x3467,
                                   0x00);
                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x3464,
                                   0x04);

                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x315a,
                                   (gainHcg >> 8) &
                                   0xff);
                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x355b,
                                   gainHcg & 0xff);

                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x315c,
                                   (gainLcg >> 8) &
                                   0xff);
                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x315d,
                                   gainLcg & 0xff);

                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x30bb,
                                   data);

                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x3464,
                                   0x14);
                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x3467,
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

            OS08a20_IsiRegisterReadIss(handle, 0x30bb, &data);
            data = (data & (againHcg));

            if ((dGainHcg != pOS08a20Ctx->OldGainHcg)
                || (againHcg != pOS08a20Ctx->OldAGainHcg)) {

                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x3467,
                                   0x00);
                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x3464,
                                   0x04);

                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x315a,
                                   ((gainHcg >> 8) &
                                0x00ff));
                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x315b,
                                   (gainHcg &
                                0x00ff));

                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x30bb,
                                   data);

                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x3464,
                                   0x14);
                result =
                    OS08a20_IsiRegisterWriteIss(handle, 0x3467,
                                   0x01);

            }
            if ((dGainHcg != pOS08a20Ctx->OldGainHcg)
                || (againHcg != pOS08a20Ctx->OldAGainHcg)) {

                pOS08a20Ctx->OldGainHcg = dGainHcg;
                pOS08a20Ctx->OldAGainHcg = againHcg;
                pOS08a20Ctx->OldAGainLcg = dGainLcg;
            }
        }

    }

    pOS08a20Ctx->AecCurGain = ((float)(NewGain));

    *pSetGain = pOS08a20Ctx->AecCurGain;
    TRACE(OS08a20_DEBUG, "%s: g=%f\n", __func__, *pSetGain);
    return (result);
}

RESULT OS08a20_IsiSetVSGainIss
    (IsiSensorHandle_t handle,
     float NewIntegrationTime,
     float NewGain, float *pSetGain, float *hdr_ratio) {
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;
    RESULT result = RET_SUCCESS;

    float Gain = 0.0f;

    uint32_t ucGain = 0U;
    uint32_t again = 0U;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (!pOS08a20Ctx) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pSetGain || !hdr_ratio)
        return (RET_NULL_POINTER);

    if (pOS08a20Ctx->KernelDriverFlag) {
        uint32_t SensorGain = 0;
        SensorGain = NewGain * pOS08a20Ctx->gain_accuracy;
        ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_VSGAIN, &SensorGain);
    } else {
        Gain = NewGain / 2.0;
        if (Gain < 1.0)
            Gain = 1.0;
        ucGain = (uint32_t) (Gain * 256);

        TRACE(OS08a20_INFO,
              "%s: set Gain ucGainvs:=0x%03x NewIntegrationTime = %f,NewGain=%f\n",
              __func__, ucGain, NewIntegrationTime, NewGain);

        if (pOS08a20Ctx->OldVsGain != ucGain) {

            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3467, 0x00);
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3464, 0x04);

            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x315e,
                               (ucGain >> 8) & 0xff);
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x315f,
                               ucGain & 0xff);

            result =
                OS08a20_IsiRegisterReadIss(handle, 0x30bb, &again);
            again &= 0x3f;
            again |= 8;
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x30bb, again);

            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3464, 0x14);
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3467, 0x01);

            pOS08a20Ctx->OldVsGain = ucGain;
        }
    }

    pOS08a20Ctx->AecCurVSGain = NewGain;
    *pSetGain = pOS08a20Ctx->AecCurGain;
    TRACE(OS08a20_DEBUG, "%s: g=%f\n", __func__, *pSetGain);
    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OS08a20_IsiSetBayerPattern(IsiSensorHandle_t handle, uint8_t pattern)
{

    RESULT result = RET_SUCCESS;
    uint8_t h_shift = 0, v_shift = 0;
    uint32_t val_h = 0, val_l = 0;
    uint16_t val = 0;
    uint8_t Start_p = 0;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    // pattern 0:B 1:GB 2:GR 3:R
    result = OS08a20_IsiSensorSetStreamingIss(handle, 0);
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

    OS08a20_IsiRegisterReadIss(handle, 0x30a0, &val_h);
    OS08a20_IsiRegisterReadIss(handle, 0x30a1, &val_l);
    val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + h_shift;
    val_h = (val >> 8) & 0xff;
    val_l = val & 0xff;
    OS08a20_IsiRegisterWriteIss(handle, 0x30a0, (uint8_t)val_h);
    OS08a20_IsiRegisterWriteIss(handle, 0x30a1, (uint8_t)val_l);

    OS08a20_IsiRegisterReadIss(handle, 0x30a2, &val_h);
    OS08a20_IsiRegisterReadIss(handle, 0x30a3, &val_l);
    val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + v_shift;
    val_h = (val >> 8) & 0xff;
    val_l = val & 0xff;
    OS08a20_IsiRegisterWriteIss(handle, 0x30a2, (uint8_t)val_h);
    OS08a20_IsiRegisterWriteIss(handle, 0x30a3, (uint8_t)val_l);

    OS08a20_IsiRegisterReadIss(handle, 0x30a4, &val_h);
    OS08a20_IsiRegisterReadIss(handle, 0x30a5, &val_l);
    val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + h_shift;
    val_h = (val >> 8) & 0xff;
    val_l = val & 0xff;
    OS08a20_IsiRegisterWriteIss(handle, 0x30a4, (uint8_t)val_h);
    OS08a20_IsiRegisterWriteIss(handle, 0x30a5, (uint8_t)val_l);

    OS08a20_IsiRegisterReadIss(handle, 0x30a6, &val_h);
    OS08a20_IsiRegisterReadIss(handle, 0x30a7, &val_l);
    val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + v_shift;
    val_h = (val >> 8) & 0xff;
    val_l = val & 0xff;
    OS08a20_IsiRegisterWriteIss(handle, 0x30a6, (uint8_t)val_h);
    OS08a20_IsiRegisterWriteIss(handle, 0x30a7, (uint8_t)val_l);

    pOS08a20Ctx->pattern = pattern;
    result = OS08a20_IsiSensorSetStreamingIss(handle, pOS08a20Ctx->Streaming);

    return (result);
}

RESULT OS08a20_IsiGetIntegrationTimeIss
    (IsiSensorHandle_t handle, float *pSetIntegrationTime)
{
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (!pOS08a20Ctx) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pSetIntegrationTime)
        return (RET_NULL_POINTER);
    *pSetIntegrationTime = pOS08a20Ctx->AecCurIntegrationTime;
    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OS08a20_IsiGetVSIntegrationTimeIss
    (IsiSensorHandle_t handle, float *pSetIntegrationTime)
{
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (!pOS08a20Ctx) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }
    if (!pSetIntegrationTime)
        return (RET_NULL_POINTER);

    *pSetIntegrationTime = pOS08a20Ctx->AecCurVSIntegrationTime;
    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OS08a20_IsiGetIntegrationTimeIncrementIss
    (IsiSensorHandle_t handle, float *pIncr)
{
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (!pOS08a20Ctx) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pIncr)
        return (RET_NULL_POINTER);

    //_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
    *pIncr = pOS08a20Ctx->AecIntegrationTimeIncrement;
    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OS08a20_IsiSetIntegrationTimeIss
    (IsiSensorHandle_t handle,
     float NewIntegrationTime,
     float *pSetIntegrationTime,
     uint8_t * pNumberOfFramesToSkip, float *hdr_ratio)
{
    RESULT result = RET_SUCCESS;

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    uint32_t exp_line = 0;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (!pOS08a20Ctx) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pSetIntegrationTime || !pNumberOfFramesToSkip) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid parameter (NULL pointer detected)\n",
              __func__);
        return (RET_NULL_POINTER);
    }

    exp_line = NewIntegrationTime / pOS08a20Ctx->one_line_exp_time;
    exp_line =
        MIN(pOS08a20Ctx->MaxIntegrationLine,
        MAX(pOS08a20Ctx->MinIntegrationLine, exp_line));

    TRACE(OS08a20_DEBUG, "%s: set AEC_PK_EXPO=0x%05x\n", __func__, exp_line);

    if (exp_line != pOS08a20Ctx->OldIntegrationTime) {
        if (pOS08a20Ctx->KernelDriverFlag) {
            ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_EXP, &exp_line);
        } else {
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3467, 0x00);
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3464, 0x04);

            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x30b6,
                               (exp_line >> 8) & 0xff);
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x30b7,
                               exp_line & 0xff);

            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3464, 0x14);
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3467, 0x01);
        }

        pOS08a20Ctx->OldIntegrationTime = exp_line;    // remember current integration time
        pOS08a20Ctx->AecCurIntegrationTime =
            exp_line * pOS08a20Ctx->one_line_exp_time;

        *pNumberOfFramesToSkip = 1U;    //skip 1 frame
    } else {
        *pNumberOfFramesToSkip = 0U;    //no frame skip
    }

    *pSetIntegrationTime = pOS08a20Ctx->AecCurIntegrationTime;

    TRACE(OS08a20_DEBUG, "%s: Ti=%f\n", __func__, *pSetIntegrationTime);
    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OS08a20_IsiSetVSIntegrationTimeIss
    (IsiSensorHandle_t handle,
     float NewIntegrationTime,
     float *pSetVSIntegrationTime,
     uint8_t * pNumberOfFramesToSkip, float *hdr_ratio)
{
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;
    RESULT result = RET_SUCCESS;
    uint32_t exp_line = 0;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (!pOS08a20Ctx) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if (!pSetVSIntegrationTime || !pNumberOfFramesToSkip) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid parameter (NULL pointer detected)\n",
              __func__);
        return (RET_NULL_POINTER);
    }

    exp_line = NewIntegrationTime / pOS08a20Ctx->one_line_exp_time;
    exp_line =
        MIN(pOS08a20Ctx->MaxIntegrationLine,
        MAX(pOS08a20Ctx->MinIntegrationLine, exp_line));

    if (exp_line != pOS08a20Ctx->OldVsIntegrationTime) {
        if (pOS08a20Ctx->KernelDriverFlag) {
            ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_VSEXP, &exp_line);
        } else {
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3467, 0x00);
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3464, 0x04);

            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x30b8,
                               (exp_line >> 8) & 0xff);
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x30b9,
                               exp_line & 0xff);

            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3467, 0x14);
            result =
                OS08a20_IsiRegisterWriteIss(handle, 0x3464, 0x01);
        }

        pOS08a20Ctx->OldVsIntegrationTime = exp_line;
        pOS08a20Ctx->AecCurVSIntegrationTime = exp_line * pOS08a20Ctx->one_line_exp_time;    //remember current integration time
        *pNumberOfFramesToSkip = 1U;    //skip 1 frame
    } else {
        *pNumberOfFramesToSkip = 0U;    //no frame skip
    }

    *pSetVSIntegrationTime = pOS08a20Ctx->AecCurVSIntegrationTime;

    TRACE(OS08a20_DEBUG, "%s: NewIntegrationTime=%f\n", __func__,
          NewIntegrationTime);
    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OS08a20_IsiExposureControlIss
    (IsiSensorHandle_t handle,
     float NewGain,
     float NewIntegrationTime,
     uint8_t * pNumberOfFramesToSkip,
     float *pSetGain, float *pSetIntegrationTime, float *hdr_ratio)
{
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;

    RESULT result = RET_SUCCESS;
    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (pOS08a20Ctx == NULL) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if ((pNumberOfFramesToSkip == NULL) || (pSetGain == NULL)
        || (pSetIntegrationTime == NULL)) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid parameter (NULL pointer detected)\n",
              __func__);
        return (RET_NULL_POINTER);
    }

    TRACE(OS08a20_DEBUG, "%s: g=%f, Ti=%f\n", __func__, NewGain,
          NewIntegrationTime);
          
    float long_gain=0;
    float long_exp=0;
    float short_gain=0;
    float short_exp=0;

    if (pOS08a20Ctx->enableHdr)
    {

        long_exp = NewIntegrationTime;
        long_gain = NewGain;

        float short_exposure_measure = NewIntegrationTime*NewGain / 16;

        if (short_exposure_measure < 60 * pOS08a20Ctx->one_line_exp_time * pOS08a20Ctx->AecMinGain)
        {
            short_exp = short_exposure_measure / pOS08a20Ctx->AecMinGain;
            short_gain = pOS08a20Ctx->AecMinGain;
        }else
        {
            short_exp = 60 * pOS08a20Ctx->one_line_exp_time;
            short_gain = short_exposure_measure / short_exp;
            if (short_gain > pOS08a20Ctx->AecMaxGain)
            {
                short_gain =  pOS08a20Ctx->AecMaxGain;
            }
        }

    }else
    {
        long_exp = NewIntegrationTime;
        long_gain = NewGain;
    }

    if (pOS08a20Ctx->enableHdr == true)
    {
        result =
            OS08a20_IsiSetVSIntegrationTimeIss(handle,
                              short_exp,
                              pSetIntegrationTime,
                              pNumberOfFramesToSkip,
                              hdr_ratio);
        result =
            OS08a20_IsiSetVSGainIss(handle, short_exp, short_gain,
                       pSetGain, hdr_ratio);
    }

    result =
        OS08a20_IsiSetIntegrationTimeIss(handle, long_exp,
                        pSetIntegrationTime,
                        pNumberOfFramesToSkip, hdr_ratio);
    TRACE(OS08a20_DEBUG, "%s: set: NewGain=%f, hcgTi=%f, hcgskip=%d\n",
          __func__, NewGain, NewIntegrationTime, *pNumberOfFramesToSkip);
    result = OS08a20_IsiSetGainIss(handle, long_gain, pSetGain, hdr_ratio);
    TRACE(OS08a20_DEBUG, "%s: set: NewGain=%f, hcgTi=%f, hcgskip=%d\n",
          __func__, NewGain, NewIntegrationTime, *pNumberOfFramesToSkip);
    TRACE(OS08a20_DEBUG, "%s: set: vsg=%f, vsTi=%f, vsskip=%d\n", __func__,
          NewGain, NewIntegrationTime, *pNumberOfFramesToSkip);
    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);

    return result;
}

RESULT OS08a20_IsiGetCurrentExposureIss
    (IsiSensorHandle_t handle, float *pSetGain, float *pSetIntegrationTime) {
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (pOS08a20Ctx == NULL) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    if ((pSetGain == NULL) || (pSetIntegrationTime == NULL))
        return (RET_NULL_POINTER);

    *pSetGain = pOS08a20Ctx->AecCurGain;
    *pSetIntegrationTime = pOS08a20Ctx->AecCurIntegrationTime;

    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OS08a20_IsiGetResolutionIss(IsiSensorHandle_t handle, uint16_t *pwidth, uint16_t *pheight) {
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (pOS08a20Ctx == NULL) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }

    *pwidth  = pOS08a20Ctx->SensorMode.width;
    *pheight =  pOS08a20Ctx->SensorMode.height;

    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OS08a20_IsiGetSensorFpsIss(IsiSensorHandle_t handle, uint32_t * pfps)
{
    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    RESULT result = RET_SUCCESS;
    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    if (pOS08a20Ctx == NULL) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    if (pOS08a20Ctx->KernelDriverFlag) {
        ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_FPS, pfps);
        pOS08a20Ctx->CurrFps = *pfps;
    }

    *pfps = pOS08a20Ctx->CurrFps;

    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return (result);
}

RESULT OS08a20_IsiSetSensorFpsIss(IsiSensorHandle_t handle, uint32_t fps)
{
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL) {
        TRACE(OS08a20_ERROR,
              "%s: Invalid sensor handle (NULL pointer detected)\n",
              __func__);
        return (RET_WRONG_HANDLE);
    }
    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;

    if (fps > pOS08a20Ctx->MaxFps) {
        TRACE(OS08a20_ERROR,
              "%s: set fps(%d) out of range, correct to %d (%d, %d)\n",
              __func__, fps, pOS08a20Ctx->MaxFps, pOS08a20Ctx->MinFps,
              pOS08a20Ctx->MaxFps);
        fps = pOS08a20Ctx->MaxFps;
    }
    if (fps < pOS08a20Ctx->MinFps) {
        TRACE(OS08a20_ERROR,
              "%s: set fps(%d) out of range, correct to %d (%d, %d)\n",
              __func__, fps, pOS08a20Ctx->MinFps, pOS08a20Ctx->MinFps,
              pOS08a20Ctx->MaxFps);
        fps = pOS08a20Ctx->MinFps;
    }

    if (pOS08a20Ctx->KernelDriverFlag) {
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_FPS, &fps);
        if (ret != 0) {
            TRACE(OS08a20_ERROR, "%s: set sensor fps=%d error\n",
                  __func__);
            return (RET_FAILURE);
        }

        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_G_SENSOR_MODE, &(pOS08a20Ctx->SensorMode));
        {
            pOS08a20Ctx->MaxIntegrationLine = pOS08a20Ctx->SensorMode.ae_info.max_interrgation_time;
            pOS08a20Ctx->AecMaxIntegrationTime = pOS08a20Ctx->MaxIntegrationLine * pOS08a20Ctx->one_line_exp_time;
        }
    }

    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return (result);
}

static RESULT OS08a20_IsiActivateTestPattern(IsiSensorHandle_t handle,
                        const bool_t enable)
{
    RESULT result = RET_SUCCESS;

    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

    if (pOS08a20Ctx->Configured != BOOL_TRUE)
        return RET_WRONG_STATE;

    if (BOOL_TRUE == enable) {
        result = OS08a20_IsiRegisterWriteIss(handle, 0x3253, 0x80);
    } else {
        result = OS08a20_IsiRegisterWriteIss(handle, 0x3253, 0x00);
    }
    pOS08a20Ctx->TestPattern = enable;

    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);

    return (result);
}

static RESULT OS08a20_IsiEnableHdr(IsiSensorHandle_t handle, const bool_t enable)
{
    RESULT result = RET_SUCCESS;
    int32_t ret = 0;
    TRACE(OS08a20_INFO, "%s: (enter)\n", __func__);

    OS08a20_Context_t *pOS08a20Ctx = (OS08a20_Context_t *) handle;
    if (pOS08a20Ctx == NULL || pOS08a20Ctx->IsiCtx.HalHandle == NULL) {
        return RET_NULL_POINTER;
    }

#ifdef SUBDEV_CHAR
    HalContext_t *pHalCtx = (HalContext_t *) pOS08a20Ctx->IsiCtx.HalHandle;
    result = OS08a20_IsiSensorSetStreamingIss(handle, 0);

    if (pOS08a20Ctx->KernelDriverFlag) {
        uint32_t hdr_mode;
        if (enable == 0) {
            hdr_mode = SENSOR_MODE_LINEAR;
        } else {
            hdr_mode = SENSOR_MODE_HDR_STITCH;
        }
        ret = ioctl(pHalCtx->sensor_fd, VVSENSORIOC_S_HDR_MODE,
              &hdr_mode);
    } else {
        result |=
            OS08a20_IsiRegisterWriteIss(handle, 0x3190,
                           enable ? 0x05 : 0x08);
    }
    result |=
        OS08a20_IsiSensorSetStreamingIss(handle, pOS08a20Ctx->Streaming);
#endif

    if (result != 0 || ret != 0) {
        TRACE(OS08a20_ERROR, "%s: change hdr status error\n", __func__);
        return RET_FAILURE;
    }

    pOS08a20Ctx->enableHdr = enable;
    TRACE(OS08a20_INFO, "%s: (exit)\n", __func__);
    return RET_SUCCESS;
}

RESULT OS08a20_IsiGetSensorIss(IsiSensor_t *pIsiSensor)
{
    RESULT result = RET_SUCCESS;
    TRACE( OS08a20_INFO, "%s (enter)\n", __func__);

    if ( pIsiSensor != NULL ) {
        pIsiSensor->pszName                         = SensorName;
        pIsiSensor->pIsiCreateSensorIss             = OS08a20_IsiCreateSensorIss;

        pIsiSensor->pIsiInitSensorIss               = OS08a20_IsiInitSensorIss;
        pIsiSensor->pIsiGetSensorModeIss            = OS08a20_IsiGetSensorModeIss;
        pIsiSensor->pIsiResetSensorIss              = OS08a20_IsiResetSensorIss;
        pIsiSensor->pIsiReleaseSensorIss            = OS08a20_IsiReleaseSensorIss;
        pIsiSensor->pIsiGetCapsIss                  = OS08a20_IsiGetCapsIss;
        pIsiSensor->pIsiSetupSensorIss              = OS08a20_IsiSetupSensorIss;
        pIsiSensor->pIsiChangeSensorResolutionIss   = OS08a20_IsiChangeSensorResolutionIss;
        pIsiSensor->pIsiSensorSetStreamingIss       = OS08a20_IsiSensorSetStreamingIss;
        pIsiSensor->pIsiSensorSetPowerIss           = OS08a20_IsiSensorSetPowerIss;
        pIsiSensor->pIsiCheckSensorConnectionIss    = OS08a20_IsiCheckSensorConnectionIss;
        pIsiSensor->pIsiGetSensorRevisionIss        = OS08a20_IsiGetSensorRevisionIss;
        pIsiSensor->pIsiRegisterReadIss             = OS08a20_IsiRegisterReadIss;
        pIsiSensor->pIsiRegisterWriteIss            = OS08a20_IsiRegisterWriteIss;

        /* AEC functions */
        pIsiSensor->pIsiExposureControlIss          = OS08a20_IsiExposureControlIss;
        pIsiSensor->pIsiGetGainLimitsIss            = OS08a20_IsiGetGainLimitsIss;
        pIsiSensor->pIsiGetIntegrationTimeLimitsIss = OS08a20_IsiGetIntegrationTimeLimitsIss;
        pIsiSensor->pIsiGetCurrentExposureIss       = OS08a20_IsiGetCurrentExposureIss;
        pIsiSensor->pIsiGetVSGainIss                    = OS08a20_IsiGetVSGainIss;
        pIsiSensor->pIsiGetGainIss                      = OS08a20_IsiGetGainIss;
        pIsiSensor->pIsiGetGainIncrementIss             = OS08a20_IsiGetGainIncrementIss;
        pIsiSensor->pIsiSetGainIss                      = OS08a20_IsiSetGainIss;
        pIsiSensor->pIsiGetIntegrationTimeIss           = OS08a20_IsiGetIntegrationTimeIss;
        pIsiSensor->pIsiGetVSIntegrationTimeIss         = OS08a20_IsiGetVSIntegrationTimeIss;
        pIsiSensor->pIsiGetIntegrationTimeIncrementIss  = OS08a20_IsiGetIntegrationTimeIncrementIss;
        pIsiSensor->pIsiSetIntegrationTimeIss           = OS08a20_IsiSetIntegrationTimeIss;
        pIsiSensor->pIsiQuerySensorIss                  = OS08a20_IsiQuerySensorIss;
        pIsiSensor->pIsiGetResolutionIss                = OS08a20_IsiGetResolutionIss;
        pIsiSensor->pIsiGetSensorFpsIss                 = OS08a20_IsiGetSensorFpsIss;
        pIsiSensor->pIsiSetSensorFpsIss                 = OS08a20_IsiSetSensorFpsIss;

        /* AWB specific functions */

        /* Testpattern */
        pIsiSensor->pIsiActivateTestPattern         = OS08a20_IsiActivateTestPattern;
        pIsiSensor->pIsiEnableHdr                   = OS08a20_IsiEnableHdr;
        pIsiSensor->pIsiSetBayerPattern             = OS08a20_IsiSetBayerPattern;

    } else {
        result = RET_NULL_POINTER;
    }

    TRACE( OS08a20_INFO, "%s (exit)\n", __func__);
    return ( result );
}

/*****************************************************************************
* each sensor driver need declare this struct for isi load
*****************************************************************************/
IsiCamDrvConfig_t IsiCamDrvConfig = {
    0,
    OS08a20_IsiQuerySensorSupportIss,
    OS08a20_IsiGetSensorIss,
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
