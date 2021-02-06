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

#if !defined __BARE_METAL_ABSTRACT_H__
#define __BARE_METAL_ABSTRACT_H__

#include "bm/base.h"

void BMA_BuffersInit(TestCtx_t *pCtx);

void BMA_BuffersCheck(TestCtx_t *pCtx);

void BMA_ConfigParse(TestCtx_t *pCtx, cJSON *pJObject);

void BMA_Create(TestCtx_t *pCtx);

void BMA_Free(TestCtx_t *pCtx);

void BMA_Golden(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig, cJSON *pReport);

void BMA_GoldenAnalyze(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                       cJSON *pReport);

void BMA_InputLoad(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                   cJSON *pReport);

void BMA_OutputSave(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                    cJSON *pReport);

void BMA_OutputVerify(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                      cJSON *pReport);

int32_t BMA_Refresh(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                    cJSON *pReport);

void BMA_Reset(TestCtx_t *pCtx);

#endif
