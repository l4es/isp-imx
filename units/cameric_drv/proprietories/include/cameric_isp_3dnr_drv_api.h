#ifndef __CAMERIC_ISP_3DNR_DRV_API_H__
#define __CAMERIC_ISP_3DNR_DRV_API_H__

/**
* @file    cameric_isp_3dnr_drv_api.h
*
* @brief   This file contains the CamerIC ISP 3DNR driver API definitions.
*
*****************************************************************************/
/**
* @cond cameric_isp_3dnr
*
* @defgroup cameric_isp_3dnr_drv_api CamerIC ISP 3DNR Driver API definitions
* @{
*
*/
#include <ebase/types.h>
#include <common/return_codes.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct CamerIcIsp3DnrConfig_s {
    uint8_t               strength;		 /**< strength */
    uint16_t              motionFactor;	 /**< motionFactor */
    uint16_t              deltaFactor; 	 /**< deltaFactor */
} CamerIcIsp3DnrConfig_t;

typedef struct CamericIsp3DnrCompress_s {
    uint8_t weightUpY[2];
    uint8_t weightDown[4];
    uint8_t weightUpX[8];

} CamericIsp3DnrCompress_t;
/*****************************************************************************/
/**
*          CamerIcIsp3DnrEnable()
*
* @brief   Enable 3DNR Module
*
* @param   handle          CamerIc driver handle
*
* @return                  Return the result of the function call.
* @retval                  RET_SUCCESS
* @retval                  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIsp3DnrEnable
(
	CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
*          CamerIcIsp3DnrDisable()
*
* @brief   Disable 3DNR Module
*
* @param   handle          CamerIc driver handle
*
* @return                  Return the result of the function call.
* @retval                  RET_SUCCESS
* @retval                  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIsp3DnrDisable
(
	CamerIcDrvHandle_t handle
);



/*****************************************************************************/
/**
*          CamerIcIsp3DnrConfig()
*
* @brief   Config 3DNR Module
*
* @param   handle          CamerIc driver handle
*
* @return                  Return the result of the function call.
* @retval                  RET_SUCCESS
* @retval                  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIsp3DnrConfig
(
	CamerIcDrvHandle_t  handle,
    CamerIcIsp3DnrConfig_t  *p3DnrCfg
);



/*****************************************************************************/
/**
*          CamerIcIsp3DnrCompute()
*
* @brief   Compute 3DNR Module register use last average, last last average and gain
*
* @param   handle          CamerIc driver handle
*
* @return                  Return the result of the function call.
* @retval                  RET_SUCCESS
* @retval                  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIsp3DnrCompute
(
	CamerIcDrvHandle_t  handle
);



/*****************************************************************************/
/**
*          CamerIcIsp3DnrUpdate()
*
* @brief   Update 3DNR Module's last average and last average
*
* @param   handle          CamerIc driver handle
*
* @return                  Return the result of the function call.
* @retval                  RET_SUCCESS
* @retval                  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIsp3DnrUpdate
(
	CamerIcDrvHandle_t  handle,
	float 				expGain
);



/*****************************************************************************/
/**
*          CamerIcIsp3DnrSetStrength()
*
* @brief   set 3DNR Module's denoiseStrength
*
* @param   handle          CamerIc driver handle
*
* @return                  Return the result of the function call.
* @retval                  RET_SUCCESS
* @retval                  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIsp3DnrSetStrength(
	CamerIcDrvHandle_t  handle,
	unsigned int denoiseStrength
);

/*****************************************************************************/
/**
*          CamerIcIsp3DnrSetCompress()
*
* @brief   config CamerIc ISP 3DNR driver compress parm for 3dnr ddr less context
*
* @return  Return the result of the function call.
* @retval  RET_SUCCESS
* @retval  RET_FAILURE
*
*****************************************************************************/
RESULT CamerIcIsp3DnrSetCompress(
	CamerIcDrvHandle_t handle,
	CamericIsp3DnrCompress_t *cmp
);

#ifdef __cplusplus
}
#endif

/* @} cameric_isp_3dnr_drv_api */

/* @endcond */

#endif /* __CAMERIC_ISP_3DNR_DRV_API_H__ */

