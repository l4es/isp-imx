/******************************************************************************\
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/**
 * @file    calibtags.h
 *
 *
 *****************************************************************************/
#pragma once

/******************************************************************************
 * XML main section
 *****************************************************************************/
#define CALIB_FILESTART_TAG "matfile"

/******************************************************************************
 * XML cell section
 *****************************************************************************/
#define CALIB_CELL_TAG "cell"

/******************************************************************************
 * XML header section
 *****************************************************************************/
#define CALIB_HEADER_TAG "header"

#define CALIB_HEADER_CREATION_DATE_TAG "creation_date"
#define CALIB_HEADER_CREATOR_TAG "creator"
#define CALIB_HEADER_SENSOR_NAME_TAG "sensor_name"
#define CALIB_HEADER_SAMPLE_NAME_TAG "sample_name"
#define CALIB_HEADER_GENERATOR_VERSION_TAG "generator_version"

/******************************************************************************
 * XML resolution tag
 *****************************************************************************/
#define CALIB_HEADER_RESOLUTION_TAG "resolution"
#define CALIB_HEADER_RESOLUTION_FORMAT "%dx%d"
#define CALIB_HEADER_RESOLUTION_NO_ELEMENTS 2

#define CALIB_HEADER_RESOLUTION_NAME_TAG "name"
#define CALIB_HEADER_RESOLUTION_ID_TAG "id"
#define CALIB_HEADER_RESOLUTION_WIDTH_TAG "width"
#define CALIB_HEADER_RESOLUTION_HEIGHT_TAG "height"
#define CALIB_HEADER_RESOLUTION_FRATE_TAG "framerate"

/******************************************************************************
 * XML sensor section
 *****************************************************************************/
#define CALIB_SENSOR_TAG "sensor"

/******************************************************************************
 * XML AWB section
 *****************************************************************************/
#define CALIB_SENSOR_AWB_TAG "AWB"

#define CALIB_SENSOR_AWB_GLOBALS_TAG "globals"
#define CALIB_SENSOR_AWB_GLOBALS_NAME_TAG "name"
#define CALIB_SENSOR_AWB_GLOBALS_RESOLUTION_TAG "resolution"
#define CALIB_SENSOR_AWB_GLOBALS_SENSOR_FILENAME_TAG "sensorFilename"
#define CALIB_SENSOR_AWB_GLOBALS_SVDMEANVALUE_TAG "SVDMeanValue"
#define CALIB_SENSOR_AWB_GLOBALS_PCAMATRIX_TAG "PCAMatrix"
#define CALIB_SENSOR_AWB_GLOBALS_CENTERLINE_TAG "CenterLine"
#define CALIB_SENSOR_AWB_GLOBALS_KFACTOR_TAG "K_Factor"
#define CALIB_SENSOR_AWB_GLOBALS_RG1_TAG "afRg1"
#define CALIB_SENSOR_AWB_GLOBALS_MAXDIST1_TAG "afMaxDist1"
#define CALIB_SENSOR_AWB_GLOBALS_RG2_TAG "afRg2"
#define CALIB_SENSOR_AWB_GLOBALS_MAXDIST2_TAG "afMaxDist2"
#define CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE1_TAG "afGlobalFade1"
#define CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST1_TAG "afGlobalGainDistance1"
#define CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE2_TAG "afGlobalFade2"
#define CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST2_TAG "afGlobalGainDistance2"
#define CALIB_SENSOR_AWB_GLOBALS_FADE2_TAG "afFade2"
#define CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMAX_TAG "afCbMinRegionMax"
#define CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMAX_TAG "afCrMinRegionMax"
#define CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMAX_TAG "afMaxCSumRegionMax"
#define CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMIN_TAG "afCbMinRegionMin"
#define CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMIN_TAG "afCrMinRegionMin"
#define CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMIN_TAG "afMaxCSumRegionMin"
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_INDOOR_MIN_TAG "fRgProjIndoorMin"
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_OUTDOOR_MIN_TAG "fRgProjOutdoorMin"
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_TAG "fRgProjMax"
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_SKY_TAG "fRgProjMaxSky"
#define CALIB_SENSOR_AWB_GLOBALS_CLIP_OUTDOOR "awb_clip_outdoor"
#define CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE "RegionSize"
#define CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_INC "RegionSizeInc"
#define CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_DEC "RegionSizeDec"
#define CALIB_SENSOR_AWB_GLOBALS_IIR "IIR"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_ADD "DampCoefAdd"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_SUB "DampCoefSub"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_FILTER_THRESHOLD "DampFilterThreshold"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MIN "DampingCoefMin"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MAX "DampingCoefMax"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_INIT "DampingCoefInit"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MAX                 \
  "ExpPriorFilterSizeMax"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MIN                 \
  "ExpPriorFilterSizeMin"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_MIDDLE "ExpPriorMiddle"

#define CALIB_SENSOR_AWB_ILLUMINATION_TAG "illumination"
#define CALIB_SENSOR_AWB_ILLUMINATION_NAME_TAG "name"
#define CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_TAG "doorType"
#define CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_INDOOR "Indoor"
#define CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_OUTDOOR "Outdoor"
#define CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_TAG "awbType"
#define CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_MANUAL "MANUAL"
#define CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_AUTO "AUTO"
#define CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_WB_TAG "manualWB"
#define CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CC_TAG "manualccMatrix"
#define CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CTO_TAG "manualccOffsets"
#define CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAG "GMM"
#define CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_MVALUE_TAG                  \
  "GaussianMeanValue"
#define CALIB_SENSOR_AWB_ILLUMINATION_GMM_INV_COV_MATRIX_TAG "invCovMatrix"
#define CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_SFACTOR_TAG                 \
  "GaussianScalingFactor"
#define CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAU_TAG "tau"
#define CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_TAG "sat_CT"
#define CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_GAIN_TAG "gains"
#define CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_SAT_TAG "sat"
#define CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_TAG "vig_CT"
#define CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_GAIN_TAG "gains"
#define CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_VIG_TAG "vig"

#define CALIB_SENSOR_AWB_ILLUMINATION_ALSC_TAG "aLSC"
#define CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_TAG "resolution"
#define CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_LSC_PROFILE_LIST_TAG            \
  "LSC_PROFILE_LIST"

#define CALIB_SENSOR_AWB_ILLUMINATION_ACC_TAG "aCC"
#define CALIB_SENSOR_AWB_ILLUMINATION_ACC_CC_PROFILE_LIST_TAG "CC_PROFILE_LIST"

/******************************************************************************
 * XML LSC section
 *****************************************************************************/
#define CALIB_SENSOR_LSC_TAG "LSC"
#define CALIB_SENSOR_LSC_PROFILE_NAME_TAG "name"
#define CALIB_SENSOR_LSC_PROFILE_RESOLUTION_TAG "resolution"
#define CALIB_SENSOR_LSC_PROFILE_ILLUMINATION_TAG "illumination"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SECTORS_TAG "LSC_sectors"
#define CALIB_SENSOR_LSC_PROFILE_LSC_NO_TAG "LSC_No"
#define CALIB_SENSOR_LSC_PROFILE_LSC_XO_TAG "LSC_Xo"
#define CALIB_SENSOR_LSC_PROFILE_LSC_YO_TAG "LSC_Yo"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_X_TAG "LSC_SECT_SIZE_X"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_Y_TAG "LSC_SECT_SIZE_Y"
#define CALIB_SENSOR_LSC_PROFILE_LSC_VIGNETTING_TAG "vignetting"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_RED_TAG "LSC_SAMPLES_red"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENR_TAG "LSC_SAMPLES_greenR"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENB_TAG "LSC_SAMPLES_greenB"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_BLUE_TAG "LSC_SAMPLES_blue"

/******************************************************************************
 * XML CC section
 *****************************************************************************/
#define CALIB_SENSOR_CC_TAG "CC"
#define CALIB_SENSOR_CC_PROFILE_NAME_TAG "name"
#define CALIB_SENSOR_CC_PROFILE_SATURATION_TAG "saturation"
#define CALIB_SENSOR_CC_PROFILE_CC_MATRIX_TAG "ccMatrix"
#define CALIB_SENSOR_CC_PROFILE_CC_OFFSETS_TAG "ccOffsets"
#define CALIB_SENSOR_CC_PROFILE_WB_TAG "wb"

/******************************************************************************
 * XML AF section
 *****************************************************************************/
#define CALIB_SENSOR_AF_TAG "AF"

/******************************************************************************
 * XML AEC section
 *****************************************************************************/
#define CALIB_SENSOR_AEC_TAG "AEC"
#define CALIB_SENSOR_AEC_SETPOINT_TAG "SetPoint"
#define CALIB_SENSOR_AEC_CLM_TOLERANCE_TAG "ClmTolerance"
#define CALIB_SENSOR_AEC_DAMP_OVER_TAG "DampOver"
#define CALIB_SENSOR_AEC_DAMP_OVER_VIDEO_TAG "DampOverVideo"
#define CALIB_SENSOR_AEC_DAMP_UNDER_TAG "DampUnder"
#define CALIB_SENSOR_AEC_DAMP_UNDER_VIDEO_TAG "DampUnderVideo"
#define CALIB_SENSOR_AEC_AFPS_MAX_GAIN_TAG "aFpsMaxGain"
#define CALIB_SENSOR_AEC_ECM_TAG "ECM"
#define CALIB_SENSOR_AEC_ECM_NAME_TAG "name"
#define CALIB_SENSOR_AEC_ECM_SCHEMES_TAG "PrioritySchemes"
#define CALIB_SENSOR_AEC_ECM_SCHEME_NAME_TAG "name"
#define CALIB_SENSOR_AEC_ECM_SCHEME_OFFSETT0FAC_TAG "OffsetT0Fac"
#define CALIB_SENSOR_AEC_ECM_SCHEME_SLOPEA0_TAG "SlopeA0"

/******************************************************************************
 * XML BLS section
 *****************************************************************************/
#define CALIB_SENSOR_BLS_TAG "BLS"
#define CALIB_SENSOR_BLS_NAME_TAG "name"
#define CALIB_SENSOR_BLS_RESOLUTION_TAG "resolution"
#define CALIB_SENSOR_BLS_DATA_TAG "blsData"

/******************************************************************************
 * XML DEGAMMA section
 *****************************************************************************/
#define CALIB_SENSOR_DEGAMMA_TAG "DEGAMMA"

/******************************************************************************
 * XML WDR section
 *****************************************************************************/
#define CALIB_SENSOR_WDR_TAG "WDR"

/******************************************************************************
 * XML CAC section
 *****************************************************************************/
#define CALIB_SENSOR_CAC_TAG "CAC"
#define CALIB_SENSOR_CAC_NAME_TAG "name"
#define CALIB_SENSOR_CAC_RESOLUTION_TAG "resolution"
#define CALIB_SESNOR_CAC_X_NORMSHIFT_TAG "x_normshift"
#define CALIB_SESNOR_CAC_X_NORMFACTOR_TAG "x_normfactor"
#define CALIB_SESNOR_CAC_Y_NORMSHIFT_TAG "y_normshift"
#define CALIB_SESNOR_CAC_Y_NORMFACTOR_TAG "y_normfactor"
#define CALIB_SESNOR_CAC_X_OFFSET_TAG "x_offset"
#define CALIB_SESNOR_CAC_Y_OFFSET_TAG "y_offset"
#define CALIB_SESNOR_CAC_RED_PARAMETERS_TAG "red_parameters"
#define CALIB_SESNOR_CAC_BLUE_PARAMETERS_TAG "blue_parameters"

/******************************************************************************
 * XML DPF section
 *****************************************************************************/
#define CALIB_SENSOR_DPF_TAG "DPF"
#define CALIB_SENSOR_DPF_NAME_TAG "name"
#define CALIB_SENSOR_DPF_RESOLUTION_TAG "resolution"
#define CALIB_SENSOR_DPF_NLL_SEGMENTATION_TAG "NLL_SEGMENTATION"
#define CALIB_SENSOR_DPF_NLL_COEFF_TAG "nll_coeff_n"
#define CALIB_SENSOR_DPF_SIGMA_GREEN_TAG "SigmaGreen"
#define CALIB_SENSOR_DPF_SIGMA_RED_BLUE_TAG "SigmaRedBlue"
#define CALIB_SENSOR_DPF_GRADIENT_TAG "Gradient"
#define CALIB_SENSOR_DPF_OFFSET_TAG "Offset"
#define CALIB_SENSOR_DPF_NLGAINS_TAG "NlGains"

/******************************************************************************
 * XML DPCC section
 *****************************************************************************/
#define CALIB_SENSOR_DPCC_TAG "DPCC"
#define CALIB_SENSOR_DPCC_NAME_TAG "name"
#define CALIB_SENSOR_DPCC_RESOLUTION_TAG "resolution"
#define CALIB_SENSOR_DPCC_REGISTER_TAG "register"

#define CALIB_SENSOR_DPCC_REGISTER_NAME_TAG "name"
#define CALIB_SENSOR_DPCC_REGISTER_VALUE_TAG "value"

#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_MODE "ISP_DPCC_MODE"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_OUTPUT_MODE "ISP_DPCC_OUT_MODE"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_SET_USE "ISP_DPCC_SET_USE"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_1                      \
  "ISP_DPCC_METHODS_SET1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_2                      \
  "ISP_DPCC_METHODS_SET2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_3                      \
  "ISP_DPCC_METHODS_SET3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_1                      \
  "ISP_DPCC_LINE_THRESH_1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_1                     \
  "ISP_DPCC_LINE_MAD_FAC_1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_1 "ISP_DPCC_PG_FAC_1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_1 "ISP_DPCC_RND_THRESH_1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_1 "ISP_DPCC_RG_FAC_1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_2                      \
  "ISP_DPCC_LINE_THRESH_2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_2                     \
  "ISP_DPCC_LINE_MAD_FAC_2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_2 "ISP_DPCC_PG_FAC_2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_2 "ISP_DPCC_RND_THRESH_2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_2 "ISP_DPCC_RG_FAC_2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_3                      \
  "ISP_DPCC_LINE_THRESH_3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_3                     \
  "ISP_DPCC_LINE_MAD_FAC_3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_3 "ISP_DPCC_PG_FAC_3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_3 "ISP_DPCC_RND_THRESH_3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_3 "ISP_DPCC_RG_FAC_3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RO_LIMITS "ISP_DPCC_RO_LIMITS"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_OFFS "ISP_DPCC_RND_OFFS"

#define CALIB_SENSOR_HDR_TAG "HDR"
#define CALIB_SENSOR_HDR_CURVE_TAG "curve"

/******************************************************************************
 * XML system section
 *****************************************************************************/
#define CALIB_SYSTEM_TAG "system"

/******************************************************************************
 * XML AWB section
 *****************************************************************************/
#define CALIB_SYSTEM_AFPS_TAG "AFPS"

#define CALIB_SYSTEM_AFPS_DEFAULT_TAG "aFpsDefault"

/******************************************************************************
 * XML tag attributes
 *****************************************************************************/
#define CALIB_ATTRIBUTE_TYPE "type"
#define CALIB_ATTRIBUTE_TYPE_CHAR "char"
#define CALIB_ATTRIBUTE_TYPE_DOUBLE "double"
#define CALIB_ATTRIBUTE_TYPE_STRUCT "struct"
#define CALIB_ATTRIBUTE_TYPE_CELL "cell"

#define CALIB_ATTRIBUTE_SIZE "size"
#define CALIB_ATTRIBUTE_SIZE_FORMAT "[%d %d]"
#define CALIB_ATTRIBUTE_SIZE_NO_ELEMENTS 2

#define CALIB_ATTRIBUTE_INDEX "index"
