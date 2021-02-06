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
 * @cond    cam_engine_aaa
 *
 * @file    cam_engine_aaa_api.h
 *
 * @brief
 *
 *   Interface description of the CamEngine Auto Algorithms.
 *
 *****************************************************************************/
/**
 *
 * @defgroup cam_engine_aaa_api CamEngine Auto Algorithms API
 * @{
 *
 */

#ifndef __CAM_ENGINE_AAA_API_H__
#define __CAM_ENGINE_AAA_API_H__

#include <ebase/types.h>
#include <common/return_codes.h>
#include <cam_device/cam_device_ispcore_defs.h>

#ifdef __cplusplus
extern "C"
{
#endif



/*****************************************************************************/
/**
 *          CamEngineAwbRgProj_t
 *
 * @brief   AWB Projection Configuration in R/G Layer
 *
 *****************************************************************************/
typedef struct CamEngineAwbRgProj_s
{
    float   fRgProjIndoorMin;
    float   fRgProjOutdoorMin;
    float   fRgProjMax;
    float   fRgProjMaxSky;
} CamEngineAwbRgProj_t;







/*****************************************************************************/
/**
 * @brief   Auto-Exposure-Control histogram.
 *
 *****************************************************************************/
#define CAM_ENGINE_AEC_HIST_NUM_BINS           16  /**< number of bins */
typedef uint32_t CamEngineAecHistBins_t[CAM_ENGINE_AEC_HIST_NUM_BINS];



/*****************************************************************************/
/**
 * @brief   Auto-Exposure-Control luminance grid.
 *
 *****************************************************************************/
#define CAM_ENGINE_AEC_EXP_GRID_ITEMS          25  /**< number of grid items (see @ref CamerIcMeanLuma_t) */
typedef uint8_t CamEngineAecMeanLuma_t[CAM_ENGINE_AEC_EXP_GRID_ITEMS];



/*****************************************************************************/
/**
 * @brief   Auto-Focus-Control mode.
 *
 *****************************************************************************/
typedef enum CamEngineAfMode_e
{
    CAM_ENGINE_AUTOFOCUS_MODE_INVALID       = 0,    /**< invalid mode (only for initialization) */
    CAM_ENGINE_AUTOFOCUS_MODE_ONESHOT       = 1,    /**< one-shot mode (runs autofocus search once and stops atomatically after positioing the lense to max) */
    CAM_ENGINE_AUTOFOCUS_MODE_CONTINOUSLY   = 2,    /**< continuously observe the focus */
    CAM_ENGINE_AUTOFOCUS_MODE_EXTERNAL      = 3,    /**< attach an external algorithm */
    CAM_ENGINE_AUTOFOCUS_MODE_STOP          = 4,    /**< stop current autofocus search */
    CAM_ENGINE_AUTOFOCUS_MODE_MAX
} CamEngineAfMode_t;



/*****************************************************************************/
/**
 * @brief   Auto-2Dimension-Noise-Reduction mode.
 *
 *****************************************************************************/
typedef enum CamEngineA2dnrMode_e
{
    CAM_ENGINE_A2DNR_MODE_INVALID = 0,    /**< invalid mode (only for initialization) */
    CAM_ENGINE_A2DNR_MODE_MANUAL  = 1,    /**< manual mode */
    CAM_ENGINE_A2DNR_MODE_AUTO    = 2,    /**< run auto mode */
    CAM_ENGINE_A2DNR_MODE_MAX
} CamEngineA2dnrMode_t;



/*****************************************************************************/
/**
 *          CamEngineA2dnrParamNode_t
 *
 * @brief   A2DNR parameters node structure
 *
 *****************************************************************************/
typedef struct CamEngineA2dnrParamNode_s
{
	float					gain;
	float					integrationTime;
    float                 	sigma;       	    /**< sigma */
    uint8_t               	strength;  		  	/**< strength */
    uint8_t               	pregmaStrength;     /**< pregmaStrength */
} CamEngineA2dnrParamNode_t;



/*****************************************************************************/
/**
 * @brief   Auto-3Dimension-Noise-Reduction mode.
 *
 *****************************************************************************/
typedef enum CamEngineA3dnrMode_e
{
    CAM_ENGINE_A3DNR_MODE_INVALID = 0,    /**< invalid mode (only for initialization) */
    CAM_ENGINE_A3DNR_MODE_MANUAL  = 1,    /**< manual mode */
    CAM_ENGINE_A3DNR_MODE_AUTO    = 2,    /**< run auto mode */
    CAM_ENGINE_A3DNR_MODE_MAX
} CamEngineA3dnrMode_t;



/*****************************************************************************/
/**
 *          CamEngineA3dnrParamNode_t
 *
 * @brief   A3DNR parameters node structure
 *
 *****************************************************************************/
typedef struct CamEngineA3dnrParamNode_s
{
	float					gain;				/**< gain */
	float					integrationTime;	/**< integrationTime */
    uint8_t               	strength;			/**< strength */
    uint16_t              	motionFactor;		/**< motionFactor */
    uint16_t              	deltaFactor;		/**< deltaFactor */
} CamEngineA3dnrParamNode_t;



/*****************************************************************************/
/**
 * @brief   Auto-Wide-Daynamic-Range(Version3) mode.
 *
 *****************************************************************************/
typedef enum CamEngineAwdr3Mode_e
{
    CAM_ENGINE_AWDR3_MODE_INVALID = 0,    /**< invalid mode (only for initialization) */
    CAM_ENGINE_AWDR3_MODE_MANUAL  = 1,    /**< manual mode */
    CAM_ENGINE_AWDR3_MODE_AUTO    = 2,    /**< run auto mode */
    CAM_ENGINE_AWDR3_MODE_MAX
} CamEngineAwdr3Mode_t;



/*****************************************************************************/
/**
 *          CamEngineAwdr3ParamNode_t
 *
 * @brief   AWDR3 parameters node structure
 *
 *****************************************************************************/
typedef struct CamEngineAwdr3ParamNode_s
{
	float					gain;				/**< gain */
	float					integrationTime;	/**< integrationTime */
    uint8_t					strength;			/**< strength */
    uint8_t					globalStrength;		/**< globalStrength */
    uint8_t					maxGain;			/**< maxGain */
} CamEngineAwdr3ParamNode_t;



/*****************************************************************************/
/**
 * @brief   Auto-Edge-Enhancement mode.
 *
 *****************************************************************************/
typedef enum CamEngineAeeMode_e
{
    CAM_ENGINE_AEE_MODE_INVALID = 0,    /**< invalid mode (only for initialization) */
    CAM_ENGINE_AEE_MODE_MANUAL  = 1,    /**< manual mode */
    CAM_ENGINE_AEE_MODE_AUTO    = 2,    /**< run auto mode */
    CAM_ENGINE_AEE_MODE_MAX
} CamEngineAeeMode_t;



/*****************************************************************************/
/**
 *          CamEngineAeeParamNode_t
 *
 * @brief   AEE parameters node structure
 *
 *****************************************************************************/
typedef struct CamEngineAeeParamNode_s
{
	float					gain;				/**< gain */
	float					integrationTime;	/**< integrationTime */
    uint8_t					strength;			/**< strength */
	uint16_t				yUpGain;			/**< yUpGain */
	uint16_t				yDownGain;			/**< yDownGain */
	uint16_t				uvGain;				/**< uvGain */
	uint16_t				edgeGain;			/**< edgeGain */
} CamEngineAeeParamNode_t;



/*****************************************************************************/
/**
 * @brief   Auto-Filter mode.
 *
 *****************************************************************************/
typedef enum CamEngineAfltMode_e
{
    CAM_ENGINE_AFLT_MODE_INVALID = 0,    /**< invalid mode (only for initialization) */
    CAM_ENGINE_AFLT_MODE_MANUAL  = 1,    /**< manual mode */
    CAM_ENGINE_AFLT_MODE_AUTO    = 2,    /**< run auto mode */
    CAM_ENGINE_AFLT_MODE_MAX
} CamEngineAfltMode_t;



/*****************************************************************************/
/**
 *          CamEngineAfltParamNode_t
 *
 * @brief   AFLT parameters node structure
 *
 *****************************************************************************/
typedef struct CamEngineAfltParamNode_s
{
	float					gain;				/**< gain */
	float					integrationTime;	/**< integrationTime */
    uint8_t					denoiseLevel;		/**< denoiseLevel */
    uint8_t					sharpenLevel;		/**< sharpenLevel */
} CamEngineAfltParamNode_t;



/*****************************************************************************/
/**
 * @brief   This functions starts the Auto-White-Balance.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   mode                working mode ( Manual | Auto )
 * @param   index               illumination profile index
 *                              Manual: profile to run
 *                              Auto: start-profile
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_INVALID_PARM    invalid configuration
 * @retval  RET_OUTOFRANGE      a configuration parameter is out of range
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAwbStart
(
    CamEngineHandle_t           hCamEngine,     /**< handle CamEngine */
    const CamEngineAwbMode_t    mode,           /**< run-mode */
    const uint32_t              index,          /**< AUTO: start-profile, MANUAL: profile to run */
    const bool_t                damp            /**< damping on/off */
);



/*****************************************************************************/
/**
 * @brief   This functions stops the Auto-White-Balance.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAwbStop
(
    CamEngineHandle_t           hCamEngine      /**< handle CamEngine */
);



/*****************************************************************************/
/**
 * @brief   This functions resets the Auto-White-Balance.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAwbReset
(
    CamEngineHandle_t           hCamEngine      /**< handle CamEngine */
);



/*****************************************************************************/
/**
 * @brief   This functions returns the Auto-White-Balance status.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: running, BOOL_FALSE: stopped
 * @param   pMode               working mode ( Manual | Auto )
 * @param   pCieProfile         illumination profile
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAwbStatus
(
    CamEngineHandle_t           hCamEngine,     /**< handle CamEngine */
    bool_t                      *pRunning,      /**< BOOL_TRUE: running, BOOL_FALSE: stopped */
    CamEngineAwbMode_t          *pMode,
    uint32_t                    *pCieProfile,
    CamEngineAwbRgProj_t        *pRgProj,
    bool_t                      *pDamping       /**< BOOL_TRUE: damping on, BOOL_FALSE: damping off */
);



/*****************************************************************************/
/**
 * @brief   This functions starts the Auto-Exposure-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAecStart
(
    CamEngineHandle_t           hCamEngine      /**< handle CamEngine */
);



/*****************************************************************************/
/**
 * @brief   This functions stops the Auto-Exposure-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAecStop
(
    CamEngineHandle_t           hCamEngine      /**< handle CamEngine */
);



/*****************************************************************************/
/**
 * @brief   This functions resets the Auto-Exposure-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAecReset
(
    CamEngineHandle_t           hCamEngine      /**< handle CamEngine */
);



/*****************************************************************************/
/**
 * @brief   This functions configures the Auto-Exposure-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAecConfigure
(
    CamEngineHandle_t           hCamEngine,      /**< handle CamEngine */
    const CamEngineAecSemMode_t mode,            /**< scene evaluation mode */
    const float                 setPoint,        /**< set point to hit by the ae control system */
    const float                 clmTolerance,    /**< tolerance */
    const float                 dampOver,        /**< damping */
    const float                 dampUnder        /**< damping */
);



/*****************************************************************************/
/**
 * @brief   This functions returns the Auto-Exposure-Control status.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: running, BOOL_FALSE: stopped
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAecStatus
(
    CamEngineHandle_t           hCamEngine,        /**< handle CamEngine */
    bool_t                      *pRunning,         /**< BOOL_TRUE: running, BOOL_FALSE: stopped */
    CamEngineAecSemMode_t       *pMode,            /**< scene evaluation mode */
    float                       *pSetPoint,        /**< set point to hit by the ae control system */
    float                       *pClmTolerance,    /**< tolerance */
    float                       *pDampOver,        /**< damping */
    float                       *pDampUnder        /**< damping */
);


/*****************************************************************************/
/**
* @brief   This functions returns the Auto-Exposure-Control status of gain and exposure.
*
* @param   hCamEngine          Handle to the CamEngine instance.
*
* @return  Return the result of the function call.
* @retval  RET_SUCCESS         function succeed
* @retval  RET_WRONG_HANDLE    invalid instance handle
*
*****************************************************************************/
RESULT CamEngineAecGetGainAndIntegrationTime
(
	CamEngineHandle_t           hCamEngine,
	float                       *pGain,
	float                       *pIntegrationTime
);

/*****************************************************************************/
/**
 * @brief   This function returns the current Auto-Exposure-Control histogram.
 *
 * @param   handle              Handle to the CamEngine instance.
 * qparam   pHistogram          pointer to the histogram bins
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAecGetHistogram
(
    CamEngineHandle_t        hCamEngine,
    CamEngineAecHistBins_t   *pHistogram
);



/*****************************************************************************/
/**
 * @brief   This function returns the current Auto-Exposure-Control luminance grid.
 *
 * @param   handle              Handle to the CamEngine instance.
 * qparam   pLuma               pointer to the luminance grid
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAecGetLuminance
(
    CamEngineHandle_t        hCamEngine,
    CamEngineAecMeanLuma_t   *pLuma
);



/*****************************************************************************/
/**
 * @brief   This function returns the current Auto-Exposure-Control object region.
 *
 * @param   handle              Handle to the CamEngine instance.
 * qparam   pLuma               pointer to the object region
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAecGetObjectRegion
(
    CamEngineHandle_t        hCamEngine,
    CamEngineAecMeanLuma_t   *pObjectRegion
);



/*****************************************************************************/
/**
 * @brief   This function returns if Auto-Focus-Control is available with the
 *          connected sensor board.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: available, BOOL_FALSE: not available
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAfAvailable
(
    CamEngineHandle_t       hCamEngine,
    bool_t                  *pAvailable
);



/*****************************************************************************/
/**
 * @brief   This function starts the Auto-Focus-Control (continous mode).
 *
 * @param   hCamEngine              Handle to the CamEngine instance.
 * @param   AutoFocusSearchAgoritm  search algorithm ( ADAPTIVE_RANGE | HILL_CLIMBING )
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAfStart
(
    CamEngineHandle_t                    hCamEngine,
    const CamEngineAfSearchAlgorithm_t   searchAgoritm
);



/*****************************************************************************/
/**
 * @brief   This function starts the Auto-Focus-Control (one-shot mode).
 *
 * @param   hCamEngine              Handle to the CamEngine instance.
 * @param   AutoFocusSearchAgoritm  search algorithm ( ADAPTIVE_RANGE | HILL_CLIMBING )
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAfOneShot
(
    CamEngineHandle_t                    hCamEngine,
    const CamEngineAfSearchAlgorithm_t   searchAgoritm
);



/*****************************************************************************/
/**
 * @brief   This function stops the Auto-Focus-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAfStop
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This function returns the Auto-Focus-Control status.
 *
 * @param   hCamEngine              Handle to the CamEngine instance.
 * @param   pRunning                BOOL_TRUE: running, BOOL_FALSE: stopped
 * @param   pAutoFocusSearchAgoritm search algorithm ( ADAPTIVE_RANGE | HILL_CLIMBING )
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAfStatus
(
    CamEngineHandle_t                   hCamEngine,
    bool_t                              *pRunning,
    CamEngineAfSearchAlgorithm_t        *pSearchAgoritm
);



/*****************************************************************************/
/**
 * @brief   This function starts the Adaptive-DPF-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAdpfStart
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This function stops the Adaptive-DPF-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAdpfStop
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This functions configures the Adaptive-DPF-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAdpfConfigure
(
    CamEngineHandle_t           hCamEngine,      /**< handle CamEngine */
    const float                 gradient,        /**< gradient */
    const float                 offset,          /**< offset */
    const float                 min,             /**< upper bound */
    const float                 div,             /**< division factor */
    const uint8_t               sigmaGreen,      /**< sigma green */
    const uint8_t               sigmaRedBlue     /**< sigma red/blue */
);



/*****************************************************************************/
/**
 * @brief   This function returns the Adaptive-DPF-Control status.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: running, BOOL_FALSE: stopped
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAdpfStatus
(
    CamEngineHandle_t   hCamEngine,
    bool_t              *pRunning,
    float               *pGradient,        /**< gradient */
    float               *pOffset,          /**< offset */
    float               *pMin,             /**< upper bound */
    float               *pDiv,             /**< division factor */
    uint8_t             *pSigmaGreen,      /**< sigma green */
    uint8_t             *pSigmaRedBlue     /**< sigma red/blue */

);


/*****************************************************************************/
/**
 * @brief   This function starts the Adaptive-DPCC-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAdpccStart
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This function stops the Adaptive-DPCC-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAdpccStop
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This function returns the Adaptive-DPF-Control status.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: running, BOOL_FALSE: stopped
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAdpccStatus
(
    CamEngineHandle_t   hCamEngine,
    bool_t              *pRunning
);



/*****************************************************************************/
/**
 * @brief   This function starts the Adaptive-2DNR-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineA2dnrStart
(
    CamEngineHandle_t 			hCamEngine,
    const CamEngineA2dnrMode_t  mode           /**< run-mode */
);



/*****************************************************************************/
/**
 * @brief   This function stops the Adaptive-2DNR-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineA2dnrStop
(
    CamEngineHandle_t hCamEngine
);


/*****************************************************************************/
/**
 * @brief   This functions configures the Adaptive-2DNR-Control
 *			manual mode parameters.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineA2dnrConfigure
(
    CamEngineHandle_t           hCamEngine,      /**< handle CamEngine */
    const float                 sigma,        	 /**< sigma */
    const uint8_t               strength,  		 /**< strength */
    const uint8_t               pregmaStrength   /**< pregmaStrength */
);



/*****************************************************************************/
/**
 * @brief   This functions set the Adaptive-2DNR-Control auto mode table.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineA2dnrSetAutoTable
(
    CamEngineHandle_t           		hCamEngine,      	/**< handle CamEngine */
    const CamEngineA2dnrParamNode_t 	*pParamNodeTable,	/**< CamEngineA2dnrParamNode_t */
    const uint8_t               		nodeNum   			/**< nodeNum */
);



/*****************************************************************************/
/**
 * @brief   This function returns the Adaptive-2DNR-Control status.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: running, BOOL_FALSE: stopped
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineA2dnrStatus
(
    CamEngineHandle_t   	hCamEngine,
    bool_t              	*pRunning,
    CamEngineA2dnrMode_t  	*pMode,				/**< run-mode */
    float					*pGain,				/**< gain */
	float					*pIntegrationTime,	/**< integrationTime */
    float               	*pSigma,			/**< sigma */
    uint8_t             	*pStrength,			/**< strength */
    uint8_t             	*pPregmaStrength	/**< pregmaStrength */
);



/*****************************************************************************/
/**
 * @brief   This function starts the Adaptive-3DNR-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineA3dnrStart
(
    CamEngineHandle_t 			hCamEngine,
    const CamEngineA3dnrMode_t  mode           /**< run-mode */
);



/*****************************************************************************/
/**
 * @brief   This function stops the Adaptive-3DNR-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineA3dnrStop
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This functions configures the Adaptive-3DNR-Control
 *			manual mode parameters.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineA3dnrConfigure
(
    CamEngineHandle_t           hCamEngine,      /**< handle CamEngine */
    const uint8_t               strength,  		 /**< strength */
    const uint16_t              motionFactor,  	 /**< motionFactor */
    const uint16_t              deltaFactor      /**< deltaFactor */
);



/*****************************************************************************/
/**
 * @brief   This functions set the Adaptive-3DNR-Control auto mode table.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineA3dnrSetAutoTable
(
    CamEngineHandle_t           		hCamEngine,      	/**< handle CamEngine */
    const CamEngineA3dnrParamNode_t 	*pParamNodeTable,	/**< CamEngineA3dnrParamNode_t */
    const uint8_t               		nodeNum   			/**< nodeNum */
);



/*****************************************************************************/
/**
 * @brief   This function returns the Adaptive-3DNR-Control status.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: running, BOOL_FALSE: stopped
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineA3dnrStatus
(
    CamEngineHandle_t   	hCamEngine,
    bool_t              	*pRunning,
    CamEngineA3dnrMode_t 	*pMode,				/**< run-mode */
    float					*pGain,				/**< gain */
	float					*pIntegrationTime,	/**< integrationTime */
    uint8_t             	*pStrength,			/**< strength */
    uint16_t            	*pMotionFactor,		/**< motionFactor */
    uint16_t            	*pDeltaFactor		/**< deltaFactor */
);



/*****************************************************************************/
/**
 * @brief   This function starts the Adaptive-WDR_V3-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAwdr3Start
(
    CamEngineHandle_t 			hCamEngine,
    const CamEngineAwdr3Mode_t  mode           /**< run-mode */
);



/*****************************************************************************/
/**
 * @brief   This function stops the Adaptive-WDR_V3-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAwdr3Stop
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This functions configures the Adaptive-WDR_V3-Control
 *			manual mode parameters.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAwdr3Configure
(
    CamEngineHandle_t           hCamEngine,      /**< handle CamEngine */
    const uint8_t               strength,  		 /**< strength */
    const uint8_t               globalStrength,  /**< globalStrength */
    const uint8_t               maxGain          /**< maxGain */
);



/*****************************************************************************/
/**
 * @brief   This functions set the Adaptive-WDR_V3-Control auto mode table.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAwdr3SetAutoTable
(
    CamEngineHandle_t           		hCamEngine,      	/**< handle CamEngine */
    const CamEngineAwdr3ParamNode_t 	*pParamNodeTable,	/**< CamEngineAwdr3ParamNode_t */
    const uint8_t               		nodeNum   			/**< nodeNum */
);



/*****************************************************************************/
/**
 * @brief   This function returns the Adaptive-WDR_V3-Control status.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: running, BOOL_FALSE: stopped
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAwdr3Status
(
    CamEngineHandle_t   	hCamEngine,
    bool_t              	*pRunning,
    CamEngineAwdr3Mode_t 	*pMode,				/**< run-mode */
    float					*pGain,				/**< gain */
	float					*pIntegrationTime,	/**< integrationTime */
    uint8_t             	*pStrength,			/**< strength */
    uint8_t             	*pGlobalStrength,	/**< globalStrength */
    uint8_t             	*pMaxGain			/**< maxGain */
);



/*****************************************************************************/
/**
 * @brief   This function starts the Adaptive-HDR-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAhdrStart
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This function stops the Adaptive-HDR-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAhdrStop
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This functions configures the Adaptive-HDR-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAhdrConfigure
(
    CamEngineHandle_t           hCamEngine,		/**< handle CamEngine */
    const uint8_t               extBit,  		/**< extBit */
    const float                 hdrRatio       	/**< hdrRatio */
);



/*****************************************************************************/
/**
 * @brief   This function returns the Adaptive-HDR-Control status.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: running, BOOL_FALSE: stopped
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAhdrStatus
(
    CamEngineHandle_t   hCamEngine,
    bool_t              *pRunning,
    uint8_t             *pExtBit,			/**< strength */
	float				*pHdrRatio			/**< sigma */
);



/*****************************************************************************/
/**
 * @brief   This function starts the Adaptive-EE-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAeeStart
(
    CamEngineHandle_t 			hCamEngine,
    const CamEngineAeeMode_t  	mode           /**< run-mode */
);



/*****************************************************************************/
/**
 * @brief   This function stops the Adaptive-EE-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAeeStop
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This functions configures the Adaptive-EE-Control
 *			manual mode parameters.
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAeeConfigure
(
    CamEngineHandle_t           hCamEngine,      /**< handle CamEngine */
    const uint8_t               strength,  		 /**< strength */
	const uint16_t 				yUpGain,		 /**< yUpGain */
	const uint16_t 				yDownGain,		 /**< yDownGain */
	const uint16_t 				uvGain,			 /**< uvGain */
	const uint16_t 				edgeGain		 /**< edgeGain */
);



/*****************************************************************************/
/**
 * @brief   This functions set the Adaptive-EE-Control auto mode table.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAeeSetAutoTable

(
    CamEngineHandle_t           		hCamEngine,      	/**< handle CamEngine */
    const CamEngineAeeParamNode_t 		*pParamNodeTable,	/**< CamEngineAeeParamNode_t */
    const uint8_t               		nodeNum   			/**< nodeNum */
);



/*****************************************************************************/
/**
 * @brief   This function returns the Adaptive-EE-Control status.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: running, BOOL_FALSE: stopped
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAeeStatus
(
    CamEngineHandle_t   hCamEngine,
    bool_t              *pRunning,
    CamEngineAeeMode_t 	*pMode,				/**< run-mode */
    float				*pGain,				/**< gain */
	float				*pIntegrationTime,	/**< integrationTime */
    uint8_t             *pStrength,			/**< strength */
    uint16_t            *pYUpGain,			/**< pYUpGain */
    uint16_t            *pYDownGain,		/**< pYDownGain */
    uint16_t            *pUvGain,			/**< pUvGain */
    uint16_t            *pEdgeGain			/**< pEdgeGain */
);



/*****************************************************************************/
/**
 * @brief   This function starts the Adaptive-Filter-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAfltStart
(
    CamEngineHandle_t 			hCamEngine,
    const CamEngineAfltMode_t  	mode           /**< run-mode */
);



/*****************************************************************************/
/**
 * @brief   This function stops the Adaptive-Filter-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAfltStop
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This functions configures the Adaptive-Filter-Control
 *			manual mode parameters.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAfltConfigure
(
    CamEngineHandle_t           hCamEngine,      /**< handle CamEngine */
    const uint8_t               denoiseLevel,	 /**< denoiseLevel */
    const uint8_t               sharpenLevel	 /**< sharpenLevel */
);



/*****************************************************************************/
/**
 * @brief   This functions set the Adaptive-Filter-Control auto mode table.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAfltSetAutoTable
(
    CamEngineHandle_t           		hCamEngine,      	/**< handle CamEngine */
    const CamEngineAfltParamNode_t 		*pParamNodeTable,	/**< CamEngineAfltParamNode_t */
    const uint8_t               		nodeNum   			/**< nodeNum */
);



/*****************************************************************************/
/**
 * @brief   This function returns the Adaptive-Filter-Control status.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: running, BOOL_FALSE: stopped
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAfltStatus
(
    CamEngineHandle_t   	hCamEngine,
    bool_t              	*pRunning,
    CamEngineAfltMode_t 	*pMode,				/**< run-mode */
    float					*pGain,				/**< gain */
	float					*pIntegrationTime,	/**< integrationTime */
    uint8_t             	*pDenoiseLevel,		/**< pDenoiseLevel */
    uint8_t             	*pSharpenLevel		/**< pSharpenLevel */
);



/*****************************************************************************/
/**
 * @brief   This function starts the Auto-Video-Stabilization-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAvsStart
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This function stops the Auto-Video-Stabilization-Control.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAvsStop
(
    CamEngineHandle_t hCamEngine
);



/*****************************************************************************/
/**
 * @brief   This functions configures the Auto-Video-Stabilization.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   useParams           Whether to use the specified params or default
 *                              parameters.
 * @param   numItpPoints        See @ref AvsDampFuncParams_t.numItpPoints.
 * @param   theta               See @ref AvsDampFuncParams_t.theta.
 * @param   baseGain            See @ref AvsDampFuncParams_t.baseGain.
 * @param   fallOff             See @ref AvsDampFuncParams_t.fallOff.
 * @param   acceleration        See @ref AvsDampFuncParams_t.acceleration.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    invalid configuration
 *
 *****************************************************************************/
RESULT CamEngineAvsConfigure
(
    CamEngineHandle_t   hCamEngine,
    const bool          useParams,
    const uint16_t      numItpPoints,
    const float         theta,
    const float         baseGain,
    const float         fallOff,
    const float         acceleration
);



/*****************************************************************************/
/**
 * @brief   This functions configures the Auto-Video-Stabilization.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pUsingParams        Whether the returned parameters have been used
 *                              to generate a damping function or if the
 *                              function results from explicitely passed
 *                              interpolation points. If *pUsingParams is
 *                              BOOL_TRUE, the following parameters contain the
 *                              configured values of damping function parameters
 *                              if the function returns successfully.
 * @param   pNumItpPoints       See @ref AvsDampFuncParams_t.numItpPoints.
 * @param   pTheta              See @ref AvsDampFuncParams_t.theta.
 * @param   pBaseGain           See @ref AvsDampFuncParams_t.baseGain.
 * @param   pFallOff            See @ref AvsDampFuncParams_t.fallOff.
 * @param   pAcceleration       See @ref AvsDampFuncParams_t.acceleration.
 * @param   pNumDampData        Number of elements pointed to by *ppDampXData
 *                              resp. *ppDampYData.
 * @param   ppDampXData         *ppDampXData points to array of damping function
 *                              interpolation points x values on successful
 *                              return of function.
 * @param   ppDampYData         *ppDampYData points to array of damping function
 *                              interpolation points y values on successful
 *                              return of function.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_NULL_POINTER    a NULL pointer has been passed
 *
 *****************************************************************************/
RESULT CamEngineAvsGetConfig
(
    CamEngineHandle_t   hCamEngine,
    bool_t              *pUsingParams,
    uint16_t            *pNumItpPoints,
    float               *pTheta,
    float               *pBaseGain,
    float               *pFallOff,
    float               *pAcceleration,
    int                 *pNumDampData,
    double             **ppDampXData,
    double             **ppDampYData
);



/*****************************************************************************/
/**
 * @brief   This function returns the Auto-Video-Stabilization-Control status.
 *
 * @param   hCamEngine          Handle to the CamEngine instance.
 * @param   pRunning            BOOL_TRUE: running, BOOL_FALSE: stopped
 * @param   pCurrDisplVec       Current displacement vector (measured by VSM)
 * @param   pCurrOffsetVec      Current offset vector (calculated by AVS)
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT CamEngineAvsGetStatus
(
    CamEngineHandle_t   hCamEngine,
    bool_t              *pRunning,
    CamEngineVector_t   *pCurrDisplVec,
    CamEngineVector_t   *pCurrOffsetVec
);



#ifdef __cplusplus
}
#endif


/* @} cam_engine_aaa_api */


#endif /* __CAM_ENGINE_AAA_API_H__ */

