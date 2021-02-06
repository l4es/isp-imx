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
#ifndef __CAMERIC_ISP_EE_DRV_H__
#define __CAMERIC_ISP_EE_DRV_H__

/**
 * @file cameric_isp_ee_drv.h
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
 * @defgroup cameric_isp_ee_drv CamerIc ISP EE Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_ee_drv_api.h>


/******************************************************************************
 * Is the hardware EE module available ?
 *****************************************************************************/
#if defined(MRV_EE_VERSION)


/*****************************************************************************/
/**
 * @brief   CamerIc ISP EE module internal driver context.
 *
 */
typedef struct CamerIcIspEeContext_s
{
    bool_t      							enabled;        /**< EE enabled */

	uint8_t  								strength;

	uint16_t 								yUpGain;
	uint16_t 								yDownGain;

	uint16_t 								uvGain;
	uint16_t 								edgeGain;

	uint8_t  								srcStrength;
	CamerIcIspEeInputDataFormatSelect_t		inputDataFormatSelect;

} CamerIcIspEeContext_t;


#else  /* #if defined(MRV_EE_VERSION) */

#endif /* #if defined(MRV_EE_VERSION) */


/* @} cameric_isp_ee_drv */

#endif /* __CAMERIC_ISP_EE_DRV_H__ */

