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

#ifndef __CAMERIC_ISP_STITCHING_DRV_H__
#define __CAMERIC_ISP_STITCHING_DRV_H__

/**
* @file cameric_isp_stithing_drv.h
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
* @defgroup cameric_isp_stitching_drv CamerIc ISP STITCHING Driver Internal API
* @{
*
*/
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_stitching_drv_api.h>



/*******************************************************************************
*
*          CamerIcIspStitchingContext_t
*
* @brief
*
*/
typedef struct CamerIcIspStitchingContext_s
{
	bool_t      								enabled;
	bool_t      								cfg_update;
	uint16_t									blsAFixed;
	uint16_t									blsBFixed;
	uint16_t									blsCFixed;
	uint16_t									blsDFixed;
	uint16_t									startLinear;
	uint16_t									factorMulLinear;
	uint16_t									startNonLinear;
	uint16_t									factorMulNonLinear;
	uint16_t									lsRatio0;
	uint16_t									lsRatio1;
	uint16_t									vsRatio0;
	uint16_t									vsRatio1;
	uint16_t            						lsExtBit;
	uint16_t            						vsExtBit;
	uint16_t									vsValidThresh;
	uint16_t									vsValidOffset;
	uint16_t									longSatThresh;
	uint16_t									longCombineWeight;
	uint8_t										colorWeight0;
	uint8_t										colorWeight1;
	uint8_t										colorWeight2;
	CamerIcGains_t								awbGains;
	CamerIcIspStitchingProperties_t				properties;
	CamerIcIspStitchingCompressLookUpTable_t 	compressLut;
} CamerIcIspStitchingContext_t;



/*****************************************************************************/
/**
*          CamerIcIspSTITCHINGInit()
*
* @brief   Initialize CamerIc ISP STITCHING driver context
*
* @return  Return the result of the function call.
* @retval  RET_SUCCESS
* @retval  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIspStitchingInit
(
	CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
*          CamerIcIspStitchingRelease()
*
* @brief   Release/Free CamerIc ISP STITCHING driver context
*
* @return  Return the result of the function call.
* @retval  RET_SUCCESS
* @retval  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIspStitchingRelease
(
	CamerIcDrvHandle_t handle
);



/* @} cameric_isp_stitching_drv */

#endif /* __CAMERIC_ISP_SITCHING_DRV_H__ */

