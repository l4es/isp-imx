/******************************************************************************
 *
 * Copyright 2018, VeriSilicon Technologies Co, Ltd. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * VeriSilicon Microelectronics(Shanghai)Co., LTd, 20F.No.560, Songtao Road
 * Pudong New Area Shanghai 201203 P.R.China
 *
 *****************************************************************************/

#ifndef __CAMERIC_ISP_COMPAND_DRV_H__
#define __CAMERIC_ISP_COMPAND_DRV_H__

/**
 * @file cameric_isp_compand_drv.h
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
 * @defgroup cameric_isp_compand_drv CamerIc ISP Compand Driver Internal API
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_isp_compand_drv_api.h>



#define BLS_PARA_MAX    (4)

/*******************************************************************************
 *
 *          CamerIcIspCmpdContext_t
 *
 * @brief
 *
 */
typedef struct CamerIcIspCmpdContext_s
{
    bool_t      enabled;
    bool_t      compressEnable;
    bool_t      expandEnable;
    bool_t      blsEnable;
    int         blsPara[ BLS_PARA_MAX ];
    bool_t      cfgd;

    bool_t      cfg_update;
} CamerIcIspCmpdContext_t;


/* @} cameric_isp_compand_drv */

#endif /* __CAMERIC_ISP_COMPAND_DRV_H__ */

