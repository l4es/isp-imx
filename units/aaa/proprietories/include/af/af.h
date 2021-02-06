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

#ifndef __AF_H__
#define __AF_H__

/**
 * @file af.h
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
 * @defgroup AFM Auto Focus Module
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>

#include <isi/isi_iss.h>
#include <isi/isi.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_afm_drv_api.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct AfContext_s *AfHandle_t;     /**< handle to AEC context */


/*******************************************************************************
 *
 *          AfSearchStrategy_t
 *
 * @brief   CamerIc AF Module search strategy
 *
 */
typedef enum AfSearchStrategy_e
{
    AFM_FSS_INVALID         = 0,
    AFM_FSS_FULLRANGE       = 1,    /**< scan the full focus range to find the point of best focus */
    AFM_FSS_HILLCLIMBING    = 2,    /**< use hillclimbing search */
    AFM_FSS_ADAPTIVE_RANGE  = 3,    /**< similar to full range search, but with multiple subsequent scans
                                         with decreasing range and step size will be performed. */
    AFM_FSS_MAX,
} AfSearchStrategy_t;



/*******************************************************************************
 *
 *          AfSearchTriggerMode_t
 *
 * @brief   CamerIc AF Module search strategy
 *
 */
typedef enum AfSearchTriggerMode_e
{
    AFM_FSTM_INVALID         = 0,
    AFM_FSTM_ONE_SHOT        = 1,   /**< (re-)start a one-shot search */
    AFM_FSTM_CONTINUOUSlY    = 2,   /**< (re-)start a continous search */
    AFM_FSTM_STOPPED         = 3,
    AFM_FSTM_MAX,
} AfSearchTriggerMode_t;



/*****************************************************************************/
/**
 *          AfInstanceConfig_t
 *
 * @brief   AF Module instance configuration structure
 *
 *****************************************************************************/
typedef struct AfInstanceConfig_s
{
    CamerIcDrvHandle_t      hCamerIc;

    AfHandle_t              hAf;            /**< handle returned by AfInit() */
} AfInstanceConfig_t;



/*****************************************************************************/
/**
 *          AfConfig_t
 *
 * @brief   AF Module configuration structure
 *
 *****************************************************************************/
typedef struct AfConfig_s
{
    IsiSensorHandle_t       hSensor;        /**< sensor handle */
    AfSearchStrategy_t      Afss;           /**< focus search strategy */
} AfConfig_t;



/*****************************************************************************/
/**
 *          AfInit()
 *
 * @brief   This function initializes the Auto Focus Module.
 *
 * @param   pConfig
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_INVALID_PARM
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT AfInit
(
    AfInstanceConfig_t *pInstConfig
);



/*****************************************************************************/
/**
 *          AfRelease()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfRelease
(
    AfHandle_t handle
);



/*****************************************************************************/
/**
 *          AfConfigure()
 *
 * @brief   This function configures the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 * @param   pConfig
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_INVALID_PARM
 * @retval  RET_WRONG_STATE
 * @retval  RET_NOTSUPP         sensor doesn't support AF
 *
 *****************************************************************************/
RESULT AfConfigure
(
    AfHandle_t handle,
    AfConfig_t *pConfig
);



/*****************************************************************************/
/**
 *          AfReConfigure()
 *
 * @brief   This function re-configures the Auto Focus Module
 *          after e.g. resolution change
 *
 * @param   handle  Handle to AFM
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
RESULT AfReConfigure
(
    AfHandle_t handle
);



/*****************************************************************************/
/**
 * @brief   This function returns BOOL_TRUE if the AF is settled.
 *
 * @param   handle      AF instance handle
 * @param   pSettled    pointer to settled value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfSettled
(
    AfHandle_t handle,
    bool_t      *pSettled
);



/*****************************************************************************/
/**
 *          AfStart()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfStart
(
    AfHandle_t                handle,
    const AfSearchStrategy_t  fss
);



/*****************************************************************************/
/**
 *          AfOneShot()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfOneShot
(
    AfHandle_t                handle,
    const AfSearchStrategy_t  fss
);


/*****************************************************************************/
/**
 *          AfStop()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfStop
(
    AfHandle_t handle
);



/*****************************************************************************/
/**
 *          AfStop()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfStatus
(
    AfHandle_t          handle,
    bool_t              *pRunning,
    AfSearchStrategy_t  *pFss
);



/*****************************************************************************/
/**
 *          AfProcessFrame()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfProcessFrame
(
    AfHandle_t                  handle,
    CamerIcAfmMeasuringResult_t *pMeasResults
);



/*****************************************************************************/
/**
 * @brief   The function
 *
 * @param   handle  AF instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfTryLock
(
    AfHandle_t handle
);



/*****************************************************************************/
/**
 * @brief   The function
 *
 * @param   handle  AF instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfUnLock
(
    AfHandle_t handle
);




#ifdef __cplusplus
}
#endif

/* @} AFM */


#endif /* __AF_H__*/
