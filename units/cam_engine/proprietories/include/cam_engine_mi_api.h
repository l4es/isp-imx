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
 * @cond    cam_engine_mi
 *
 * @file    cam_engine_mi_api.h
 *
 * @brief
 *
 *   Interface description of the CamEngine MI.
 *
 *****************************************************************************/
/**
 *
 * @defgroup cam_engine_mi_api CamEngine MI Api
 * @{
 *
 */

#ifndef __CAM_ENGINE_MI_API_H__
#define __CAM_ENGINE_MI_API_H__

#include <ebase/types.h>
#include <common/return_codes.h>

#ifdef __cplusplus
extern "C"
{
#endif


#include <cameric_drv/cameric_mi_drv_api.h>



/*****************************************************************************/
/**
 * @brief   This function enables/disables horizontal flip.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineOriginal
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This function enables/disables horizontal flip.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineHorizontalFlip
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This function enables/disables vertical flip.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineVerticalFlip
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This function enables/disables rotation.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineRotateLeft
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This function enables/disables rotation.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineRotateRight
(
    CamEngineHandle_t hCamEngine
);


#ifdef ISP_MI_HANDSHAKE_NANO
/*****************************************************************************/
/**
 * @brief   This function works for handshake buffer allocation.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 * @param   pHandshake          handle to the handshake struct pointer
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineMiHandshakeBufferAlloc
(
    CamEngineHandle_t hCamEngine,
    CamerIcMiMpHandshake_t *pHandshake
);


/*****************************************************************************/
/**
 * @brief   Dump function for handshake ring buffer.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 * @param   pHandshake          handle to the handshake struct pointer
 * @param   dump_name           File name of dump file
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineMiHandshakeDumpBufToFile
(
    CamEngineHandle_t       hCamEngine,
    CamerIcMiMpHandshake_t *pHandshake,
    const char *            dump_name
);

/*****************************************************************************/
/**
 * @brief   This function works for handshake buffer free.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 * @param   pHandshake          handle to the handshake struct pointer
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineMiHandshakeBufferFree
(
    CamEngineHandle_t hCamEngine,
    CamerIcMiMpHandshake_t *pHandshake
);


/*****************************************************************************/
/**
 * @brief   Dump function works for Fifo depth register value dump.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 * @param   dump_name           dump file name
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineMiFifoDepthDumpBufToFile
(
    CamEngineHandle_t       hCamEngine,
    const char *            dump_name
);

#endif

/*****************************************************************************/
/**
 * @brief   Dump configure function works for MI MP media buffer dump.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 * @param   dump_name           dump file name
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_NULL_POINTER    handle is NULL
 * @retval  RET_FAILURE         module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineMiMpDumpBufCfg
(
    CamEngineHandle_t       hCamEngine,
    const char *            mp_dump_name
);

/*****************************************************************************/
/**
 * @brief   Dump configure function works for MI SP media buffer dump.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 * @param   dump_name           dump file name
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_NULL_POINTER    handle is NULL
 * @retval  RET_FAILURE         module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineMiSpDumpBufCfg
(
    CamEngineHandle_t       hCamEngine,
    const char *            sp_dump_name
);

/*****************************************************************************/
/**
 * @brief   Dump function works for MI MP media buffer dump to file.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 * @param   dump_name           dump file name
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_NULL_POINTER    handle is NULL
 * @retval  RET_FAILURE         module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineMiMpDumpToFile
(
    CamEngineHandle_t       hCamEngine
);

/*****************************************************************************/
/**
 * @brief   Dump function works for MI SP media buffer dump to file.
 *
 * @param   hCamEngine          handle to the CamEngine instance
 * @param   dump_name           dump file name
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_NULL_POINTER    handle is NULL
 * @retval  RET_FAILURE         module not available by driver or hardware
 *
 *****************************************************************************/
RESULT CamEngineMiSpDumpToFile
(
    CamEngineHandle_t       hCamEngine
);

#ifdef ISP_DEC
//#ifdef HAL_CMODEL
RESULT CamEngineMiMpCmpDumpToFile
(
    CamEngineHandle_t       hCamEngine,
    char* 					dstFileName
);

//#endif
#endif


#ifdef __cplusplus
}
#endif

/* @} cam_engine_mi_api */

#endif /* __CAM_ENGINE_MI_API_H__ */


