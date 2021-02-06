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
 * @file cam_engine_common.h
 *
 * @brief
 *   Common definitions of the CamEngine.
 *
 *****************************************************************************/
/**
 *
 * @defgroup cam_engine_common CamEngine Common Definitions
 * @{
 *
 */
#ifndef __CAM_ENGINE_COMMON_H__
#define __CAM_ENGINE_COMMON_H__
#include <cam_device/cam_device_ispcore_defs.h>


#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/**
 * @brief   Handle to a CamEngine instance.
 *
 *****************************************************************************/
typedef struct CamEngineContext_s *CamEngineHandle_t;


/*****************************************************************************/
/**
 * @brief   Commands for the CamEngine
 *
 *****************************************************************************/
typedef enum CamEngineCmdId_e
{
    CAM_ENGINE_CMD_INVALID                  = 0,    /**< invalid command (only for initialization) */
    CAM_ENGINE_CMD_START                    = 1,    /**< start a cam-engine instance */
    CAM_ENGINE_CMD_STOP                     = 2,    /**< stop a cam-engine instance */
    CAM_ENGINE_CMD_SHUTDOWN                 = 3,    /**< shutdown a stopped cam-engine instance */

    CAM_ENGINE_CMD_START_STREAMING          = 4,    /**< start streaming */
    CAM_ENGINE_CMD_STOP_STREAMING           = 5,    /**< stop streaming */

    CAM_ENGINE_CMD_ACQUIRE_LOCK             = 6,    /**< locks the auto algorithms */
    CAM_ENGINE_CMD_RELEASE_LOCK             = 7,    /**< releases locks of the auto algorithms */

    CAM_ENGINE_CMD_INTERNAL_BASE            = 1000,                                 /**< base for internal commands */
    CAM_ENGINE_CMD_AAA_LOCKED               = (CAM_ENGINE_CMD_INTERNAL_BASE + 0),   /**< selected auto-algorithms are locked now */
    CAM_ENGINE_CMD_HW_STREAMING_FINISHED    = (CAM_ENGINE_CMD_INTERNAL_BASE + 20),  /**< send by CamerIc hw, if ISP disabled (streaming finished) */
    CAM_ENGINE_CMD_HW_DMA_FINISHED          = (CAM_ENGINE_CMD_INTERNAL_BASE + 21),  /**< send by CamerIc hw, if DMA transfer completed */
    CAM_ENGINE_CMD_HW_JPE_DATA_ENCODED      = (CAM_ENGINE_CMD_INTERNAL_BASE + 22),  /**< send by CamerIc hw, if JPE data encoded */

    CAM_ENGINE_CMD_MAX
} CamEngineCmdId_t;


/*****************************************************************************/
/**
 * @brief States of the CamEngine.
 *
 *****************************************************************************/
typedef enum CamEngineState_e
{
    eCamEngineStateInvalid    = 0x0000,   /**< FSM state is invalid since CamEngine instance does not exist. */
    eCamEngineStateInitialize = 0x0001,   /**< FSM is in state initialized. */
    eCamEngineStateRunning    = 0x0002,   /**< FSM is in state running. */
    eCamEngineStateStreaming  = 0x0003,   /**< FSM is in state streaming. */
    CAM_ENGINE_STATE_MAX
} CamEngineState_t;


/*****************************************************************************/
/**
 * @brief Processing paths of the CamEngine.
 *
 *****************************************************************************/
typedef enum CamEnginePathType_e
{
    CAM_ENGINE_PATH_INVALID = -1,
    CAM_ENGINE_PATH_MAIN    = 0,
    CAM_ENGINE_PATH_SELF    = 1,
    CAM_ENGINE_PATH_SELF2_BP   = 2,
    CAM_ENGINE_PATH_RDI     = 3,
    CAM_ENGINE_PATH_META    = 4,
    CAM_ENGINE_PATH_MAX     = 5
} CamEnginePathType_t;


/*****************************************************************************/
/**
 * @brief Input config types of the CamEngine.
 *
 *****************************************************************************/
typedef enum CamEngineConfigType_e
{
    CAM_ENGINE_CONFIG_INVALID = 0,
    CAM_ENGINE_CONFIG_SENSOR  = 1,
    CAM_ENGINE_CONFIG_IMAGE   = 2,
    CAM_ENGINE_CONFIG_TPG     = 3,
    CAM_ENGINE_CONFIG_MAX     = 4
} CamEngineConfigType_t;


/*****************************************************************************/
/**
 * @brief Flicker period types for the AEC algorithm.
 *
 *****************************************************************************/
typedef enum CamEngineFlickerPeriod_e
{
    CAM_ENGINE_FLICKER_OFF   = 0x00,
    CAM_ENGINE_FLICKER_100HZ = 0x01,
    CAM_ENGINE_FLICKER_120HZ = 0x02
} CamEngineFlickerPeriod_t;


/*****************************************************************************/
/**
 *  @brief Command completion signaling callback
 *
 *  Callback for signaling command completion which could require application
 *  interaction. The cmdId (see @ref CamEngineCmdId_t) identifies the completed
 *  command.
 *
 *****************************************************************************/
typedef void (* CamEngineCompletionCb_t)
(
    CamEngineCmdId_t    cmdId,          /**< command Id of the notifying event */
    RESULT              result,         /**< result of the executed command */
    const void          *pUserCbCtx     /**< user data pointer that was passed on creation (see @ref CamEngineInstanceConfig_t) */
);


/*****************************************************************************/
/**
 *  @brief AFPS resolution change request signaling callback
 *
 *  Callback for signaling an AFPS resolution (better: frame rate) change
 *  request to the application.
 *
 *****************************************************************************/
typedef void (*CamEngineAfpsResChangeCb_t)
(
    uint32_t            NewResolution,  /**< new resolution to switch to */
    const void          *pUserCbCtx     /**< user data pointer that was passed on creation (see @ref CamEngineInstanceConfig_t) */
);


/*****************************************************************************/
/**
 *  @brief Full buffer signaling callback
 *
 *  Callback for signaling a full buffer which should be handled by the
 *  application. The path (see @ref CamEnginePath_t) identifies the output
 *  path.
 *
 *****************************************************************************/
typedef void (*CamEngineBufferCb_t)
(
    CamEnginePathType_t path,           /**< output path of the media buffer */
    MediaBuffer_t       *pMediaBuffer,  /**< full media buffer */
    void                *pBufferCbCtx   /**< user data pointer that was passed on registering the callback (see @ref CamEngineRegisterBufferCb) */
);


/*****************************************************************************/
/**
 * @brief   Configuration structure of the output path.
 *
 *****************************************************************************/
typedef struct CamEnginePathConfig_s
{
    uint16_t                width;
    uint16_t                height;
    CamerIcMiDataMode_t     mode;
    CamerIcMiDataLayout_t   layout;
    CamerIcMiDataAlignMode_t   alignMode;
} CamEnginePathConfig_t;


/*****************************************************************************/
/**
 * @brief   Generic structure to define a window.
 *
 *****************************************************************************/
typedef struct CamEngineWindow_s
{
    uint16_t    hOffset;
    uint16_t    vOffset;
    uint16_t    width;
    uint16_t    height;
} CamEngineWindow_t;


/*****************************************************************************/
/**
 * @brief   Generic structure to define a vector.
 *
 *****************************************************************************/
typedef struct CamEngineVector_s
{
    int16_t   x;
    int16_t   y;
} CamEngineVector_t;


/*****************************************************************************/
/**
 * @brief   Generic structure for the white balance gains of the four color
 *          components.
 *
 *****************************************************************************/
typedef struct CamEngineWbGains_s
{
    float Red;
    float GreenR;
    float GreenB;
    float Blue;
} CamEngineWbGains_t;


/*****************************************************************************/
/**
 * @brief   Generic structure for the cross talk matrix of the four color
 *          components.
 *
 *****************************************************************************/
typedef struct CamEngineCcMatrix_s
{
    float Coeff[9U];
} CamEngineCcMatrix_t;


/*****************************************************************************/
/**
 * @brief   Generic structure for the cross talk offset of the four color
 *          components.
 *
 *****************************************************************************/
typedef struct CamEngineCcOffset_s
{
    int16_t Red;
    int16_t Green;
    int16_t Blue;
} CamEngineCcOffset_t;


/*****************************************************************************/
/**
 * @brief   Generic structure for the black level of the four color components.
 *
 *****************************************************************************/
typedef struct CamEngineBlackLevel_s
{
    uint16_t Red;
    uint16_t GreenR;
    uint16_t GreenB;
    uint16_t Blue;
} CamEngineBlackLevel_t;

typedef struct CamEngineAev2Cfg_s
{
    unsigned int vOffset;
    unsigned int hOffset;
    unsigned int width;
    unsigned int height;

    uint8_t rWeight;
    uint8_t grWeight;
    uint8_t gbWeight;
    uint8_t bWeight;
} CamEngineAev2Cfg_t;



#ifdef __cplusplus
}
#endif


/* @} cam_engine_common */


#endif /* __CAM_ENGINE_COMMON_H__ */

