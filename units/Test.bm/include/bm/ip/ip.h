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

#if !defined __IP_IP_H__
#define __IP_IP_H__

#include "bm/base.h"
#include "json/cJSON.h"

void IP_Create(TestCtx_t *pCtx);

void IP_Error(TestCtx_t *pCtx);

void IP_Free(TestCtx_t *pCtx);

void IP_Golden(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig, cJSON *pReport);

void IP_GoldenAnalyze(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                      cJSON *pReport);

void IP_Hack(TestCtx_t *pCtx, uint64_t address, BMTest_t *pBmTest,
             uint32_t *pValue, cJSON *pReport);

void IP_InputLoad(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig, cJSON *pReport);

void IP_OutputSave(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                   cJSON *pReport);

void IP_Parse(TestCtx_t *pCtx, cJSON *);

void IP_RefreshBegin(TestCtx_t *pCtx);

void IP_RefreshAfter(TestCtx_t *pCtx);

void IP_Reset(TestCtx_t *pCtx);

void IP_WaitForReady(TestCtx_t *pCtx);

#endif
