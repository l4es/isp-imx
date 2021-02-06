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

#ifndef __CAMERIC_DRV_H__
#define __CAMERIC_DRV_H__

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
#include <isp_ioctl.h>

/**
 * @file cameric_drv.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup cameric_drv CamerIc Driver Internal
 * @{
 *
 */

#include <ebase/types.h>
#include <hal/hal_api.h>

#include <common/return_codes.h>

#include "cameric_isp_drv.h"

#ifdef MRV_BLACK_LEVEL_VERSION
#include "cameric_isp_bls_drv.h"
#endif /* MRV_BLACK_LEVEL_VERSION */

#ifdef MRV_AUTO_EXPOSURE_VERSION
#include "cameric_isp_exp_drv.h"
#endif /* MRV_AUTO_EXPOSURE_VERSION */

#ifdef MRV_AWB_VERSION
#include "cameric_isp_awb_drv.h"
#endif /* MRV_AWB_VERSION */

#ifdef MRV_ELAWB_VERSION
#include "cameric_isp_elawb_drv.h"
#endif /* MRV_ELAWB_VERSION */
#ifdef MRV_AUTOFOCUS_VERSION
#include "cameric_isp_afm_drv.h"
#endif /* MRV_AUTOFOCUS_VERSION */

#include "cameric_isp_hist_drv.h"
#include "cameric_isp_wdr_drv.h"

#ifdef MRV_LSC_VERSION
#include "cameric_isp_lsc_drv.h"
#endif /* MRV_LSC_VERSION */

#ifdef MRV_FILTER_VERSION
#include "cameric_isp_flt_drv.h"
#endif /* MRV_FILTER_VERSION */

#ifdef MRV_VSM_VERSION
#include "cameric_isp_vsm_drv.h"            // isp video stabilization driver module
#endif /* MRV_VSM_VERSION */

#ifdef MRV_IMAGE_EFFECTS_VERSION
#include "cameric_ie_drv.h"
#else
#include "cameric_ie_drv_api.h"
#endif /* MRV_IMAGE_EFFECTS_VERSION */

#ifdef MRV_JPE_VERSION
#include "cameric_jpe_drv.h"
#endif /* MRV_JPE_VERSION */

#include "cameric_mi_drv.h"

#ifdef MRV_MIPI_VERSION
#include "cameric_mipi_drv.h"
#endif /* MRV_MIPI_VERSION */

#ifdef MRV_GCMONO_VERSION
#include "cameric_isp_gcmono_drv.h"
#endif /* MRV_GCMONO_VERSION */

#ifdef MRV_RGBGAMMA_VERSION
#include "cameric_isp_rgbgamma_drv.h"
#endif /* MRV_RGBGAMMA_VERSION */

#ifdef MRV_WDR2_VERSION
#include "cameric_isp_wdr2_drv.h"
#else
#include "cameric_isp_wdr2_drv_api.h"
#endif /* MRV_WDR2_VERSION */

#ifdef MRV_WDR3_VERSION
#include "cameric_isp_wdr3_drv.h"
#else
#include "cameric_isp_wdr3_drv_api.h"
#endif /* MRV_WDR3_VERSION */

#ifdef MRV_CMPD_VERSION
#include "cameric_isp_compand_drv.h"
#endif

#ifdef MRV_STITCHING_VERSION
#include "cameric_isp_stitching_drv.h"
#else
#include "cameric_isp_stitching_drv_api.h"
#endif

#ifdef MRV_EE_VERSION
#include "cameric_isp_ee_drv.h"
#else
#include "cameric_isp_ee_drv_api.h"
#endif

#ifdef MRV_2DNR_VERSION
#include "cameric_isp_2dnr_drv.h"
#else
#include "cameric_isp_2dnr_drv_api.h"
#endif

#ifdef MRV_3DNR_VERSION
#include "cameric_isp_3dnr_drv.h"
#else
#include "cameric_isp_3dnr_drv_api.h"
#endif

#ifdef MRV_AEV2_VERSION
#include "cameric_isp_expv2_drv.h"
#endif

#if MRV_DEMOSAIC_VERSION
#include "cameric_isp_dmsc2_drv.h"

#endif

#if MRV_GREENEQUILIBRATION_VERSION
#include "cameric_isp_green_equilibration_drv.h"
#endif

#if MRV_COLOR_ADJUST_VERSION
#include "cameric_isp_color_adjust_drv.h"

#endif
#if MRV_DEC_VERSION
#include "cameric_isp_dec_drv.h"

#endif
/******************************************************************************/
/**
 *          CamerIcDriverState_t
 *
 * @brief   Enumeration type which represents the internal state of the CamerIc
 *          software driver.
 *
 * @note
 *
 *****************************************************************************/
typedef enum CamerIcDriverState_e
{
    CAMERIC_DRIVER_STATE_INVALID    = 0,    /**< invalid state */
    CAMERIC_DRIVER_STATE_INIT       = 1,    /**< driver is initialized */
    CAMERIC_DRIVER_STATE_RUNNING    = 2,    /**< driver is started and running */
    CAMERIC_DRIVER_STATE_STOPPED    = 3,    /**< driver is stopped */
    CAMERIC_DRIVER_STATE_MAX
} CamerIcDriverState_t;



/******************************************************************************/
/**
 *          CamerIcDrvContext_t
 *
 * @brief   Context type of the CamerIc software driver.
 *
 * @note
 *
 *****************************************************************************/
typedef struct CamerIcDrvContext_s
{
    uint32_t                        base;               /**< base address of CamerIc */
    uint32_t                        dec_base;               /**< base address of Dec ip*/

    CamerIcDriverState_t            DriverState;        /**< internal state of the software driver */

    CamerIcMainPathMux_t            MpMux;              /**< main path muxer (vi_mp_mux) */
    CamerIcSelfPathMux_t            SpMux;              /**< self path muxer (vi_dma_spmux) */
    CamerIcYcSplitterChannelMode_t  YcSplitter;         /**< y/c-spliiter (vi_chan_mode) */
    CamerIcImgEffectsMux_t          IeMux;              /**< image effects muxer (vi_dma_iemux) */
    CamerIcDmaReadPath_t            DmaRead;            /**< dma read switch (vi_dma_switch) */
    CamerIcInterfaceSelect_t        IfSelect;           /**< interface selector (if_select) */

    HalHandle_t                     HalHandle;          /**< HAL handle given by CamerIcDriverInit */

    uint32_t                        tpg_flag;           /** TPG test internal flag*/
	uint32_t                        mcmBypassEnable;    /** MCM bypass enable*/
    uint32_t                        aec_skip_cnt;       /* skip aec */
    uint32_t                        awb_skip_cnt;       /* skip awb */

    CamerIcCompletionCb_t           *pCapturingCompletionCb;
    CamerIcCompletionCb_t           *pStopInputCompletionCb;
    CamerIcCompletionCb_t           *pDmaCompletionCb;

    CamerIcIspContext_t             *pIspContext;       /**< pointer to CamerIc ISP driver context */
    CamerIcMiContext_t              *pMiContext;        /**< pointer to CamerIc MI driver context */

#ifdef MRV_MIPI_VERSION
    CamerIcMipiContext_t            *pMipiContext;      /**< pointer to CamerIc MIPI driver context */
#endif /* MRV_MIPI_VERSION */

#ifdef MRV_BLACK_LEVEL_VERSION
    CamerIcIspBlsContext_t          *pIspBlsContext;    /**< pointer to CamerIc ISP BLS driver context */
#endif /* MRV_BLACK_LEVEL_VERSION */

#ifdef MRV_LSC_VERSION
    CamerIcIspLscContext_t          *pIspLscContext;    /**< pointer to CamerIc ISP LSC driver context */
#endif /* MRV_LSC_VERSION */

#ifdef MRV_FILTER_VERSION
    CamerIcIspFltContext_t          *pIspFltContext;    /**< pointer to CamerIc ISP FLT driver context */
#endif /* MRV_FILTER_VERSION */

#ifdef MRV_AUTO_EXPOSURE_VERSION
    CamerIcIspExpContext_t          *pIspExpContext;    /**< pointer to CamerIc ISP EXPOSURE driver context */
#endif /* MRV_BLACK_LEVEL_VERSION */

#ifdef MRV_AEV2_VERSION
    CamerIcIspExpV2Context_t         *pIspExpV2Context;    /**< pointer to CamerIc ISP EXPOSURE driver context */
#endif /* MRV_BLACK_LEVEL_VERSION */

#if MRV_HISTOGRAM_VERSION
    CamerIcIspHistContext_t         *pIspHistContext;   /**< pointer to CamerIc ISP HIST driver context */
#endif /* MRV_HISTOGRAM_VERSION */

#ifdef MRV_AWB_VERSION
    CamerIcIspAwbContext_t          *pIspAwbContext;    /**< pointer to CamerIc ISP Auto white balance driver context */
#endif /* MRV_AWB_VERSION */

#ifdef MRV_ELAWB_VERSION
    CamerIcIspElAwbContext_t        *pIspElAwbContext;  /**< pointer to CamerIc ISP elliptic auto white balance driver context */
#endif /* MRV_AWB_VERSION */

#ifdef MRV_AUTOFOCUS_VERSION
    CamerIcIspAfmContext_t          *pIspAfmContext;    /**< pointer to CamerIc ISP Auto white balance driver context */
#endif /* MRV_AUTOFOCUS_VERSION */

#if MRV_WDR_VERSION
    CamerIcIspWdrContext_t          *pIspWdrContext;    /**< pointer to CamerIc ISP WDR driver context */
#endif /* MRV_WDR_VERSION */

#ifdef MRV_IMAGE_EFFECTS_VERSION
    CamerIcIeContext_t              *pIeContext;        /**< pointer to CamerIc IE driver context */
#endif /* MRV_IMAGE_EFFECTS_VERSION */

#ifdef MRV_JPE_VERSION
    CamerIcJpeContext_t             *pJpeContext;       /**< pointer to CamerIc JPE driver context */
#endif /* MRV_JPE_VERSION */

#if MRV_VSM_VERSION
    CamerIcIspVsmContext_t          *pIspVsmContext;            /**< reference to CamerIc ISP VSM driver context */
#endif

#ifdef MRV_GCMONO_VERSION
    CamerIcIspGcMonoContext_t       *pIspGcMonoContext;         /**< reference to CamerIc ISP Gcmono driver context */
#endif

#ifdef MRV_RGBGAMMA_VERSION
    CamerIcIspRgbGammaContext_t       *pIspRgbGammaContext;         /**< reference to CamerIc ISP Gcmono driver context */
#endif

#if MRV_WDR2_VERSION
    CamerIcIspWdr2Context_t         *pIspWdr2Context;           /**< pointer to CamerIc ISP WDR2 driver context */
#endif

#ifdef MRV_WDR3_VERSION
    CamerIcIspWdr3Context_t         *pIspWdr3Context;           /**< pointer to CamerIc ISP WDR3 driver context */
#endif

#ifdef MRV_CMPD_VERSION
    CamerIcIspCmpdContext_t         *pIspCmpdContext;           /**< pointer to CamerIc ISP compand driver context */
#endif

#ifdef MRV_STITCHING_VERSION
	CamerIcIspStitchingContext_t    *pIspStitchingContext;	   /**< pointer to CamerIc ISP STITCHING drivercontext */
#endif

#ifdef MRV_EE_VERSION
    CamerIcIspEeContext_t 			*pIspEeContext;            /**< pointer to CamerIc ISP STITCHING driver context */
#endif

#ifdef MRV_2DNR_VERSION
    CamerIcIsp2DnrContext_t         *pIsp2DnrContext;          /**< pointer to CamerIc ISP STITCHING driver context */
#endif

#ifdef MRV_3DNR_VERSION
	CamerIcIsp3DnrContext_t         *pIsp3DnrContext;          /* pointer to CamerIc isp 3DNR driver context */
#endif
#if MRV_DEMOSAIC_VERSION
    CamericIspDemosaic_t *pIspDmscContext; /* pointer to CamerIc isp dmsc driver context */

#endif
#if MRV_GREENEQUILIBRATION_VERSION
    CamerIcIspGreenEqlrContext_t         *pIspGreenEqlrContext;           /* pointer to CamerIc isp green equilibration driver context */

#endif
#if MRV_COLOR_ADJUST_VERSION
    CamerIcIspCaContext_t         *pIspCaContext;           /* pointer to CamerIc isp color adjust driver context */
#endif
#if MRV_DEC_VERSION
    CamerIcIspDecContext_t         *pIspDecContext;           /* pointer to CamerIc isp DEC driver context */

#endif
    //int fd;
    struct isp_ic_dev ic_dev;
    uint64_t frameNumber[CAMERIC_MI_PATH_MAX];
    uint64_t fpsCount[CAMERIC_MI_PATH_MAX];
    uint64_t initTick[CAMERIC_MI_PATH_MAX];
    float fps[CAMERIC_MI_PATH_MAX];
} CamerIcDrvContext_t;

int cameric_ioctl(CamerIcDrvContext_t* ctx, int cmd, void* args);

/* @} cameric_drv */

#endif /* __CAMERIC_DRV_H__ */
