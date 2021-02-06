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

#ifndef __CAMERIC_ISP_3DNR_DRV_H__
#define __CAMERIC_ISP_3DNR_DRV_H__

/**
* @file cameric_isp_3dnr_drv.h
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
* @defgroup cameric_isp_3dnr_drv CamerIc ISP 3DNR Driver Internal API
* @{
*
*/
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_3dnr_drv_api.h>



#define CAMERIC_3DNR_SPACIAL_CURVE_SEGMENTS    17
#define CAMERIC_3DNR_TEMPERAL_CURVE_SEGMENTS   17

/*******************************************************************************
*
*          CamerIcIsp3ndrCurve_t
*
* @brief
*
*/
typedef struct CamerIcIsp3DNRCurve_s
{
	uint16_t   spacialCurve[CAMERIC_3DNR_SPACIAL_CURVE_SEGMENTS];
	uint16_t   temperalCurve[CAMERIC_3DNR_TEMPERAL_CURVE_SEGMENTS];
} CamerIcIsp3DNRCurve_t;


/*******************************************************************************
*
*          CamerIcIsp3DnrContext_t
*
* @brief
*
*/
typedef struct CamerIcIsp3DnrContext_s
{
	bool_t      enabled;
	bool_t      horizontal_en;
	bool_t		vertical_en;
	bool_t      temperal_en;
	bool_t      dilate_en;

    uint8_t     strength;
    uint16_t    motionFactor;
    uint16_t    deltaFactor;

	uint32_t    lastAverage;
	uint32_t    lastLastAverage;
	float       sensorAgain;
} CamerIcIsp3DnrContext_t;



/*****************************************************************************/
/**
*          CamerIcIsp3DnrInit()
*
* @brief   Initialize CamerIc ISP 3DNR driver context
*
* @return  Return the result of the function call.
* @retval  RET_SUCCESS
* @retval  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIsp3DnrInit
(
	CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
*          CamerIcIsp3DnrRelease()
*
* @brief   Release/Free CamerIc ISP 3DNR driver context
*
* @return  Return the result of the function call.
* @retval  RET_SUCCESS
* @retval  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIsp3DnrRelease
(
	CamerIcDrvHandle_t handle
);



/* @} cameric_isp_wdr_drv */

#endif /* __CAMERIC_ISP_3DNR_DRV_H__ */

