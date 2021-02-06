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
 * @file isi_priv.h
 *
 * @brief Interface description for image sensor specific implementation (iss).
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup isi_priv
 * @{
 *
 */
#ifndef __ISI_PRIV_H__
#define __ISI_PRIV_H__

#include <ebase/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "isi_iss.h"

/******************************************************************************
* DEFINES
******************************************************************************/
#define MAX_REGISTER_TABLE_ENTRIES 256

#define ISI_I2C_NR_DAT_BYTES_1  (1)                     // sensor has some  8-bit registers
#define ISI_I2C_NR_DAT_BYTES_2  (2)                     // sensor has some 16-bit registers
#define ISI_I2C_NR_DAT_BYTES_4  (4)                     // sensor has some 32-bit registers


/******************************************************************************
* TYPEDEFS
******************************************************************************/

/*****************************************************************************/
/**
 *          IsiSensorContext_t
 *
 * @brief   abstract class of a sensor context
 *
 */
/*****************************************************************************/
typedef struct IsiSensorContext_s
{
    int            fd;                  /**< /dev/v4l-subdev file description */
    HalHandle_t    HalHandle;           /**< Handle of HAL session to use. */
    IsiSensor_t    *pSensor;            /**< points to the sensor device */
} IsiSensorContext_t;




/******************************************************************************
* FUNCTIONS
******************************************************************************/
//! for the current register tables (tsIsiRegDescription)
#if 0
RESULT IsiRegDefaultsApply( const tsIsiRegDescription* patRegDescr );
RESULT IsiRegDefaultsVerify( const tsIsiRegDescription* patRegDescr );
#endif


/*****************************************************************************/
/**
 *          IsiGetNrDatBytesIss
 *
 * @brief   Extracts from the register description table the amount of bytes
 *          the desired register consists of
 *
 * @param   address          register address
 * @param   pRegDesc         register description table
 *
 * @return  Return the number of bytes
 * @retval  ISI_I2C_NR_DAT_BYTES_1
 * @retval  ISI_I2C_NR_DAT_BYTES_2
 * @retval  ISI_I2C_NR_DAT_BYTES_4
 * @retval  0U                      unknown register
 *
 *****************************************************************************/
uint8_t IsiGetNrDatBytesIss
(
    const uint32_t            Address,
    const IsiRegDescription_t *pRegDesc
);



/*****************************************************************************/
/**
 *          IsiRegDefaultsApply
 *
 * @brief   This function applies the default values of the registers specified
 *          in the given table. Writes to all registers that have been declared
 *          as writable and do have a default value (appropriate enum in table).
 *
 * @param   handle      Handle to image sensor device
 * @param   pRegDesc    Register description table
 *
 * @return  Return the number of bytes
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiRegDefaultsApply
(
    IsiSensorHandle_t         handle,
    const IsiRegDescription_t *pRegDesc
);



/*****************************************************************************/
/**
 *          IsiRegDefaultsVerify
 *
 * @brief   Reads back registers from the image sensor and compares them
 *          against the register table. Only registers that are readable,
 *          writable and not volatile will be checked.
 *
 * @param   handle      Handle to image sensor device
 * @param   pRegDesc    Register description table
 *
 * @return  Return the number of bytes
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiRegDefaultsVerify
(
    IsiSensorHandle_t         handle,
    const IsiRegDescription_t *pRegDesc
);

#ifdef __cplusplus
}
#endif

#endif /* __ISI_PRIV_H__ */
