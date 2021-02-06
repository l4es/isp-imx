/******************************************************************************
*
* Copyright 2010, Dream Chip Technologies GmbH. All rights reserved.
* No part of this work may be reproduced, modified, distributed, transmitted,
* transcribed, or translated into any language or computer format, in any form
* or by any means without written permission of:
* Dream Chip Technologies GmbH, Steinriede 10, 30827 Garbsen / Berenbostel,
* Germany
*
*****************************************************************************/
#ifndef __CAMERIC_ISP_2DNR_DRV_H__
#define __CAMERIC_ISP_2DNR_DRV_H__

/**
* @file cameric_isp_2dnr_drv.h
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
* @defgroup cameric_isp_2dnr_drv CamerIc ISP 2DNR Driver Internal API
* @{
*
*/
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_2dnr_drv_api.h>


#define  DENOISE2D_BIN  60


/*******************************************************************************
*
*          CamerIcIsp2DnrContext_t
*
* @brief
*
*/
typedef struct CamerIcIsp2DnrContext_s
{
	bool_t      enabled;
	uint8_t		strength;  		  				/**< strength */
    uint8_t		pregmaStrength;   				/**< pregmaStrength */
#if 0
	float 		sigma;       	  				/**< sigma */
#else
	uint16_t    sigmaY[DENOISE2D_BIN];			/**< sigmaY */
#endif
} CamerIcIsp2DnrContext_t;

/* @} cameric_isp_2dnr_drv */

#endif /* __CAMERIC_ISP_2DNR_DRV_H__ */


