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
 * @file cam_engine_drv.h
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup module_name Module Name
 * @{
 *
 */

#ifndef __CAM_ENGINE_DRV_H__
#define __CAM_ENGINE_DRV_H__

#include <ebase/types.h>
#include <oslayer/oslayer.h>

#include <common/return_codes.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_drv_api.h>
#include <cameric_drv/cameric_isp_bls_drv_api.h>
#include <cameric_drv/cameric_isp_degamma_drv_api.h>
#include <cameric_drv/cameric_isp_dpf_drv_api.h>
#include <cameric_drv/cameric_isp_wdr_drv_api.h>
#include <cameric_drv/cameric_isp_dpcc_drv_api.h>
#include <cameric_drv/cameric_isp_is_drv_api.h>
#include <cameric_drv/cameric_isp_hist_drv_api.h>
#include <cameric_drv/cameric_isp_exp_drv_api.h>
#include <cameric_drv/cameric_isp_expv2_drv_api.h>
#include <cameric_drv/cameric_isp_awb_drv_api.h>
#include <cameric_drv/cameric_isp_afm_drv_api.h>
#include <cameric_drv/cameric_isp_vsm_drv_api.h>
#include <cameric_drv/cameric_isp_cac_drv_api.h>
#include <cameric_drv/cameric_isp_cnr_drv_api.h>
#include <cameric_drv/cameric_isp_flt_drv_api.h>
#include <cameric_drv/cameric_isp_lsc_drv_api.h>
#include <cameric_drv/cameric_isp_stitching_drv_api.h>

#ifdef ISP_GCMONO
#include <cameric_drv/cameric_isp_gcmono_drv_api.h>
#endif

#ifdef ISP_RGBGC
#include <cameric_drv/cameric_isp_rgbgamma_drv_api.h>
#endif

#ifdef ISP_DEMOSAIC2
#include <cameric_drv/cameric_isp_dmsc2_drv_api.h>
#endif

#include <cameric_drv/cameric_isp_green_equilibration_drv_api.h>

#ifdef ISP_CA
#include <cameric_drv/cameric_isp_color_adjust_drv_api.h>
#endif

#include <cameric_drv/cameric_jpe_drv_api.h>
#include <cameric_drv/cameric_mi_drv_api.h>
#include <cameric_drv/cameric_mipi_drv_api.h>
#ifdef ISP_DEC
#include <cameric_drv/cameric_isp_dec_drv_api.h>
#endif

#include "cam_engine.h"


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineInitCamerIcDrv
(
    CamEngineContext_t              *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReleaseCamerIcDrv
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineStartCamerIcDrv
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineStopCamerIcDrv
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupCamerIcDrv
(
    CamEngineContext_t                  *pCamEngineCtx,
    const CamerIcIspDemosaicBypass_t    demosaicMode,
    const uint8_t                       demosaicThreshold,
    const bool_t                        activateGammaIn,
    const bool_t                        activateGammaOut,
    const bool_t                        activateWB
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReStartEventCbCamerIcDrv
(
    CamEngineContext_t              *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupJpeDrv
(
    CamEngineContext_t *pCamEngineCtx
);



/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReleaseJpeDrv
(
    CamEngineContext_t  *pCamEngineCtx
);



/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupHistogramDrv
(
    CamEngineContext_t          *pCamEngineCtx,
    const bool_t                enable,
    const CamerIcIspHistMode_t  mode
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReleaseHistogramDrv
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupExpDrv
(
    CamEngineContext_t                  *pCamEngineCtx,
    const bool_t                        enable,
    const CamerIcIspExpMeasuringMode_t  mode
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReleaseExpDrv
(
    CamEngineContext_t  *pCamEngineCtx
);

/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupExpV2Drv
(
    CamEngineContext_t                  *pCamEngineCtx,
    const bool_t                        enable,
    const CamEngineAev2Cfg_t            *aev2Cfg

);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReleaseExpV2Drv
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupAwbDrv
(
    CamEngineContext_t  *pCamEngineCtx,
    const bool_t        enable
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReleaseAwbDrv
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupAfmDrv
(
    CamEngineContext_t  *pCamEngineCtx,
    const bool_t        enable
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReleaseAfmDrv
(
    CamEngineContext_t  *pCamEngineCtx
);

RESULT CamEngineSetupVsmDrv
(
    CamEngineContext_t  *pCamEngineCtx,
    CamerIcWindow_t     *winCfg,
    const bool_t        enable
);

RESULT CamEngineReleaseVsmDrv
(
    CamEngineContext_t  *pCamEngineCtx
);

/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupLscDrv
(
    CamEngineContext_t  *pCamEngineCtx,
    const bool_t        enable
);

/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupBlsDrv
(
    CamEngineContext_t          *pCamEngineCtx,
    const bool_t                enable,
    const CamEngineBlackLevel_t *pBlvl
);

RESULT CamEngineSetupCompandDrv
(
    CamEngineContext_t              *pCamEngineCtx,
    const bool_t                    enable,
    CamEngineConfig_t               *pConfig,
    const CamEngineBlackLevel_t     *pBlvl
);

#ifdef ISP_GCMONO
/******************************************************************************
 * CamEngineSetupGcmonoDrv()
 *
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupGcmonoDrv
(
    CamEngineContext_t                   *pCamEngineCtx,
    CamerIcIspGcMonoContext_t            *pGcMonoCtx
);

/******************************************************************************
 * CamEngineSwitchGcmonoDrv()
 *
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSwitchGcmonoDrv
(
    CamEngineContext_t                   *pCamEngineCtx,
    CamerIcIspGCMonoSwitch_t              enable
);
#endif

/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupCacDrv
(
    CamEngineContext_t      *pCamEngineCtx,
    const bool_t            enable
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineStartMipiDrv
(
    CamEngineContext_t *pCamEngineCtx,
    MipiDataType_t      mipiMode
);


/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineStopMipiDrv
(
    CamEngineContext_t *pCamEngineCtx
);

#ifdef ISP_DEC
/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/

RESULT CamEngineSetupDecDrv
(
    CamEngineContext_t  *pCamEngineCtx,
    const bool_t        enable
);
#endif
RESULT CamEngineSetupHdrDrv
(
    CamEngineContext_t      *pCamEngineCtx,
    const bool_t            enable
);
RESULT CamEngineReleaseHdrDrv
(
    CamEngineContext_t  *pCamEngineCtx
);

RESULT CamEngineSetupDigitalGainDrv(CamEngineContext_t *pCamEngineCtx,
                                    CamerIcIspDigitalGainConfig_t *pDiGainCfg, const bool_t enable);

RESULT CamEngineSetupGreenEquilibrationDrv
(
    CamEngineContext_t  *pCamEngineCtx,
    CamerIcIspGreenEqlr_t *pCfg,
    const bool_t        enable
);

#ifdef ISP_DEMOSAIC2

RESULT CamEngineSetupDmsc2Drv
(
    CamEngineContext_t  *pCamEngineCtx,
    CamericIspDemosaic_t     *demosaicCfg,
    const bool_t        enable
);

RESULT CamEngineReleaseDmsc2Drv
(
    CamEngineContext_t  *pCamEngineCtx
);
RESULT CamEngineSetupFltDrv(CamEngineContext_t *pCamEngineCtx, CamericIspDmscFlt_t *pFltCfg, const bool_t enable);

#else
/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 * @param   param2      Describe the parameter 2
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupFltDrv(CamEngineContext_t *pCamEngineCtx, const bool_t enable);

#endif
RESULT CamEngineConfigDummyHblank(CamEngineContext_t *pCamEngineCtx, uint8_t bp,uint8_t fp,uint8_t w,uint8_t in_hsize);
RESULT CamEngineConfig3DnrCompress(CamEngineContext_t *pCamEngineCtx,
                                   CamericIsp3DnrCompress_t *compress);

/* @} module_name*/

#endif /*__CAM_ENGINE_DRV_H__*/

