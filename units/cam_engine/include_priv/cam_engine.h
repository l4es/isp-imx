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
 * @file cam_engine.h
 *
 * @brief
 *   Internal interface of the CamEngine.
 *
 *****************************************************************************/
/**
 *
 * @defgroup cam_engine CamEngine internal interface
 * @{
 *
 */
#ifndef __CAM_ENGINE_H__
#define __CAM_ENGINE_H__

#include <ebase/types.h>
#include <oslayer/oslayer.h>
#include <common/return_codes.h>

#include <bufferpool/media_buffer.h>
#include <bufferpool/media_buffer_pool.h>
#include <bufferpool/media_buffer_queue_ex.h>

#include <cameric_drv/cameric_drv_api.h>
#include <mipi_drv/mipi_drv_api.h>

#include <aec/aec.h>
#include <awb/awb.h>
#include <af/af.h>
#include <adpf/adpf.h>
#include <adpcc/adpcc.h>
#include <a2dnr/a2dnr.h>
#include <a3dnr/a3dnr.h>
#include <awdr3/awdr3.h>
#include <ahdr/ahdr.h>
#include <aee/aee.h>
#include <aflt/aflt.h>


#include <avs/avs.h>




#include <mom_ctrl/mom_ctrl_api.h>
#include <mim_ctrl/mim_ctrl_api.h>
#include <bufsync_ctrl/bufsync_ctrl_api.h>

#include "cam_engine_api.h"
#include "cam_engine_isp_api.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @brief Generic command type.
 *
 */
typedef struct CamEngineCmd_s
{
    CamEngineCmdId_t    cmdId;
    void                *pCmdCtx;
} CamEngineCmd_t;


/**
 * @brief Chain index.
 *
 */
typedef enum CamEngineChainIdx_e
{
    CHAIN_LEFT  = 0,
    CHAIN_RIGHT = 1,
    CHAIN_MAX   = 2,
} CamEngineChainIdx_t;


/**
 * @brief   Internal context of the chain
 *
 * @note
 *
 */
typedef struct ChainCtx_s
{
    int                     mipiFd;
    IsiSensorHandle_t       hSensor;
    CamerIcDrvHandle_t      hCamerIc;
    MipiDrvHandle_t         hMipi;

    uint32_t                BufNum;

    MediaBufPoolMemory_t    BufPoolMemMain;
    MediaBufPool_t          BufPoolMain;
    MediaBufPoolConfig_t    BufPoolConfigMain;

    MediaBufPoolMemory_t    BufPoolMemSelf;
    MediaBufPool_t          BufPoolSelf;
    MediaBufPoolConfig_t    BufPoolConfigSelf;

	MediaBufPoolMemory_t    BufPoolMemSelf2;
    MediaBufPool_t          BufPoolSelf2;
    MediaBufPoolConfig_t    BufPoolConfigSelf2;

	MediaBufPoolMemory_t    BufPoolMemRdi;
    MediaBufPool_t          BufPoolRdi;
    MediaBufPoolConfig_t    BufPoolConfigRdi;

    MediaBufPoolMemory_t    BufPoolMemMeta;
    MediaBufPool_t          BufPoolMeta;
    MediaBufPoolConfig_t    BufPoolConfigMeta;

    osQueue                 MainPathQueue;          /**< MainPathQueue */

    osEvent                 MomEventCmdStart;
    osEvent                 MomEventCmdStop;
    MomCtrlContextHandle_t  hMomCtrl;

    CamEngineWindow_t       isWindow;
    CamerIcCompletionCb_t   CamerIcCompletionCb;
} ChainCtx_t;


/**
 * @brief Internal context of the cam-engine
 *
 */
typedef struct CamEngineContext_s
{
    CamEngineState_t            state;              /**< State of the CamEngine */

    CamEngineCompletionCb_t     cbCompletion;       /**< Completion callback. */
    CamEngineAfpsResChangeCb_t  cbAfpsResChange;    /**< Afps resolution chnage request callback */
    void                        *pUserCbCtx;        /**< User context for completion & Afps callbacks. */

    CamerIcMiOrientation_t      orient;

    uint32_t                    maxCommands;        /**< Max pending commands in command queue. */
    osQueue                     commandQueue;       /**< Command queue. */
    osThread                    thread;             /**< CamEngine thread. */

    CamerIcCompletionCb_t       camCaptureCompletionCb;     /**< normally implemented in CamEngine */
    CamerIcCompletionCb_t       camStopInputCompletionCb;

    bool_t                      isSystem3D;
    bool_t                      enable3D;
    bool_t                      enableDMA;
    ChainCtx_t                  chain[CHAIN_MAX];

    HalHandle_t                 hHal;               /**< handle to HAL */

    CamEngineWindow_t           acqWindow;
    CamEngineWindow_t           outWindow;
    CamEngineWindow_t           isWindow;
    CamerIcIspMode_t            inMode;

#ifdef ISP_DPF_RAW
    //DPF RAW out control
    CamerIcIspDpfRaw_t            dpf_mode;
#endif
#ifdef ISP_MI_FIFO_DEPTH_NANO
    //NANO Mi MP fifo control
    CamerIcMiMpOutputFifoDepth_t  mi_mp_fifo_depth;
#endif
#ifdef ISP_MI_ALIGN_NANO
    //NANO Mi MP output control
    CamerIcMiMpOutputLsbAlign_t   mi_mp_lsb_align;
#endif
#ifdef ISP_MI_BYTESWAP
    CamerIcMiMpOutputSwapByte_t   mi_mp_byte_swap;
#endif
#ifdef ISP_MI_HANDSHAKE_NANO
    CamerIcMiMpHandshake_t        mi_mp_handshake;
#endif

    CamEngineFlickerPeriod_t    flickerPeriod;
    bool_t                      enableAfps;

    /* output size */
    uint32_t                    outWidth[CAM_ENGINE_PATH_MAX];
    uint32_t                    outHeight[CAM_ENGINE_PATH_MAX];

    /* jpeg - encoder */
    bool_t                      enableJPE;
    osEvent                     JpeEventGenHeader;

    /* Mim-Ctrl */
    osEvent                     MimEventCmdStart;
    osEvent                     MimEventCmdStop;
    MimCtrlContextHandle_t      hMimCtrl;

    /* BufSync-Ctrl */
    osEvent                     BufSyncEventStart;
    osEvent                     BufSyncEventStop;
    BufSyncCtrlHandle_t         hBufSyncCtrl;

    MediaBufPoolConfig_t        BufferPoolConfigInput;   /**< configuration self-path bufferpool */
    MediaBufPoolMemory_t        BufferPoolMemInput;      /**< */
    MediaBufPool_t              BufferPoolInput;         /**< */

    PicBufMetaData_t            *pDmaBuffer;
    MediaBuffer_t               *pDmaMediaBuffer;

    CamEngineBufferCb_t         cbBuffer;
    void*                       *pBufferCbCtx;

    CamResolutionName_t         ResName;                /**< identifier to get resolution dependend calibration data from db */
    CamCalibDbHandle_t          hCamCalibDb;            /**< handle to calibration database */

    CamEngineLockType_t         LockMask;
    CamEngineLockType_t         LockedMask;

    AecHandle_t                 hAec;
    AecHandle_t                 hAev2;
    AwbHandle_t                 hAwb;
    AfHandle_t                  hAf;
    AdpfHandle_t                hAdpf;
    AdpccHandle_t               hAdpcc;
	A2dnrHandle_t               hA2dnr;
    A3dnrHandle_t               hA3dnr;
	Awdr3Handle_t               hAwdr3;
    AhdrHandle_t                hAhdr;
	AeeHandle_t               	hAee;
    AfltHandle_t                hAflt;
    AvsHandle_t                 hAvs;                       /**< handle of aito video stabilization module */
    bool_t                      availableAf;
    bool_t                      enableVsm;
    AvsConfig_t                 avsConfig;
    AvsDampFuncParams_t         avsDampParams;
    int                         numDampData;
    double                      *pDampXData;
    double                      *pDampYData;
#ifdef SUBDEV_V4L2
    char szIspNodeNameLeft[32];
    char szIspNodeNameRight[32];
#endif
    bool_t hdr;

} CamEngineContext_t;




/*****************************************************************************/
/**
 * @brief   Create an instance of CamEngine
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineCreate
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   Destroy an instance of CamEngine
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineDestroy
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   initialize
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineInitCamerIc
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineConfig_t   *pConfig
);


/*****************************************************************************/
/**
 * @brief   undo init
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReleaseCamerIc
(
    CamEngineContext_t  *pCamEngineCtx
);



/*****************************************************************************/
/**
 * @brief   prepare access to calibration database
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEnginePrepareCalibDbAccess
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   initialize
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineInitPixelIf
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineConfig_t   *pConfig
);


/*****************************************************************************/
/**
 * @brief   undo init
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReleasePixelIf
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   initialize
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineInitDrvForSensor
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineConfig_t   *pConfig
);


/*****************************************************************************/
/**
 * @brief   initialize
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineInitDrvForTestpattern
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineConfig_t   *pConfig
);


/*****************************************************************************/
/**
 * @brief   initialize
 *
 * @param   pCamEngineCtx   Pointer to the context of cam-engine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineInitDrvForDma
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   initialize
 *
 * @param   pCamEngineCtx   Pointer to the context of cam-engine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineInitDrvForTpg
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineConfig_t   *pConfig
);


/*****************************************************************************/
/**
 * @brief   reinitialize
 *
 * @param   pCamEngineCtx   Pointer to the context of cam-engine instance
 * @param   numFramesToSkip Number of frames to skip
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReInitDrv
(
    CamEngineContext_t  *pCamEngineCtx,
    uint32_t            numFramesToSkip
);


/*****************************************************************************/
/**
 * @brief   undo initialize
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineReleaseDrv
(
    CamEngineContext_t  *pCamEngineCtx
);


/*****************************************************************************/
/**
 * @brief   setup
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupAcqForSensor
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineConfig_t   *pConfig,
    CamEngineChainIdx_t idx
);


/*****************************************************************************/
/**
 * @brief   setup
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupAcqForDma
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineConfig_t   *pConfig,
    CamEngineChainIdx_t idx
);

/*****************************************************************************/
/**
 * @brief   setup TPG module of ISP
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupAcqForTpg
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineConfig_t   *pConfig,
    CamEngineChainIdx_t idx
);

/*****************************************************************************/
/**
 * @brief   setup
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupAcqResolution
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineChainIdx_t idx
);


/*****************************************************************************/
/**
 * @brief   CamEngineConfigureDataPath
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
 RESULT CamEngineSetupMiDataPath
(
    CamEngineContext_t              *pCamEngineCtx,
    const CamEnginePathConfig_t     *pConfigMain,
    const CamEnginePathConfig_t     *pConfigSelf,
    const CamEnginePathConfig_t     *pConfigSelf2,
    const CamEnginePathConfig_t     *pConfigRdi,
    const CamEnginePathConfig_t     *pConfigMeta,
    CamEngineChainIdx_t             idx
);


/*****************************************************************************/
/**
 * @brief   setup
 *
 * @param   pCamEngineCtx   Pointer to the context of CamEngine instance
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_VAL2
 *
 *****************************************************************************/
RESULT CamEngineSetupMiResolution
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineChainIdx_t idx,
    CamerIcMiDataMode_t modeMain,
    CamerIcMiDataMode_t modeSelf,
    CamerIcMiDataMode_t modeSelf2,
    CamerIcMiDataMode_t modeRdi,
	CamerIcMiDataMode_t modeMeta
);


/*****************************************************************************/
/**
 * @brief   CamEnginePreloadImage
 *
 * @param   pCamEngineCtx   context of the cam-engine instance
 * @param   pConfig         pointer to CamEngineConfig_t
 * @param   is_lsb_aligned  if need shift bit for msb aligned

 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeed
 * @retval  RET_WRONG_HANDLE    pCamEngine is NULL or invalid
 * @retval  RET_CANCELED        cam-instance is shutting down
 * @retval  RET_FAILURE         write to command-queue failed
 *
 *****************************************************************************/
RESULT CamEnginePreloadImage
(
    CamEngineContext_t  *pCamEngineCtx,
    CamEngineConfig_t   *pConfig,
    uint32_t            is_lsb_aligned
);


/*****************************************************************************/
/**
 * @brief   This functions sends a command to a CamEngine instance by putting
 *          a command id into the command-queue of the cam-egine instance.
 *
 * @param   pCamEngineCtx   context of the CamEngine instance
 * @param   pCommand        pointer to command to send
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         operation succeed
 * @retval  RET_WRONG_HANDLE    pCamEngine is NULL or invalid
 * @retval  RET_CANCELED        cam-instance is shutting down
 * @retval  RET_FAILURE         write to command-queue failed
 *
 *****************************************************************************/
RESULT CamEngineSendCommand
(
    CamEngineContext_t   *pCamEngineCtx,
    CamEngineCmd_t       *pCommand
);


#ifdef __cplusplus
}
#endif


/* @} cam_engine */

#endif /* __CAM_ENGINE_H__ */

